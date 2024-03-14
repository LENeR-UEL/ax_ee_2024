import { createNativeStackNavigator } from "@react-navigation/native-stack";
import {
  NavigationContainer,
  NavigationProp,
  useNavigationContainerRef
} from "@react-navigation/native";

import BluetoothConnectView from "./views/BluetoothConnectView";
import ParalelaView from "./views/Paralela";
import MalhaAbertaView from "./views/MalhaAbertaView";
import OperationView from "./views/OperationView";
import { createStackNavigator } from "@react-navigation/stack";
import { AppBar } from "./components/AppBar";

export type ScreenNames = ["Bluetooth", "Paralela", "Malha Aberta", "Operação"];
export type RootStackParamList = Record<ScreenNames[number], undefined>;
export type StackNavigation = NavigationProp<RootStackParamList>;

export interface RouteParams {
  title: string;
  previousPage: ScreenNames[number] | null;
}

const Stack = createNativeStackNavigator<Record<ScreenNames[number], RouteParams>>();

export function Router() {
  return (
    <Stack.Navigator
      screenOptions={{
        header: (props) => <AppBar title={(props.route.params! as any).title} />,
        freezeOnBlur: true
      }}
    >
      <Stack.Screen
        name="Bluetooth"
        component={BluetoothConnectView}
        initialParams={{ title: "Conexão Bluetooth", previousPage: null }}
      />
      <Stack.Screen
        name="Paralela"
        component={ParalelaView}
        initialParams={{ title: "Paralela", previousPage: "Bluetooth" }}
      />
      <Stack.Screen
        name="Malha Aberta"
        component={MalhaAbertaView}
        initialParams={{ title: "Malha Aberta", previousPage: "Paralela" }}
      />
      <Stack.Screen
        name="Operação"
        component={OperationView}
        initialParams={{ title: "Operação", previousPage: "Malha Aberta" }}
      />
    </Stack.Navigator>
  );
}
