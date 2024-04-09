import { View } from "react-native";
import { useBluetoothConnection } from "../../bluetooth/Context";
import { NextViewButton } from "../../components/NextViewButton";
import { Text } from "react-native-paper";
import { useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";

export default function ParameterSetup() {
  const ble = useBluetoothConnection();
  const [status, sendControl] = useFirmwareStatus();

  async function saveParameters() {
    await sendControl({ controlCode: "ParameterSetup_Complete", waitForResponse: true });
  }

  return (
    <View style={{ flex: 1 }}>
      <Text>Parametrização...</Text>
      <NextViewButton
        onClick={saveParameters}
        visible={ble.status === "CONNECTED"}
        icon="drag-vertical-variant"
        label={'Salvar e ir para "Paralela"'}
        target="Paralela"
      />
    </View>
  );
}
