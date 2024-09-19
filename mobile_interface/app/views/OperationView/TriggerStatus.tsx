import { Text, View } from "react-native";
import StatusTriggerActive from "../../../assets/OnlineStatusAvailable.svg";
import StatusTriggerInactive from "../../../assets/OnlineStatusBusy.svg";
import { run } from "../../utils/run";

export default function TriggerStatus(props: { active: boolean }) {
  return (
    <View
      style={{
        flexDirection: "row",
        gap: 8,
        justifyContent: "center",
        alignItems: "center",
        paddingVertical: 8
      }}>
      {run(() => {
        if (props.active) {
          return (
            <>
              <StatusTriggerActive width={24} height={24} />
              <Text>Trigger ativo</Text>
            </>
          );
        } else {
          return (
            <>
              <StatusTriggerInactive width={24} height={24} />
              <Text
                style={{
                  fontWeight: "bold"
                }}>
                Trigger inativo
              </Text>
            </>
          );
        }
      })}
    </View>
  );
}
