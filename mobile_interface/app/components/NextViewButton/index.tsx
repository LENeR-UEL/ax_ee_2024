import { MaterialCommunityIcons } from "@expo/vector-icons";
import { StyleSheet } from "react-native";
import { FAB } from "react-native-paper";
import { hapticFeedbackControl } from "../../haptics/HapticFeedback";
import { ScreenNames } from "../../Routing";
import { useNavigation } from "../../hooks/useNavigation";

interface Props {
  visible: boolean;
  icon?: string;
  label?: string;
  target: ScreenNames[number];
  onClick?: () => Promise<void> | void;
}

export function NextViewButton(props: Props) {
  const navigator = useNavigation();

  async function handleClick() {
    hapticFeedbackControl();
    if (props.onClick) await props.onClick();
    navigator.navigate(props.target);
  }

  if (!props.visible) {
    return;
  }

  return (
    <FAB
      style={styles.button}
      label={props.label ?? "Próxima etapa"}
      onPress={handleClick}
      animated={false}
      icon={(iconProps) => (
        <MaterialCommunityIcons name={(props.icon as any) ?? "check"} {...iconProps} />
      )}
    />
  );
}

const styles = StyleSheet.create({
  button: {
    position: "absolute",
    bottom: 32,
    right: 32
  }
});
