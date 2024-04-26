import { ScrollView, StyleSheet, ToastAndroid, View } from "react-native";
import { Text, FAB, Button } from "react-native-paper";
import { MaterialCommunityIcons } from "@expo/vector-icons";
import { StatusDisplay } from "../../components/StatusDisplay";
import { WeightIndicationBar } from "./WeightIndicatorBar";
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

export default function OperationView() {
  const navigator = useNavigation();
  const [status, sendControl] = useFirmwareStatus();

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
    const unsubscribe = navigator.addListener("beforeRemove", (e) => {
      // Nessas situações, é inseguro permitir a mudança de estado no firmware.
      if (
        state === FirmwareState.OperationGradualIncrease ||
        state === FirmwareState.OperationTransition ||
        state === FirmwareState.OperationMalhaFechada ||
        (state === FirmwareState.OperationStop && status.pwm > 0)
      ) {
        e.preventDefault();
        ToastAndroid.showWithGravity(
          "É preciso finalizar as etapas de operação primeiro.",
          250,
          ToastAndroid.BOTTOM
        );
        return;
      }

      sendControl({ controlCode: "MainOperation_GoBackToMESECollecter", waitForResponse: true });
    });

    return () => {
      unsubscribe();
    };
  }, [status.mainOperationState?.state, status.pwm]);

  useEffect(() => {
    hapticFeedbackControl()
      .then(() => timeout(100))
      .then(() => hapticFeedbackControl());
  }, [status.mainOperationState?.state]);

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
          setpointColor="#C8E6C9"
          hideSetpointText={true}
          style={styles.weightBar}
        />
        <WeightIndicationBar
          textTop="DIR"
          maximumValue={status.collectedWeight}
          value={status.weightR}
          setpointValue={status.setpoint}
          onSetpointChange={onSetpointChange}
          fillColor="#9E9D24"
          setpointColor="#F0F4C3"
          hideSetpointText={false}
          style={styles.weightBar}
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
      <Text style={styles.statusText}>
        {run(() => {
          const state = status.mainOperationState;
          switch (state?.state) {
            case FirmwareState.OperationStart: {
              return `Aguardando peso atingir o setpoint (${status.weightL + status.weightR} / ${status.setpoint * 2} kg)`;
            }
            case FirmwareState.OperationGradualIncrease:
              return `Incremento manual, de 0 até MESE (${status.pwm} → ${status.mese} μs) (timer: ${state.pwmIncreaseTimeDelta} ms)`;
            case FirmwareState.OperationTransition: {
              return `Transição. Aguardando tempo...\n${state.timeDelta} / ${status.parameters.transitionTime} ms`;
            }
            case FirmwareState.OperationMalhaFechada:
              return `Malha fechada.\nErro: ${state.currentErrorValue} kg\nTimer: ${state.errorPositiveTimer} / ${status.parameters.malhaFechadaAboveSetpointTime} ms`;
            case FirmwareState.OperationStop: {
              if (status.pwm === 0) {
                return "Finalizado.";
              } else {
                return `Decremento manual, até 0 (${status.pwm} → 0 μs) (timer: ${state.pwmDecreaseTimeDelta} ms)`;
              }
            }
            default:
              return "";
          }
        }) +
          run(() => {
            return "\n\nTrigger EEG: " + status.isOVBoxFlagSet ? "Ativo" : "Inativo";
          })}
      </Text>
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
    gap: 0
  },
  statusText: {
    textAlign: "center",
    //@ts-expect-error
    width: "calc(100% - 96)",
    marginHorizontal: 48
  }
});
