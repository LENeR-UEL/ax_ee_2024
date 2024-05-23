import { Animated, ScrollView, StyleSheet, ToastAndroid, View } from "react-native";
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
import { useFirmwareStatus } from "../../bluetooth/useFirmwareStatus";
import { useNavigation } from "../../hooks/useNavigation";
import { Image } from "expo-image";
import StatusCANUp from "../../../assets/OnlineStatusAvailable.svg";
import StatusCANDown from "../../../assets/OnlineStatusBusy.svg";
import { useBeepSound } from "../../hooks/useBeepSound/useBeepSound";

export default function BluetoothConnectView() {
  const navigator = useNavigation();
  const [isConnecting, setConnecting] = useState(false);
  const ble = useBluetoothConnection();
  const [firmwareStatus, sendControl] = useFirmwareStatus();

  const $foundDevices = useRef<Device[]>([]);
  const updateOnFoundDevice = useUpdate();

  const [connectionModalShown, setConnectionModalShown] = useState(false);

  const pulseAnim = useRef(new Animated.Value(0));

  const sfxControl = useBeepSound("control");

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

    sfxControl.play(true);
    dismissConnectionModal();
    hapticFeedbackControl();
    setConnecting(true);
    ble.stopScan();
    ble.connect(device).then(() => {
      setConnecting(false);
      hapticFeedbackProcessEnd();
    });
  };

  const disconnect = async () => {
    if (ble.status !== "CONNECTED") {
      return;
    }

    sfxControl.play(true);
    hapticFeedbackControl();
    await ble.disconnect();
  };

  const invokeReset = async () => {
    if (ble.status !== "CONNECTED") {
      return;
    }

    sfxControl.play(true);
    hapticFeedbackControl();
    await sendControl({ controlCode: "FirmwareInvokeReset", waitForResponse: false });
    ble.disconnect();
    ToastAndroid.showWithGravity("Gateway reiniciando...", 1000, ToastAndroid.BOTTOM);
  };

  async function onStartProcessClick() {
    if (ble.status !== "CONNECTED") return;
    await sendControl({ controlCode: "ParameterSetup_Complete", waitForResponse: true });
    navigator.navigate("Paralela");
  }

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
              <>
                <Button
                  contentStyle={styles.pairButtonInner}
                  mode="elevated"
                  onPress={disconnect}
                  disabled={isConnecting}>
                  <MaterialCommunityIcons name="close" color="#484848" size={16} />
                  {"    "}
                  Desconectar
                </Button>
                <Button
                  onPress={invokeReset}
                  mode="elevated"
                  contentStyle={styles.resetButtonInner}>
                  <MaterialCommunityIcons name="nuke" color="#484848" size={16} />
                  {"    "}
                  Reiniciar Gateway
                </Button>
                <Button
                  onPress={() => navigator.navigate("Parametrização")}
                  mode="elevated"
                  contentStyle={styles.parameterButtonInner}>
                  <MaterialCommunityIcons name="cog" color="#484848" size={16} />
                  {"    "}
                  Parametrização
                </Button>
                <Button
                  onPress={onStartProcessClick}
                  mode="contained"
                  contentStyle={styles.startButtonInner}
                  disabled={!firmwareStatus.statusFlags.isCANAvailable}>
                  <MaterialCommunityIcons name="weight-kilogram" color="#fff" size={16} />
                  {"    "}
                  Iniciar processo
                </Button>
                <View
                  style={{
                    flexDirection: "row",
                    gap: 8,
                    justifyContent: "flex-start",
                    alignItems: "center"
                  }}>
                  {run(() => {
                    if (firmwareStatus.statusFlags.isCANAvailable) {
                      return (
                        <>
                          <StatusCANUp width={16} height={16} />
                          <Text>Barramento disponível</Text>
                        </>
                      );
                    } else {
                      return (
                        <>
                          <StatusCANDown width={24} height={24} />
                          <Text
                            style={{
                              fontWeight: "bold",
                              marginRight: 30
                            }}>
                            Barramento indisponível - não é possível operar o aplicativo. Reinicie
                            os hardwares Gateway e Estimulador manualmente.
                          </Text>
                        </>
                      );
                    }
                  })}
                </View>
              </>
            );
          } else {
            return (
              <Button
                contentStyle={styles.pairButtonInner}
                mode="elevated"
                onPress={showConnectionModal}
                disabled={isConnecting}>
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
              {$foundDevices.current.map((device) => {
                return (
                  <Button
                    key={device.id}
                    contentStyle={styles.deviceListDevice}
                    onPress={() => onDeviceSelect(device)}>
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
    paddingVertical: 4
  },
  resetButtonInner: {
    paddingVertical: 4
  },
  parameterButtonInner: {
    paddingVertical: 4
  },
  startButtonInner: {
    paddingVertical: 4
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
