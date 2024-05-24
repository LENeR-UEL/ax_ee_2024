import { StyleSheet, View } from "react-native";
import { Button } from "react-native-paper";
import { useCountdown } from "../../hooks/useCountdown";
import { useEffect, useRef } from "react";
import { StatusDisplay } from "../../components/StatusDisplay";
import { MaterialCommunityIcons } from "@expo/vector-icons";
import { hapticFeedbackControl } from "../../haptics/HapticFeedback";
import { NextViewButton } from "../../components/NextViewButton";
import { useUpdateEffect } from "../../hooks/useUpdateEffect";
import { useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";

export default function ParalelaView() {
  const [status, sendControl] = useFirmwareStatus();

  const weightLRef = useRef<number>();
  const weightRRef = useRef<number>();

  weightLRef.current = status.weightL;
  weightRRef.current = status.weightR;

  const countdown = useCountdown(async () => {
    // Requisitar dado...
    await sendControl({ controlCode: "Parallel_RegisterWeight", waitForResponse: true });
    hapticFeedbackControl();
  }, 1000);

  const startCountdown = () => {
    countdown.setCount(3);
  };

  const shownWeight = countdown.isCounting
    ? status.weightL + status.weightR
    : status.collectedWeight;

  // Vibrar quando o dado de peso médio é coletado do Gateway, sem vibrar na primeira render
  useUpdateEffect(() => {
    console.log("Peso médio coletado!");
    hapticFeedbackControl();
  }, [status.collectedWeight]);

  async function goToMESECollecter() {
    await sendControl({ controlCode: "Parallel_Complete", waitForResponse: true });
  }

  useEffect(() => {
    return () => {
      sendControl({ controlCode: "Parallel_GoBackToParameterSetup", waitForResponse: true });
    };
  }, []);

  return (
    <View style={{ flex: 1 }}>
      <View style={StyleSheet.compose(styles.barRow, { marginTop: 144 })}>
        <StatusDisplay
          style={StyleSheet.compose(styles.collectedData, styles.bar)}
          textLeft="L"
          textMain={status.weightL.toFixed(0)}
          textRight="kg"
        />
        <StatusDisplay
          style={StyleSheet.compose(styles.collectedData, styles.bar)}
          textLeft="R"
          textMain={status.weightR.toFixed(0)}
          textRight="kg"
        />
      </View>
      <View style={StyleSheet.compose(styles.barRow, { marginBottom: 32 })}>
        <StatusDisplay
          style={StyleSheet.compose(styles.collectedData, styles.bar)}
          textLeft="coleta"
          textMain={shownWeight.toFixed(0)}
          textRight="kg"
        />
      </View>
      <Button
        style={styles.actionButton}
        contentStyle={styles.actionButtonInner}
        mode="elevated"
        icon={() => countdown.isCounting && <MaterialCommunityIcons name="timer-sand" size={24} />}
        onPress={startCountdown}
        disabled={countdown.isCounting}>
        {countdown.isCounting ? `${countdown.count}s` : `Iniciar`}
      </Button>
      <NextViewButton
        icon="seat-legroom-extra"
        label={'Ir para "Malha aberta"'}
        target="Malha Aberta"
        visible={status.collectedWeight > 0}
        onClick={goToMESECollecter}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  barRow: {
    flexDirection: "row",
    marginHorizontal: 16,
    gap: 4
  },
  bar: {
    flexGrow: 1
  },
  actionButton: {
    marginTop: 0,
    margin: 16,
    backgroundColor: "#f0f0f0"
  },
  actionButtonInner: {
    paddingVertical: 12,
    backgroundColor: "#f0f0f0"
  },
  collectedData: {
    marginVertical: 2
  }
});
