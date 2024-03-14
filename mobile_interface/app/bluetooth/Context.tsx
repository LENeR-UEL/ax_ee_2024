import { PropsWithChildren, createContext, useContext, useEffect, useMemo, useState } from "react";
import requestBluetoothPermission from "./requestPermission";
import { BleManager, Device, ScanMode, State } from "react-native-ble-plx";
import connectToDevice from "./connectToDevice";
import { ToastAndroid } from "react-native";

interface BTDisconnected {
  bleManager: BleManager | null;
  status: "DISCONNECTED";
  device: null;
  connect(device: Device): Promise<boolean>;
  beginScan(callbackDeviceFound: (device: Device) => void): Promise<boolean>;
  stopScan(): void;
}

interface BTConnected {
  bleManager: BleManager;
  status: "CONNECTED";
  device: Device;
  disconnect(): Promise<void>;
}

export type BluetoothContext = BTConnected | BTDisconnected;

const btContext = createContext<BluetoothContext | null>(null);

export const useBluetoothConnection = () => useContext(btContext)!;

export const BluetoothProvider = (props: PropsWithChildren) => {
  const bleManager = useMemo(() => {
    try {
      return new BleManager();
    } catch (error) {
      console.error("Failed to create Bluetooth Manager.", error);
      ToastAndroid.showWithGravity(
        "Houve um erro ao inicializar a lógica de Bluetooth.",
        3000,
        ToastAndroid.BOTTOM
      );

      return null;
    }
  }, []);

  const [btDevice, setBtDevice] = useState<Device | null>(null);

  let btObject: BluetoothContext;

  if (btDevice === null) {
    btObject = {
      bleManager: bleManager,
      status: "DISCONNECTED",
      device: null,
      connect: async (device) => {
        const fail = (message: string) => {
          ToastAndroid.showWithGravity(message, 3000, ToastAndroid.BOTTOM);
          setBtDevice(null);
          return false;
        };

        const connectedDevice = await connectToDevice(device);

        if (!connectedDevice) return fail("Dispositivo não encontrado.");

        device.onDisconnected((error, device) => {
          console.info(`Bluetooth device ${device.id} disconnected, error=`, error);

          setBtDevice(null);
        });

        setBtDevice(device);

        return true;
      },
      beginScan: async (callback) => {
        const fail = (message: string) => {
          ToastAndroid.showWithGravity(message, 3000, ToastAndroid.BOTTOM);
          setBtDevice(null);
          return false;
        };

        if (bleManager === null || (await bleManager.state()) !== State.PoweredOn)
          return fail("A conexão Bluetooth não está disponível.");

        if ((await requestBluetoothPermission()) === false)
          return fail("A permissão de Bluetooth não foi obtida.");

        console.log("Begin scanning");

        bleManager.stopDeviceScan();
        bleManager.startDeviceScan(null, { scanMode: ScanMode.LowLatency }, (error, device) => {
          if (error) {
            // Handle error (scanning will be stopped automatically)
            console.error("Error during device scan...", error);
            return;
          }

          device = device!;

          callback(device);
        });

        return true;
      },
      stopScan: () => {
        console.log("stop scan");
        bleManager?.stopDeviceScan();
      }
    };
  } else {
    btObject = {
      bleManager: bleManager!,
      status: "CONNECTED",
      device: btDevice,
      disconnect: async () => {
        await btDevice?.cancelConnection();
        setBtDevice(null);
      }
    };
  }

  return <btContext.Provider value={btObject}>{props.children}</btContext.Provider>;
};
