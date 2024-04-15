import { View } from "react-native";
import { useBluetoothConnection } from "../../bluetooth/Context";
import { NextViewButton } from "../../components/NextViewButton";
import Slider from "@react-native-community/slider";
import { Button, Text } from "react-native-paper";
import { useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";
import GroupBox from "./GroupBox";
import { useState } from "react";
import { hapticFeedbackControl, hapticFeedbackControlLight } from "../../haptics/HapticFeedback";
import { ScrollView } from "react-native-gesture-handler";
import { useNavigation } from "../../hooks/useNavigation";

export default function ParameterSetup() {
  const navigator = useNavigation();
  const ble = useBluetoothConnection();
  const [status, sendControl] = useFirmwareStatus();

  const [rampupInterval, setRampupInterval] = useState(500);
  const [rampupStep, setRampupStep] = useState(1);

  const [transitionDuration, setTransitionDuration] = useState(0);

  const [winddownInterval, setWinddownInterval] = useState(500);
  const [winddownStep, setWinddownStep] = useState(1);

  async function saveParameters() {
    hapticFeedbackControl();
    await sendControl({ controlCode: "ParameterSetup_Complete", waitForResponse: true });
    navigator.goBack();
  }

  async function resetParameters() {
    hapticFeedbackControl();
  }

  return (
    <ScrollView
      contentContainerStyle={{ flexGrow: 1, gap: 48, paddingVertical: 16 }}
      style={{ padding: 16, gap: 32 }}>
      <GroupBox title="Rampa de subida">
        <Text style={{ marginBottom: 8 }}>
          No final da parte de operação, ao diminuir o PWM ao zero, especifique o intervalo entre as
          etapas.
        </Text>
        <View style={{ flexDirection: "row" }}>
          <View style={{ width: "50%" }}>
            <Text style={{ paddingHorizontal: 16 }}>{rampupStep} µS</Text>
            <Slider
              step={1}
              minimumValue={1}
              maximumValue={25}
              value={rampupStep}
              onValueChange={(v) => {
                hapticFeedbackControlLight();
                setRampupStep(v);
              }}
            />
          </View>
          <View style={{ width: "50%" }}>
            <Text style={{ paddingHorizontal: 16 }}>{rampupInterval} ms</Text>
            <Slider
              step={50}
              minimumValue={100}
              maximumValue={1000}
              value={rampupInterval}
              onValueChange={(v) => {
                hapticFeedbackControlLight();
                setRampupInterval(v);
              }}
            />
          </View>
        </View>
        <Text>
          Durante a curva de subida, o PWM aumentará de {rampupStep} em {rampupStep} µS, com{" "}
          {rampupInterval} ms de intervalo, até chegar no valor de MESE.
        </Text>
      </GroupBox>
      <GroupBox title="Transição">
        <Text>
          Durante a etapa de transição, o participante deverá ficar em pé sem segurar as barras por{" "}
          {transitionDuration} ms, antes de entrar na etapa de malha fechada.
        </Text>
        <View>
          <Text style={{ paddingHorizontal: 16 }}>{transitionDuration} ms</Text>
          <Slider
            step={100}
            minimumValue={100}
            maximumValue={5000}
            value={transitionDuration}
            onValueChange={(v) => {
              hapticFeedbackControlLight();
              setTransitionDuration(v);
            }}
          />
        </View>
      </GroupBox>
      <GroupBox title="Rampa de descida">
        <Text style={{ marginBottom: 8 }}>
          No final da parte de operação, ao diminuir o PWM ao zero, especifique o intervalo entre as
          etapas.
        </Text>
        <View style={{ flexDirection: "row" }}>
          <View style={{ width: "50%" }}>
            <Text style={{ paddingHorizontal: 16 }}>{winddownStep} µS</Text>
            <Slider
              step={1}
              minimumValue={1}
              maximumValue={25}
              value={winddownStep}
              onValueChange={(v) => {
                hapticFeedbackControlLight();
                setWinddownStep(v);
              }}
            />
          </View>
          <View style={{ width: "50%" }}>
            <Text style={{ paddingHorizontal: 16 }}>{winddownInterval} ms</Text>
            <Slider
              step={50}
              minimumValue={100}
              maximumValue={1000}
              value={winddownInterval}
              onValueChange={(v) => {
                hapticFeedbackControlLight();
                setWinddownInterval(v);
              }}
            />
          </View>
        </View>
        <Text>
          Durante a curva de descida, o PWM diminuirá de {winddownStep} em {winddownStep} µS, com{" "}
          {winddownInterval} ms de intervalo, até zerar.
        </Text>
      </GroupBox>
      <Text style={{ paddingHorizontal: 12 }}>
        Os parâmetros serão salvos na memória do hardware Gateway.
      </Text>
      <View style={{ flexDirection: "row", justifyContent: "space-between", paddingBottom: 16 }}>
        <Button mode="text" onPress={resetParameters}>
          Redefinir campos
        </Button>
        <Button mode="contained" onPress={saveParameters}>
          Salvar
        </Button>
      </View>
    </ScrollView>
  );
}
