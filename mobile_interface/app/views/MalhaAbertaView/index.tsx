import { StyleSheet, View } from "react-native";
import { Button, FAB } from "react-native-paper";
import { MaterialCommunityIcons } from "@expo/vector-icons";
import { StatusDisplay } from "../../components/StatusDisplay";
import { useBoolean } from "../../hooks/useBoolean";
import { run } from "../../utils/run";
import {
  hapticFeedbackControl,
  hapticFeedbackProcessEnd,
  hapticFeedbackProcessStart
} from "../../haptics/HapticFeedback";
import { useEffect } from "react";
import { useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";
import { NextViewButton } from "../../components/NextViewButton";
import { useUpdateEffect } from "../../hooks/useUpdateEffect";

export default function MalhaAbertaView() {
  const isOperating = useBoolean();
  const isWindingDown = useBoolean();

  const [status, sendControl] = useFirmwareStatus();

  const handleStartOperation = () => {
    if (isOperating.value === true) {
      return;
    }

    hapticFeedbackProcessStart();

    isOperating.setTrue();
    isWindingDown.setFalse();
  };

  const handleRegisterPwmValue = async () => {
    if (isOperating.value === false) {
      // Nada a parar
      return;
    }

    hapticFeedbackProcessEnd();
    isOperating.setFalse();
    isWindingDown.setTrue();

    await sendControl({ controlCode: "SaveMese", waitForResponse: true });
    await sendControl({ controlCode: "ResetPwmGradual", waitForResponse: true });
  };

  const changeMese = async (sign: "+" | "-") => {
    if (isOperating.value === false) {
      return;
    }

    await sendControl({
      controlCode: sign === "+" ? "IncreasePwmStep" : "DecreasePwmStep",
      waitForResponse: true
    });
  };

  useEffect(() => {
    if (isWindingDown && status.pwm === 0) {
      isWindingDown.setFalse();
    }
  }, [isWindingDown, status.pwm]);

  // Mecanismo de segurança: caso o usuário saia da tela pelo botão Back do celular,
  // devemos zerar o estímulo elétrico gradualmente.
  useEffect(() => {
    return () => {
      sendControl({ controlCode: "ResetPwmGradual", waitForResponse: true });
    };
  }, []);

  useUpdateEffect(() => {
    hapticFeedbackControl();
  }, [status.pwm]);

  return (
    <View style={{ flex: 1 }}>
      <View style={styles.statusDisplays}>
        <StatusDisplay textLeft="MESE" textMain={status.mese.toString()} />
        <StatusDisplay textLeft="PWM" textMain={status.pwm.toString()} textRight="µS" />
      </View>
      <View style={styles.valueButtonsContainer}>
        <FAB
          animated={false}
          mode="elevated"
          icon={() => <MaterialCommunityIcons name="minus" size={24} />}
          onPress={() => changeMese("-")}
          disabled={isOperating.value === false}
        ></FAB>
        <FAB
          animated={false}
          mode="elevated"
          icon={() => <MaterialCommunityIcons name="plus" size={24} />}
          onPress={() => changeMese("+")}
          disabled={isOperating.value === false}
        ></FAB>
      </View>
      {run(() => {
        if (isOperating.value === true || isWindingDown.value === true) {
          return (
            <Button
              style={styles.pairButton}
              mode="elevated"
              onPress={handleRegisterPwmValue}
              disabled={isWindingDown.value === true}
              icon={() => <MaterialCommunityIcons name="content-save" size={24} />}
            >
              Registrar valor PWM
            </Button>
          );
        }

        return (
          <Button style={styles.pairButton} mode="elevated" onPress={handleStartOperation}>
            Iniciar
          </Button>
        );
      })}
      <NextViewButton
        icon="seat-legroom-extra"
        label={'Ir para "Operação"'}
        target="Operação"
        visible={isWindingDown.value === false && isOperating.value === false && status.mese > 0}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  text: {
    marginHorizontal: 32,
    marginVertical: 8
  },
  pairButton: {
    margin: 32,
    height: 72,
    justifyContent: "center"
  },
  statusDisplays: {
    marginTop: 64,
    marginHorizontal: 32,
    marginVertical: 16,
    gap: 8
  },
  valueButtonsContainer: {
    flexDirection: "row",
    justifyContent: "center",
    alignItems: "center",
    gap: 16,
    margin: 32,
    position: "relative"
  }
});
