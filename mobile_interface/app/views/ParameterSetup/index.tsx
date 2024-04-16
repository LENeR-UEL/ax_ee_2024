import { ToastAndroid, View } from "react-native";
import Slider from "@react-native-community/slider";
import { Button, Text } from "react-native-paper";
import { useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";
import GroupBox from "./GroupBox";
import { hapticFeedbackControl, hapticFeedbackControlLight } from "../../haptics/HapticFeedback";
import { ScrollView } from "react-native-gesture-handler";
import { useNavigation } from "../../hooks/useNavigation";

export default function ParameterSetup() {
  const [status, sendControl] = useFirmwareStatus();

  async function saveParameters() {
    hapticFeedbackControl();
    await sendControl({ controlCode: "ParameterSetup_Save", waitForResponse: true });
    ToastAndroid.showWithGravity("Dados salvos no Gateway.", 1000, ToastAndroid.BOTTOM);
  }

  async function resetParameters() {
    hapticFeedbackControl();
    await sendControl({ controlCode: "ParameterSetup_Reset", waitForResponse: true });
    ToastAndroid.showWithGravity("Dados redefinidos no Gateway.", 1000, ToastAndroid.BOTTOM);
  }

  return (
    <ScrollView
      contentContainerStyle={{ flexGrow: 1, gap: 48, paddingVertical: 16 }}
      style={{ padding: 16, gap: 32 }}>
      <GroupBox title="Rampa de subida">
        <Text style={{ marginBottom: 8 }}>
          No início da parte de operação, ao aumentar o PWM até MESE, especifique o intervalo entre
          as etapas.
        </Text>
        <View style={{ flexDirection: "row" }}>
          <View style={{ width: "50%" }}>
            <Text style={{ paddingHorizontal: 16 }}>
              {status.parameters.gradualIncreaseStep} µS
            </Text>
            <Slider
              step={1}
              minimumValue={1}
              maximumValue={25}
              value={status.parameters.gradualIncreaseStep}
              onValueChange={(v) => {
                hapticFeedbackControlLight();
                sendControl({
                  controlCode: "ParameterSetup_SetGradualIncreaseStep",
                  waitForResponse: false,
                  data: Math.floor(v)
                });
              }}
            />
          </View>
          <View style={{ width: "50%" }}>
            <Text style={{ paddingHorizontal: 16 }}>
              {status.parameters.gradualIncreaseInterval} ms
            </Text>
            <Slider
              step={50}
              minimumValue={100}
              maximumValue={1000}
              value={status.parameters.gradualIncreaseInterval}
              onValueChange={(v) => {
                hapticFeedbackControlLight();
                sendControl({
                  controlCode: "ParameterSetup_SetGradualIncreaseInterval",
                  waitForResponse: false,
                  data: Math.floor(v / 50)
                });
              }}
            />
          </View>
        </View>
        <Text>
          Durante a curva de subida, o PWM aumentará de {status.parameters.gradualIncreaseStep} em{" "}
          {status.parameters.gradualIncreaseStep} µS, com{" "}
          {status.parameters.gradualIncreaseInterval} ms de intervalo, até chegar no valor de MESE.
        </Text>
      </GroupBox>
      <GroupBox title="Transição">
        <Text>
          Durante a etapa de transição, o participante deverá ficar em pé sem segurar as barras por{" "}
          {status.parameters.transitionTime} ms, antes de entrar na etapa de malha fechada.
        </Text>
        <View>
          <Text style={{ paddingHorizontal: 16 }}>{status.parameters.transitionTime} ms</Text>
          <Slider
            step={100}
            minimumValue={100}
            maximumValue={5000}
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
      <GroupBox title="Rampa de descida">
        <Text style={{ marginBottom: 8 }}>
          No final da parte de operação, ao diminuir o PWM ao zero, especifique o intervalo entre as
          etapas.
        </Text>
        <View style={{ flexDirection: "row" }}>
          <View style={{ width: "50%" }}>
            <Text style={{ paddingHorizontal: 16 }}>
              {status.parameters.gradualDecreaseStep} µS
            </Text>
            <Slider
              step={1}
              minimumValue={1}
              maximumValue={25}
              value={status.parameters.gradualDecreaseStep}
              onValueChange={(v) => {
                hapticFeedbackControlLight();
                sendControl({
                  controlCode: "ParameterSetup_SetGradualDecreaseStep",
                  waitForResponse: false,
                  data: Math.floor(v)
                });
              }}
            />
          </View>
          <View style={{ width: "50%" }}>
            <Text style={{ paddingHorizontal: 16 }}>
              {status.parameters.gradualDecreaseInterval} ms
            </Text>
            <Slider
              step={50}
              minimumValue={100}
              maximumValue={1000}
              value={status.parameters.gradualDecreaseInterval}
              onValueChange={(v) => {
                hapticFeedbackControlLight();
                sendControl({
                  controlCode: "ParameterSetup_SetGradualDecreaseInterval",
                  waitForResponse: false,
                  data: Math.floor(v / 100)
                });
              }}
            />
          </View>
        </View>
        <Text>
          Durante a curva de descida, o PWM diminuirá de {status.parameters.gradualDecreaseStep} em{" "}
          {status.parameters.gradualDecreaseStep} µS, com{" "}
          {status.parameters.gradualDecreaseInterval} ms de intervalo, até zerar.
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
