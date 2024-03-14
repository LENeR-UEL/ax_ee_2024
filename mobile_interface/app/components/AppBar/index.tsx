import { MaterialCommunityIcons } from "@expo/vector-icons";
import { Appbar } from "react-native-paper";
import { useNavigation } from "../../hooks/useNavigation";

interface Props {
  title: string;
}

export function AppBar(props: Props) {
  const navigator = useNavigation();

  return (
    <Appbar.Header elevated>
      {navigator.canGoBack() && (
        <Appbar.Action
          icon={({ color, size }) => (
            <MaterialCommunityIcons name="arrow-left" size={size} color={color} />
          )}
          onPress={() => navigator.goBack()}
        />
      )}
      <Appbar.Content title={props.title} />
    </Appbar.Header>
  );
}
