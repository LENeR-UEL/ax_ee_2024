import {
  MutableRefObject,
  PropsWithChildren,
  createContext,
  useContext,
  useMemo,
  useRef,
  useState
} from "react";
import requestBluetoothPermission from "./requestPermission";
import { BleManager, Device, ScanMode, State, fullUUID } from "react-native-ble-plx";
import connectToDevice from "./connectToDevice";
import { ToastAndroid } from "react-native";
import { useNavigation } from "../hooks/useNavigation";
import { CommonActions } from "@react-navigation/native";
import { ScreenNames } from "../Routing";

export const SERVICE_UUID = fullUUID("ab04");
export const STATUS_UUID = fullUUID("ff01");
export const CONTROL_UUID = fullUUID("ff0f");

interface BTDisconnected {
  bleManager: BleManager | null;
  status: "DISCONNECTED";
  device: null;
  deviceRef: MutableRefObject<Device | null>;
  connect(device: Device): Promise<boolean>;
  beginScan(callbackDeviceFound: (device: Device) => void): Promise<boolean>;
  stopScan(): void;
}

interface BTConnected {
  bleManager: BleManager;
  status: "CONNECTED";
  device: Device;
  deviceRef: MutableRefObject<Device | null>;
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
  const deviceRef = useRef<Device | null>(null);

  let btObject: BluetoothContext;

  if (btDevice === null) {
    btObject = {
      bleManager: bleManager,
      status: "DISCONNECTED",
      device: null,
      deviceRef,
      connect: async (device) => {
        const fail = (message: string) => {
          ToastAndroid.showWithGravity(message, 3000, ToastAndroid.BOTTOM);
          setBtDevice(null);
          deviceRef.current = null;
          return false;
        };

        const connectedDevice = await connectToDevice(device);

        if (!connectedDevice) return fail("Dispositivo não encontrado.");

        // Enviar StillAlive periódico para que o hardware não considere que a conexão foi perdida
        const timer = setInterval(() => {
          console.debug("Sending still alive packet...");
          device.writeCharacteristicWithResponseForService(
            SERVICE_UUID,
            CONTROL_UUID,
            "AQA=" // 0x0100, code=StillAlive extraData=00
          );
        }, 250);

        device.onDisconnected((error, device) => {
          console.info(`Bluetooth device ${device.id} disconnected, error=`, error);

          clearInterval(timer);
          setBtDevice(null);
          deviceRef.current = null;
        });

        setBtDevice(device);
        deviceRef.current = device;

        return true;
      },
      beginScan: async (callback) => {
        const fail = (message: string) => {
          ToastAndroid.showWithGravity(message, 3000, ToastAndroid.BOTTOM);
          setBtDevice(null);
          deviceRef.current = null;
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
      deviceRef,
      disconnect: async () => {
        await btDevice?.cancelConnection();
        setBtDevice(null);
        deviceRef.current = null;
      }
    };
  }

  const navigator = useNavigation();
  const previousHasDeviceRef = useRef(false);
  if (previousHasDeviceRef.current && btDevice === null) {
    navigator.dispatch(
      CommonActions.reset({
        index: 1,
        routes: [{ name: "Bluetooth" satisfies ScreenNames[number] }]
      })
    );
    alert("Conexão Bluetooth caiu. Não é possível operar o aplicativo.");
  }
  previousHasDeviceRef.current = btDevice !== null;

  return <btContext.Provider value={btObject}>{props.children}</btContext.Provider>;
};
