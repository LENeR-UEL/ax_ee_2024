import { Device } from "react-native-ble-plx";
import { ToastAndroid } from "react-native";

/**
 * Scans for a compatible device, and connects to it. If a device is not found,
 * or there is an error connecting to it, this function resolves to null.
 * @returns the connected device, or null if not connected.
 */
export default async function connectToDevice(device: Device): Promise<Device | null> {
  try {
    device = await device.connect({
      refreshGatt: "OnConnected"
    });

    device = await device.requestMTU(512);

    console.log("Got MTU size: " + device.mtu);

    if (device.mtu < 32) {
      alert("Erro na conexão Bluetooth: dispositivo não suporta MTU maior que 32.");
      throw new Error("Got insufficient MTU size: " + device.mtu);
    }

    return await device.discoverAllServicesAndCharacteristics();
  } catch (error) {
    ToastAndroid.showWithGravity(
      "Houve um erro ao conectar ao dispositivo.",
      3000,
      ToastAndroid.BOTTOM
    );

    console.error(error);

    return null;
  }
}
