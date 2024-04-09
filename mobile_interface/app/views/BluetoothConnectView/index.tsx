import { Animated, ScrollView, StyleSheet, View } from "react-native";
import { Button, Text, ActivityIndicator, Dialog, Portal } from "react-native-paper";
import { MaterialCommunityIcons } from "@expo/vector-icons";
import { useBluetoothConnection } from "../../bluetooth/Context";
import { useEffect, useRef, useState } from "react";
import { useUpdate } from "../../hooks/useUpdate";
import { Device } from "react-native-ble-plx";
import {
  hapticFeedbackControl,
  hapticFeedbackProcessEnd,
  hapticFeedbackProcessError
} from "../../haptics/HapticFeedback";
import { run } from "../../utils/run";
import { NextViewButton } from "../../components/NextViewButton";

export default function BluetoothConnectView() {
  const [isConnecting, setConnecting] = useState(false);
  const ble = useBluetoothConnection();

  const $foundDevices = useRef<Device[]>([]);
  const updateOnFoundDevice = useUpdate();

  const [connectionModalShown, setConnectionModalShown] = useState(false);

  const pulseAnim = useRef(new Animated.Value(0));

  useEffect(() => {
    Animated.timing(pulseAnim.current, {
      toValue: ble.status === "CONNECTED" ? 1 : 0.1,
      duration: ble.status === "CONNECTED" ? 2000 : 2000,
      useNativeDriver: true
    }).start();
  }, [pulseAnim.current, ble.status]);

  const showConnectionModal = async () => {
    if (ble.status === "CONNECTED") {
      return;
    }

    const scanBegan = await ble.beginScan((device) => {
      if (!device.name) {
        return;
      }

      // Adiciona o dispositivo à lista de dispositivos apenas se já não está ali
      if ($foundDevices.current.findIndex((d) => d.id === device.id) > -1) {
        return;
      }

      $foundDevices.current = [...$foundDevices.current, device];
      updateOnFoundDevice();
    });

    // Só mostrar o modal se o scan começou
    // Ex. usuário está com BT desligado, não mostrar modal
    if (scanBegan) {
      setConnectionModalShown(true);
      $foundDevices.current = [];
    } else {
      hapticFeedbackProcessError();
    }
  };

  const dismissConnectionModal = () => {
    ble.bleManager?.stopDeviceScan();
    setConnectionModalShown(false);

    if (ble.status === "DISCONNECTED") {
      ble.stopScan();
    }
  };

  const onDeviceSelect = (device: Device) => {
    if (ble.status !== "DISCONNECTED") {
      return;
    }

    dismissConnectionModal();
    hapticFeedbackControl();
    setConnecting(true);
    ble.connect(device).then(() => {
      setConnecting(false);
      hapticFeedbackProcessEnd();
    });
  };

  const disconnect = async () => {
    if (ble.status !== "CONNECTED") {
      return;
    }

    await ble.disconnect();
    hapticFeedbackControl();
  };

  console.log(`Encontrado ${$foundDevices.current.length} dispositivos distintos`);

  const contents = (
    <View style={{ flex: 1 }}>
      <Animated.View style={{ ...styles.iconWrapper, opacity: pulseAnim.current }}>
        <MaterialCommunityIcons name="bluetooth" size={96} color="#484848" />
      </Animated.View>
      {run(() => {
        if (ble.status === "CONNECTED") {
          return <Text style={styles.text}>Conectado ao dispositivo {ble.device!.name}.</Text>;
        }

        return <Text style={styles.text}>Conecte ao dispositivo via Bluetooth.</Text>;
      })}
      {isConnecting && (
        <View style={styles.statusIndicatorContainer}>
          <ActivityIndicator size="small" />
          <Text style={styles.statusIndicator}>Conectando ao dispositivo...</Text>
        </View>
      )}
      <View style={styles.buttons}>
        {run(() => {
          if (ble.status === "CONNECTED") {
            return (
              <Button
                contentStyle={styles.pairButtonInner}
                mode="elevated"
                onPress={disconnect}
                disabled={isConnecting}
              >
                Desconectar
              </Button>
            );
          } else {
            return (
              <Button
                contentStyle={styles.pairButtonInner}
                mode="elevated"
                onPress={showConnectionModal}
                disabled={isConnecting}
              >
                Procurar dispositivos...
              </Button>
            );
          }
        })}
      </View>
      <Portal>
        <Dialog visible={connectionModalShown} onDismiss={dismissConnectionModal}>
          <Dialog.Title>Selecione o dispositivo</Dialog.Title>
          <Dialog.Content>
            <View style={styles.statusIndicatorModalContainer}>
              <ActivityIndicator size="small" />
              <Text style={styles.statusIndicator}>Procurando dispositivos...</Text>
            </View>
            <ScrollView contentContainerStyle={styles.deviceList}>
              {$foundDevices.current.map((device, index) => {
                return (
                  <Button
                    key={device.id}
                    contentStyle={styles.deviceListDevice}
                    onPress={() => onDeviceSelect(device)}
                  >
                    {device.name}
                  </Button>
                );
              })}
            </ScrollView>
          </Dialog.Content>
          <Dialog.Actions>
            <Button onPress={dismissConnectionModal}>Cancelar</Button>
          </Dialog.Actions>
        </Dialog>
      </Portal>
      <NextViewButton
        visible={connectionModalShown === false && ble.status === "CONNECTED"}
        icon="cog"
        label={'Ir para "Parametrização"'}
        target="Parametrização"
      />
    </View>
  );

  return contents;
}

const styles = StyleSheet.create({
  iconWrapper: {
    marginTop: 64,
    marginVertical: 16,
    justifyContent: "center",
    alignItems: "center"
  },
  text: {
    marginHorizontal: 32,
    marginVertical: 8
  },
  buttons: {
    marginVertical: 8,
    marginHorizontal: 32,
    gap: 8
  },
  pairButtonInner: {
    paddingVertical: 12,
    backgroundColor: "#f0f0f0"
  },
  statusIndicator: {
    marginLeft: 16
  },
  statusIndicatorContainer: {
    marginHorizontal: 32,
    flexDirection: "row",
    alignItems: "center"
  },
  deviceList: {
    gap: 4
  },
  deviceListDevice: {
    justifyContent: "flex-start"
  },
  statusIndicatorModalContainer: {
    flexDirection: "row",
    justifyContent: "flex-start",
    alignItems: "center",
    marginBottom: 16
  }
});
