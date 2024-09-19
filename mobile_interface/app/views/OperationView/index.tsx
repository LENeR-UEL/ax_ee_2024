import { ScrollView, StyleSheet, ToastAndroid, View } from "react-native";
import { Text, FAB, Button } from "react-native-paper";
import { MaterialCommunityIcons } from "@expo/vector-icons";
import { StatusDisplay } from "../../components/StatusDisplay";
import { OverlayBar, WeightIndicationBar } from "./WeightIndicatorBar";
import { useEffect, useRef } from "react";
import { FirmwareState, useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";
import {
  hapticFeedbackControl,
  hapticFeedbackControlLight,
  hapticFeedbackProcessEnd
} from "../../haptics/HapticFeedback";
import { run } from "../../utils/run";
import { timeout } from "../../utils/timeout";
import { useNavigation } from "../../hooks/useNavigation";
import { useBeepSound } from "../../hooks/useBeepSound/useBeepSound";
import { useUpdateEffect } from "../../hooks/useUpdateEffect";
import { useBluetoothConnection } from "../../bluetooth/Context";
import TriggerStatus from "./TriggerStatus";

export default function OperationView() {
  const navigator = useNavigation();
  const ble = useBluetoothConnection();
  const [status, sendControl] = useFirmwareStatus();

  const weightTotal = status.weightL + status.weightR;

  async function updateMaxMese(operation: "+" | "-") {
    if (operation === "+") {
      await sendControl({
        controlCode: "MainOperation_IncreaseMESEMaxOnce",
        waitForResponse: true
      });
    } else {
      await sendControl({
        controlCode: "MainOperation_DecreaseMESEMaxOnce",
        waitForResponse: true
      });
    }

    await hapticFeedbackControl();
  }

  async function emergencyStop() {
    sendControl({ controlCode: "MainOperation_EmergencyStop", waitForResponse: true });

    ToastAndroid.showWithGravity("Parada de emergência acionada.", 1000, ToastAndroid.BOTTOM);

    for (let i = 0; i < 4; i++) await hapticFeedbackProcessEnd();
  }

  const _setpointRef = useRef(status.setpoint);
  _setpointRef.current = status.setpoint;
  async function onSetpointChange(newSetpoint: number) {
    newSetpoint = Math.round(newSetpoint);
    if (_setpointRef.current === newSetpoint) return;
    await sendControl({
      controlCode: "MainOperation_SetSetpoint",
      data: newSetpoint,
      waitForResponse: false
    });
    await hapticFeedbackControlLight();
  }

  useEffect(() => {
    const state = status.mainOperationState?.state ?? null;
    const unsubscribe = navigator.addListener("beforeRemove", async (e) => {
      // Em BluetoothContext, há um handler em device disconnected que seta o valor de device para null
      const connected = ble.deviceRef.current !== null;

      const isCriticalState =
        state === FirmwareState.OperationGradualIncrease ||
        state === FirmwareState.OperationTransition ||
        state === FirmwareState.OperationMalhaFechada ||
        (state === FirmwareState.OperationStop && status.pwm > 0);

      console.log({ connected, isCriticalState, state });

      // Nessas situações, é inseguro permitir a mudança de estado no firmware.
      if (connected && isCriticalState) {
        e.preventDefault();
        ToastAndroid.showWithGravity(
          "É preciso finalizar as etapas de operação primeiro.",
          250,
          ToastAndroid.BOTTOM
        );
        return;
      }

      sendControl({ controlCode: "MainOperation_GoBackToParallel", waitForResponse: true });
    });

    return () => {
      unsubscribe();
    };
  }, [status.mainOperationState?.state, status.pwm]);

  const beeper_setpointWeight = useBeepSound("PesoAcimaSetpoint");
  const beeper_PercentualEEGAtingido = useBeepSound("PercentualEEGAtingido");
  const beeper_FESAtivado_hl2 = useBeepSound("FESAtivado_hl2");
  const beeper_EstimulacaoContinua = useBeepSound("EstimulacaoContinua");
  const beeper_FESDesligada = useBeepSound("FESDesligada");

  // resetar buzzers no início
  useEffect(() => {
    beeper_setpointWeight.stop();
    beeper_PercentualEEGAtingido.stop();
    beeper_FESAtivado_hl2.stop();
    beeper_FESDesligada.stop();
    beeper_EstimulacaoContinua.stop();
    return () => {
      beeper_setpointWeight.stop();
      beeper_PercentualEEGAtingido.stop();
      beeper_FESAtivado_hl2.stop();
      beeper_FESDesligada.stop();
      beeper_EstimulacaoContinua.stop();
    };
  }, []);

  // buzzer ao iniciar a operação, indicando ao usuário pressionar as barras
  useUpdateEffect(() => {
    if (status.mainOperationState === null || status.statusFlags === null) return;

    const { state } = status.mainOperationState;
    const { isEEGFlagSet } = status.statusFlags;

    if (state === FirmwareState.OperationStart && isEEGFlagSet) {
      beeper_PercentualEEGAtingido.play(true);
    } else {
      beeper_PercentualEEGAtingido.stop();
    }
  }, [status.mainOperationState?.state, status.statusFlags.isEEGFlagSet]);

  // buzzer ao entrar na curva de subida
  useUpdateEffect(() => {
    if (status.mainOperationState === null) return;

    const { state } = status.mainOperationState!;
    if (state === FirmwareState.OperationGradualIncrease) {
      beeper_FESAtivado_hl2.play(true);
    }

    if (state === FirmwareState.OperationStop) {
      beeper_FESAtivado_hl2.stop();
      beeper_FESDesligada.play(true);
    }
  }, [status.mainOperationState?.state]);

  // buzzer constante enquanto estiver estimulando
  useUpdateEffect(() => {
    if (status.pwm !== 0) {
      beeper_EstimulacaoContinua.play(false);
    } else {
      beeper_EstimulacaoContinua.stop();
    }
  }, [status.pwm]);

  // buzzer constante enquanto peso >= setpoint
  useUpdateEffect(() => {
    if (weightTotal >= status.setpoint * 2) {
      beeper_setpointWeight.play(false);
    } else {
      beeper_setpointWeight.stop();
    }
  }, [weightTotal, status.setpoint]);

  useEffect(() => {
    hapticFeedbackControl()
      .then(() => timeout(100))
      .then(() => hapticFeedbackControl());
  }, [status.mainOperationState?.state]);

  const indicatorBars: OverlayBar[] = [
    { label: "Setpoint", showAtWeight: status.setpoint, color: "#C8E6C9" }
  ];

  if (status.mainOperationState?.state === FirmwareState.OperationStart) {
    indicatorBars.push({
      label: "Início",
      showAtWeight: status.setpoint * 0.2,
      color: "#ff0000"
    });
  }

  return (
    <ScrollView>
      <View
        style={StyleSheet.compose(
          { flexDirection: "row", marginHorizontal: 26, marginBottom: 16 },
          styles.weightBarsWrapper
        )}>
        <WeightIndicationBar
          textTop="ESQ"
          maximumValue={status.collectedWeight}
          value={status.weightL}
          setpointValue={status.setpoint}
          onSetpointChange={onSetpointChange}
          fillColor="#2E7D32"
          hideBarText
          style={styles.weightBar}
          bars={indicatorBars}
        />
        <WeightIndicationBar
          textTop="DIR"
          maximumValue={status.collectedWeight}
          value={status.weightR}
          setpointValue={status.setpoint}
          onSetpointChange={onSetpointChange}
          fillColor="#9E9D24"
          style={styles.weightBar}
          bars={indicatorBars}
        />
      </View>
      <View style={StyleSheet.compose(styles.group, styles.displaysGroup)}>
        <StatusDisplay textLeft="PWM" textMain={`${status.pwm}/${status.meseMax}`} textRight="µS" />
        <View style={styles.statusDisplayButtons}>
          <Text>MESE máximo: </Text>
          <FAB
            animated={false}
            size="small"
            icon={() => <MaterialCommunityIcons name="minus" size={24} />}
            onPress={() => updateMaxMese("-")}></FAB>
          <FAB
            animated={false}
            size="small"
            icon={() => <MaterialCommunityIcons name="plus" size={24} />}
            onPress={() => updateMaxMese("+")}></FAB>
        </View>
      </View>
      <Button
        mode="elevated"
        style={{ marginHorizontal: 72, marginVertical: 32 }}
        labelStyle={{ color: "#ff2222" }}
        contentStyle={{ backgroundColor: "#ffcccc" }}
        onPress={emergencyStop}>
        Parada de emergência
      </Button>
      <Text style={styles.statusText2}>
        {run(() => {
          const state = status.mainOperationState;
          switch (state?.state) {
            case FirmwareState.OperationStart: {
              return `Aguardando peso atingir a barra de início`;
            }
            case FirmwareState.OperationGradualIncrease:
              return `Incremento manual, de 0 até MESE (${status.pwm} → ${status.mese} μs)\nTimer: ${state.pwmIncreaseTimeDelta} ms`;
            case FirmwareState.OperationTransition: {
              return `Transição. Aguardando tempo...\n${state.timeDelta} / ${status.parameters.transitionTime} ms`;
            }
            case FirmwareState.OperationMalhaFechada:
              return `Malha fechada.\nErro: ${state.currentErrorValue} kg\nTimer: ${state.errorPositiveTimer} / ${status.parameters.malhaFechadaAboveSetpointTime} ms`;
            case FirmwareState.OperationStop: {
              if (status.pwm === 0) {
                return "Finalizado.";
              } else {
                return `Decremento manual, até 0 (${status.pwm} → 0 μs)\nTimer: ${state.pwmDecreaseTimeDelta} ms`;
              }
            }
            default:
              return "";
          }
        })}
      </Text>
      <TriggerStatus active={status.statusFlags.isEEGFlagSet} />
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  group: {
    borderRadius: 16,
    borderColor: "#ccc",
    borderWidth: 1,
    marginVertical: 2,
    marginHorizontal: 16,
    padding: 8,
    flexDirection: "row",
    alignItems: "center",
    gap: 16
  },
  displaysGroup: {
    flexDirection: "column",
    alignItems: "stretch",
    gap: 2
  },
  display: {
    flexGrow: 1
  },
  statusDisplayButtons: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "flex-end",
    gap: 8,
    marginTop: 4
  },
  lastGroup: {
    /* for scrolling */
    marginBottom: 16
  },
  weightBar: {
    flexGrow: 1
  },
  weightBarsWrapper: {
    marginTop: 16,
    gap: 4
  },
  statusText2: {
    textAlign: "center",
    //@ts-expect-error
    width: "calc(100% - 96)",
    marginHorizontal: 48
  }
});
