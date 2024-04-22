import { ScrollView, StyleSheet, ToastAndroid, View } from "react-native";
import { Text, FAB } from "react-native-paper";
import { MaterialCommunityIcons } from "@expo/vector-icons";
import { StatusDisplay } from "../../components/StatusDisplay";
import { WeightIndicationBar } from "./WeightIndicatorBar";
import { useEffect, useRef } from "react";
import { FirmwareState, useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";
import { hapticFeedbackControl, hapticFeedbackControlLight } from "../../haptics/HapticFeedback";
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
      <View style={StyleSheet.compose(styles.group, styles.weightBarsWrapper)}>
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
      <Text style={styles.statusText}>
        {run(() => {
          const state = status.mainOperationState;
          switch (state?.state) {
            case FirmwareState.OperationStart: {
              return `Aguardando peso atingir o setpoint (${status.weightL + status.weightR} / ${status.setpoint * 2})`;
            }
            case FirmwareState.OperationGradualIncrease:
              return `Incremento manual, de 0 até MESE (${status.pwm} / ${status.mese}) (timer: ${state.pwmIncreaseTimeDelta} / ${status.parameters.gradualIncreaseInterval} ms)`;
            case FirmwareState.OperationTransition: {
              const timer = state.weightClass === 0 ? state.weightClassTimer : 0;
              return `Transição. Aguardando liberação do peso nas barras (classe 0)\nClasse atual: ${state.weightClass}\nTimer: ${timer} / ${status.parameters.transitionTime} ms`;
            }
            case FirmwareState.OperationMalhaFechada:
              return `Malha fechada.\nErro: ${state.currentErrorValue} kg\nTimer: ${state.errorPositiveTimer} / ${status.parameters.malhaFechadaAboveSetpointTime} ms`;
            case FirmwareState.OperationStop: {
              if (status.pwm === 0) {
                return "Finalizado.";
              } else {
                return `Decremento manual, até 0 (${status.pwm} / 0) (timer: ${state.pwmDecreaseTimeDelta} / ${status.parameters.gradualDecreaseInterval} ms)`;
              }
            }
            default:
              return "";
          }
        }) +
          run(() => {
            return status.isOVBoxFlagSet ? "\n\nFLAG: TRUE" : "\n\nFLAG: FALSE";
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
    justifyContent: "flex-end",
    gap: 2,
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
