import { registerRootComponent } from "expo";
import { BluetoothProvider } from "./bluetooth/Context";
import { SafeAreaProvider } from "react-native-safe-area-context";
import { PaperProvider } from "react-native-paper";
import theme from "./theme";

import { Router } from "./Routing";
import { NavigationContainer, useNavigationContainerRef } from "@react-navigation/native";
import { GestureHandlerRootView } from "react-native-gesture-handler";
import { StatusBar } from "expo-status-bar";
import { useKeepAwake } from "expo-keep-awake";

export default function App() {
  const navigator = useNavigationContainerRef();

  useKeepAwake();

  return (
    <SafeAreaProvider>
      <BluetoothProvider>
        <GestureHandlerRootView style={{ flex: 1 }}>
          <NavigationContainer ref={navigator}>
            <PaperProvider theme={theme}>
              <StatusBar backgroundColor={theme.colors.elevation.level2} style="dark" />
              <Router />
            </PaperProvider>
          </NavigationContainer>
        </GestureHandlerRootView>
      </BluetoothProvider>
    </SafeAreaProvider>
  );
}

registerRootComponent(App);
