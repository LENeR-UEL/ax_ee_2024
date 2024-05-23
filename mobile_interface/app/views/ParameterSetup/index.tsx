import { ToastAndroid, View } from "react-native";
import Slider from "@react-native-community/slider";
import { Button, Text } from "react-native-paper";
import { useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";
import GroupBox from "./GroupBox";
import { hapticFeedbackControl, hapticFeedbackControlLight } from "../../haptics/HapticFeedback";
import { ScrollView } from "react-native-gesture-handler";
import { useBeepSound } from "../../hooks/useBeepSound/useBeepSound";

export default function ParameterSetup() {
  const [status, sendControl] = useFirmwareStatus();
  const sfxControl = useBeepSound("control");

  async function saveParameters() {
    sfxControl.play(true);
    hapticFeedbackControl();
    await sendControl({ controlCode: "ParameterSetup_Save", waitForResponse: true });
    ToastAndroid.showWithGravity("Dados salvos no Gateway.", 1000, ToastAndroid.BOTTOM);
  }

  async function resetParameters() {
    sfxControl.play(true);
    hapticFeedbackControl();
    await sendControl({ controlCode: "ParameterSetup_Reset", waitForResponse: true });
    ToastAndroid.showWithGravity("Dados redefinidos no Gateway.", 1000, ToastAndroid.BOTTOM);
  }

  return (
    <ScrollView
      contentContainerStyle={{ flexGrow: 1, gap: 16, paddingVertical: 16 }}
      style={{ padding: 16, gap: 32 }}>
      <GroupBox title="Curva de subida">
        <Text style={{ marginBottom: 8 }}>
          No início da parte de operação, ao aumentar o PWM até MESE, especifique o tempo total de
          curva.
        </Text>
        <View style={{ flexDirection: "column" }}>
          <Text style={{ paddingHorizontal: 16 }}>{status.parameters.gradualIncreaseTime} ms</Text>
          <Slider
            step={100}
            minimumValue={1}
            maximumValue={5000}
            value={status.parameters.gradualIncreaseTime}
            onValueChange={(v) => {
              hapticFeedbackControlLight();
              sendControl({
                controlCode: "ParameterSetup_SetGradualIncreaseTime",
                waitForResponse: false,
                data: Math.floor(v / 100)
              });
            }}
          />
        </View>
        <Text>
          Durante a curva de descida, o PWM aumentará até o MESE em{" "}
          {status.parameters.gradualIncreaseTime} ms.
        </Text>
      </GroupBox>
      <GroupBox title="Transição">
        <Text>
          Durante a etapa de transição, o participante deverá ficar em pé por{" "}
          {status.parameters.transitionTime} ms, antes de entrar na etapa de malha fechada.
        </Text>
        <View>
          <Text style={{ paddingHorizontal: 16 }}>{status.parameters.transitionTime} ms</Text>
          <Slider
            step={100}
            minimumValue={100}
            maximumValue={20000}
            value={status.parameters.transitionTime}
            onValueChange={(v) => {
              hapticFeedbackControlLight();
              sendControl({
                controlCode: "ParameterSetup_SetTransitionTime",
                waitForResponse: false,
                data: Math.floor(v / 100)
              });
            }}
          />
        </View>
      </GroupBox>
      <GroupBox title="Malha fechada - tempo após setpoint">
        <Text>
          Durante a etapa de malha fechada, o participante irá se apoiar nas barras conforme está
          fadigando. Quando o peso medido for maior que o setpoint durante{" "}
          {status.parameters.malhaFechadaAboveSetpointTime} ms, entraremos na rampa de descida.
        </Text>
        <View>
          <Text style={{ paddingHorizontal: 16 }}>
            {status.parameters.malhaFechadaAboveSetpointTime} ms
          </Text>
          <Slider
            step={100}
            minimumValue={1000}
            maximumValue={10000}
            value={status.parameters.malhaFechadaAboveSetpointTime}
            onValueChange={(v) => {
              hapticFeedbackControlLight();
              sendControl({
                controlCode: "ParameterSetup_SetMalhaFechadaAboveSetpointTime",
                waitForResponse: false,
                data: Math.floor(v / 100)
              });
            }}
          />
        </View>
      </GroupBox>
      <GroupBox title="Curva de descida (finalização)">
        <Text style={{ marginBottom: 8 }}>
          No final da parte de operação, ao diminuir o PWM até zerar, especifique o tempo total de
          curva.
        </Text>
        <View style={{ flexDirection: "column" }}>
          <Text style={{ paddingHorizontal: 16 }}>{status.parameters.gradualDecreaseTime} ms</Text>
          <Slider
            step={100}
            minimumValue={1}
            maximumValue={5000}
            value={status.parameters.gradualDecreaseTime}
            onValueChange={(v) => {
              hapticFeedbackControlLight();
              sendControl({
                controlCode: "ParameterSetup_SetGradualDecreaseTime",
                waitForResponse: false,
                data: Math.floor(v / 100)
              });
            }}
          />
        </View>
        <Text>
          Durante a curva de subida, o PWM diminuirá até 0 em até
          {status.parameters.gradualDecreaseTime} ms.
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
