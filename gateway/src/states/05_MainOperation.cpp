#include <Arduino.h>
#include <esp_log.h>
#include <string.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "MainOperation";

enum class MainOperationState
{
    // Aguardar peso medido zerar (classe 0) para podermos iniciar a operação
    START_WAIT_FOR_ZERO = 5,
    // Voluntário sentado... Aguardar peso nas barras = setpoint
    START_WAIT_FOR_WEIGHT_SETPOINT,
    // Aumento gradual do PWM, em malha aberta, de 0 até MESE
    GRADUAL_INCREMENT,
    // Voluntário de pé, mas ainda se equilibrando com as barras. Aguardar classe 0 por 2 segundos, significando que ele está de pé e equilibrado
    TRANSITION,
    // Estimulação contínua, malha fechada, entre MESE e MESE_MAX. Aguardar fadiga do voluntário (classe 2 por 2s)
    ACTION_CONTROL,
    // Decremento gradual do PWM, do valor atual até 0
    GRADUAL_DECREMENT,
    // Operação finalizada
    STOPPED
};

static const uint8_t MAIN_OPERATION_TWAI_SEND_INTERVAL_MS = 100;
static long lastTwaiSendTime;

// Dividir o peso lido em grupos desse tamanho. Usado para transições de estado, já que pode haver flutuações na leitura das balanças.
static const uint8_t MAIN_OPERATION_WEIGHT_CLASS_SIZE = 10;
static uint8_t lastWeightClass = 0;
static long lastWeightClassChangeTime = 0;

static const uint8_t GRADUAL_PWM_STEP = 5;
static const uint16_t GRADUAL_INTERVAL_MS = 500;
static long lastGradualChangeTime = 0;

static const uint8_t MAIN_OPERATION_MESE_MAX_STEP = 5;

// Instante mais recente onde o valor de erro era negativo
// Usado para calcular quanto tempo o erro está positivo, no estado de transição
static long calculatedErrorValueLastNegativeTime = 0;

static MainOperationState substate;

uint8_t determineCurrentWeightClass()
{
    int weightTotal = scaleGetWeightL() + scaleGetWeightR();
    return weightTotal / MAIN_OPERATION_WEIGHT_CLASS_SIZE;
}

void onMainOperationStateEnter()
{
    substate = MainOperationState::START_WAIT_FOR_ZERO;
    lastTwaiSendTime = 0;
    lastWeightClass = determineCurrentWeightClass();
    lastWeightClassChangeTime = millis();
    lastGradualChangeTime = millis();
    calculatedErrorValueLastNegativeTime = millis();
    data.mainOperationStateInformApp[0] = 0;
    data.mainOperationStateInformApp[1] = 0;
    data.mainOperationStateInformApp[2] = 0;
    data.mainOperationStateInformApp[3] = 0;
}

void onMainOperationStateLoop()
{
    if (!espBle.isConnected())
    {
        stateManager.switchTo(StateKind::Disconnected);
        return;
    }

    const long now = millis();

    // Verificar se a classe de peso mudou
    const uint8_t currentWeightClass = determineCurrentWeightClass();
    if (currentWeightClass != lastWeightClass)
    {
        lastWeightClass = currentWeightClass;
        lastWeightClassChangeTime = now;
    }

    const int classChangeTimeDelta = now - lastWeightClassChangeTime;

    FlagTrigger trigger = FlagTrigger::MalhaAberta;

    switch (substate)
    {
    case MainOperationState::START_WAIT_FOR_ZERO:
    {
        ESP_LOGD(TAG, "Iniciando... Aguardando peso classe 0 durante 2000 ms. Delta: %d  Classe atual: %d", classChangeTimeDelta, currentWeightClass);
        if (currentWeightClass == 0 && classChangeTimeDelta >= 2000)
        {
            ESP_LOGD(TAG, "Objetivo alcançado.");
            substate = MainOperationState::START_WAIT_FOR_WEIGHT_SETPOINT;
            lastWeightClassChangeTime = now;
            break;
        }

        data.mainOperationStateInformApp[0] = (uint8_t)substate;
        data.mainOperationStateInformApp[1] = currentWeightClass;
        data.mainOperationStateInformApp[2] = 0;
        data.mainOperationStateInformApp[3] = 0;

        break;
    }
    case MainOperationState::START_WAIT_FOR_WEIGHT_SETPOINT:
    {
        ESP_LOGD(TAG, "Iniciando... Aguardando peso >= setpoint.");
        uint16_t currentWeight = scaleGetWeightL() + scaleGetWeightR();
        if (currentWeight >= data.setpoint)
        {
            ESP_LOGD(TAG, "Objetivo alcançado.");
            substate = MainOperationState::GRADUAL_INCREMENT;
            lastGradualChangeTime = now;
            break;
        }

        twaiSend(TwaiSendMessageKind::SetRequestedPwm, 0);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);

        data.mainOperationStateInformApp[0] = (uint8_t)substate;
        data.mainOperationStateInformApp[1] = 0;
        data.mainOperationStateInformApp[2] = 0;
        data.mainOperationStateInformApp[3] = 0;

        break;
    }
    case MainOperationState::GRADUAL_INCREMENT:
    {
        ESP_LOGD(TAG, "Incremento manual... Aguardando pwmFeedback >= MESE. Atual: %d, MESE: %d", data.pwmFeedback, data.mese);

        if (data.pwmFeedback >= data.mese)
        {
            ESP_LOGD(TAG, "Objetivo alcançado.");
            substate = MainOperationState::TRANSITION;
            lastWeightClassChangeTime = now;
            break;
        }

        if (now - lastGradualChangeTime >= GRADUAL_INTERVAL_MS)
        {
            ESP_LOGD(TAG, "Incremento.");

            lastGradualChangeTime = now;

            twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
            twaiSend(TwaiSendMessageKind::SetRequestedPwm, data.pwmFeedback + GRADUAL_PWM_STEP);
        }

        trigger = FlagTrigger::MalhaAberta;

        data.mainOperationStateInformApp[0] = (uint8_t)substate;
        data.mainOperationStateInformApp[1] = 0;
        data.mainOperationStateInformApp[2] = 0;
        data.mainOperationStateInformApp[3] = 0;

        break;
    }
    case MainOperationState::TRANSITION:
    {
        ESP_LOGD(TAG, "Transição... Aguardando classe de peso 0 durante 2000ms. Delta: %d Classe atual: %d", classChangeTimeDelta, currentWeightClass);

        // 0kg durante 2 segundos
        if (lastWeightClass == 0 && classChangeTimeDelta >= 2000)
        {
            ESP_LOGD(TAG, "Objetivo alcançado.");
            substate = MainOperationState::ACTION_CONTROL;
            lastWeightClassChangeTime = now;
            lastGradualChangeTime = now;
            calculatedErrorValueLastNegativeTime = now;
            break;
        }

        trigger = FlagTrigger::MalhaAberta;

        data.mainOperationStateInformApp[0] = (uint8_t)substate;
        data.mainOperationStateInformApp[1] = currentWeightClass;
        data.mainOperationStateInformApp[2] = 0;
        data.mainOperationStateInformApp[3] = 0;

        break;
    }
    case MainOperationState::ACTION_CONTROL:
    {
        short currentErrorValue = (scaleGetWeightL() + scaleGetWeightR()) - data.setpoint;
        if (currentErrorValue < 0)
        {
            calculatedErrorValueLastNegativeTime = now;
        }

        ESP_LOGD(TAG, "Operação... Aguardando erro positivo durante 2000ms. Delta = %d ms, erro = %d", now - calculatedErrorValueLastNegativeTime, currentErrorValue);

        // Erro positivo durante 2000ms?
        if (now - calculatedErrorValueLastNegativeTime >= 2000)
        {
            ESP_LOGD(TAG, "Objetivo alcançado.");
            substate = MainOperationState::GRADUAL_DECREMENT;
            lastWeightClassChangeTime = now;
            lastGradualChangeTime = now;

            break;
        }

        trigger = FlagTrigger::MalhaFechadaOperacao;

        data.mainOperationStateInformApp[0] = (uint8_t)substate;
        data.mainOperationStateInformApp[1] = (currentErrorValue & 0xFF00) << 8;
        data.mainOperationStateInformApp[2] = currentErrorValue & 0x00FF;
        data.mainOperationStateInformApp[3] = 0;

        break;
    }
    case MainOperationState::GRADUAL_DECREMENT:
    {
        ESP_LOGD(TAG, "Decremento manual... Aguardando pwmFeedback = 0. Atual: %d", data.pwmFeedback);

        if (data.pwmFeedback == 0)
        {
            ESP_LOGD(TAG, "Objetivo alcançado.");
            substate = MainOperationState::STOPPED;
            lastWeightClassChangeTime = now;
            break;
        }

        if (now - lastGradualChangeTime >= GRADUAL_INTERVAL_MS)
        {
            ESP_LOGD(TAG, "Decremento.");

            lastGradualChangeTime = now;

            uint8_t requestedPwm = data.pwmFeedback;
            if (requestedPwm <= GRADUAL_PWM_STEP)
            {
                requestedPwm = 0;
            }
            else
            {
                requestedPwm = requestedPwm - GRADUAL_PWM_STEP;
            }

            twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)FlagTrigger::MalhaAberta);
            twaiSend(TwaiSendMessageKind::SetRequestedPwm, requestedPwm);
        }

        trigger = FlagTrigger::MalhaAberta;
        data.mainOperationStateInformApp[0] = (uint8_t)substate;
        data.mainOperationStateInformApp[1] = 0;
        data.mainOperationStateInformApp[2] = 0;
        data.mainOperationStateInformApp[3] = 0;
    }
    case MainOperationState::STOPPED:
    {

        ESP_LOGD(TAG, "Parado.");
        trigger = FlagTrigger::MalhaAberta;
        data.mainOperationStateInformApp[0] = (uint8_t)substate;
        data.mainOperationStateInformApp[1] = 0;
        data.mainOperationStateInformApp[2] = 0;
        data.mainOperationStateInformApp[3] = 0;
    }
    }

    if (now - lastTwaiSendTime >= MAIN_OPERATION_TWAI_SEND_INTERVAL_MS)
    {
        lastTwaiSendTime = now;
        twaiSend(TwaiSendMessageKind::WeightL, scaleGetWeightL());
        twaiSend(TwaiSendMessageKind::WeightR, scaleGetWeightR());
        twaiSend(TwaiSendMessageKind::Setpoint, data.setpoint);
        twaiSend(TwaiSendMessageKind::Mese, data.mese);
        twaiSend(TwaiSendMessageKind::MeseMax, data.meseMax);
        twaiSend(TwaiSendMessageKind::Trigger, (uint8_t)trigger);
    }
}

void onMainOperationStateTWAIMessage(TwaiReceivedMessage *receivedMessage)
{
    switch (receivedMessage->Kind)
    {
    case TwaiReceivedMessageKind::PwmFeedbackEstimulador:
        data.pwmFeedback = receivedMessage->ExtraData;
        break;
    }
}

void onMainOperationStateBLEControl(BluetoothControlCode code, uint8_t extraData)
{
    switch (code)
    {
    case BluetoothControlCode::MainOperation_SetSetpoint:
        data.setpoint = extraData;
        break;
    case BluetoothControlCode::MainOperation_IncreaseMESEMaxOnce:
        data.meseMax += MAIN_OPERATION_MESE_MAX_STEP;
        break;
    case BluetoothControlCode::MainOperation_DecreaseMESEMaxOnce:
        if (data.meseMax <= MAIN_OPERATION_MESE_MAX_STEP)
        {
            data.meseMax = 0;
        }
        else
        {
            data.meseMax -= MAIN_OPERATION_MESE_MAX_STEP;
        }
        break;
    case BluetoothControlCode::MainOperation_GoBackToMESECollecter:
        stateManager.switchTo(StateKind::MESECollecter);
        return;
    case BluetoothControlCode::FirmwareInvokeReset:
        esp_restart();
        return;
    }
}

void onMainOperationStateExit() {}
