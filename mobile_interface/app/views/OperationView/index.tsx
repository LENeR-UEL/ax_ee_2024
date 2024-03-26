import { ScrollView, StyleSheet, View } from "react-native";
import { FAB } from "react-native-paper";
import { MaterialCommunityIcons } from "@expo/vector-icons";
import { StatusDisplay } from "../../components/StatusDisplay";
import { WeightIndicationBar } from "./WeightIndicatorBar";
import { useEffect, useRef } from "react";
import { useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";
import { useBluetoothConnection } from "../../bluetooth/Context";
import { hapticFeedbackControl, hapticFeedbackControlLight } from "../../haptics/HapticFeedback";

export default function OperationView() {
  const bt = useBluetoothConnection();

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
    return () => {
      sendControl({ controlCode: "MainOperation_GoBackToMESECollecter", waitForResponse: true });
    };
  }, []);

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
          style={styles.weightBar}
        />
      </View>
      <View style={StyleSheet.compose(styles.group, styles.displaysGroup)}>
        <StatusDisplay textLeft="PWM" textMain={status.pwm.toString()} textRight="µS" />
        <StatusDisplay
          textLeft="MESE"
          textMain={status.mese.toString()}
          textRight="atual"
          style={styles.display}
        />
      </View>
      <View style={Object.assign({}, styles.group, styles.displaysGroup, styles.lastGroup)}>
        <View style={styles.statusDisplayWrapper}>
          <StatusDisplay
            textLeft="MESE"
            textMain={status.meseMax.toString()}
            textRight="max"
            style={styles.display}
          />
          <View style={styles.statusDisplayButtons}>
            <FAB
              animated={false}
              size="small"
              icon={() => <MaterialCommunityIcons name="minus" size={24} />}
              onPress={() => updateMaxMese("-")}
            ></FAB>
            <FAB
              animated={false}
              size="small"
              icon={() => <MaterialCommunityIcons name="plus" size={24} />}
              onPress={() => updateMaxMese("+")}
            ></FAB>
          </View>
        </View>
        <View style={styles.statusDisplayWrapper}></View>
      </View>
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
    padding: 16,
    flexDirection: "row",
    alignItems: "center",
    gap: 16
  },
  displaysGroup: {
    flexDirection: "column",
    alignItems: "stretch",
    gap: 2
  },
  statusDisplayWrapper: {
    flexDirection: "row",
    alignItems: "center",
    gap: 8
  },
  display: {
    flexGrow: 1
  },
  statusDisplayButtons: {
    flexDirection: "row",
    gap: 2
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
    gap: 2
  }
});
