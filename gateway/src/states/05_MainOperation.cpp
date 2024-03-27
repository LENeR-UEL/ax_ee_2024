#include <Arduino.h>
#include <esp_log.h>
#include "../Bluetooth/Bluetooth.h"
#include "../Twai/Twai.h"
#include "../Data.h"
#include "../StateManager.h"
#include "../Scale/Scale.h"

static const char *TAG = "MainOperation";

#define MAIN_OPERATION_TWAI_SEND_INTERVAL_MS 100
#define MAIN_OPERATION_MESE_MAX_STEP 5

enum class MainOperationState
{
    // Aguardar peso medido zerar (classe 0) para podermos iniciar a operação
    START_WAIT_FOR_ZERO,
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

static long lastTwaiSendTime;

static uint8_t lastWeightClass = 0;
static long lastWeightClassChangeTime = 0;

#define GRADUAL_PWM_STEP 5
#define GRADUAL_INTERVAL_MS 1000
static long lastGradualChangeTime = 0;

static MainOperationState substate;

// Dividir o peso lido em grupos desse tamanho. Usado para transições de estado, já que pode haver flutuações na leitura das balanças.
static const uint8_t MAIN_OPERATION_WEIGHT_CLASS_SIZE = 10;
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
            break;
        }

        trigger = FlagTrigger::MalhaAberta;

        break;
    }
    case MainOperationState::ACTION_CONTROL:
    {
        ESP_LOGD(TAG, "Operação... Aguardando classe de peso >=2 durante 2000ms. Delta: %d Classe atual: %d", classChangeTimeDelta, currentWeightClass);

        // 20kg+ aplicado durante 2000ms
        if (lastWeightClass >= 2 && classChangeTimeDelta >= 2000)
        {
            ESP_LOGD(TAG, "Objetivo alcançado.");
            substate = MainOperationState::GRADUAL_DECREMENT;
            lastWeightClassChangeTime = now;
            lastGradualChangeTime = now;

            break;
        }

        trigger = FlagTrigger::MalhaFechadaOperacao;

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
    }
    case MainOperationState::STOPPED:
    {

        ESP_LOGD(TAG, "Parado.");
        trigger = FlagTrigger::MalhaAberta;
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
