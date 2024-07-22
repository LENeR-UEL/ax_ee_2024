import { useState } from "react";
import { View } from "react-native";
import Dialog from "react-native-dialog";

interface WeightInputDialogProps {
  visible: boolean;
  onDismiss: () => void;
  onConfirm: (weight: number) => void;
}

export function WeightInputDialog(props: WeightInputDialogProps) {
  const [valueStr, setValueStr] = useState("50");

  let numberValue = 0;
  let hasParseError = false;

  try {
    numberValue = parseFloat(valueStr);
    numberValue = Math.min(Math.max(numberValue, 0), 255);

    if (isNaN(numberValue)) {
      throw numberValue;
    }
  } catch (_) {
    // Ignore parse error
    hasParseError = true;
  }

  return (
    <View>
      <Dialog.Container visible={props.visible}>
        <Dialog.Title>Definir peso manual</Dialog.Title>
        <Dialog.Description>Escreva o peso do participante, em kg.</Dialog.Description>
        <Dialog.Input
          label="Peso (kg)"
          keyboardType="numeric"
          textAlign="right"
          value={valueStr}
          onChangeText={(v) => setValueStr(v)}></Dialog.Input>
        <Dialog.Button label="Cancelar" onPress={props.onDismiss} />
        <Dialog.Button
          label="Definir peso"
          onPress={() => props.onConfirm(numberValue)}
          disabled={hasParseError}
        />
      </Dialog.Container>
    </View>
  );
}
