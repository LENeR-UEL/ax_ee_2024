import { PermissionsAndroid, Platform, ToastAndroid } from "react-native";

const { requestMultiple, RESULTS, PERMISSIONS } = PermissionsAndroid;

export default async function requestBluetoothPermission() {
  if (Platform.OS === "ios") {
    return true;
  }

  if (Platform.OS === "android" && PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION) {
    const apiLevel = parseInt(Platform.Version.toString(), 10);

    if (apiLevel < 31) {
      const granted = await PermissionsAndroid.request(PERMISSIONS.ACCESS_FINE_LOCATION);
      return granted === PermissionsAndroid.RESULTS.GRANTED;
    }

    if (PERMISSIONS.BLUETOOTH_SCAN && PERMISSIONS.BLUETOOTH_CONNECT) {
      const result = await requestMultiple([
        PERMISSIONS.BLUETOOTH_SCAN,
        PERMISSIONS.BLUETOOTH_CONNECT,
        PERMISSIONS.ACCESS_FINE_LOCATION
      ]);

      return (
        result["android.permission.BLUETOOTH_CONNECT"] === RESULTS.GRANTED &&
        result["android.permission.BLUETOOTH_SCAN"] === RESULTS.GRANTED &&
        result["android.permission.ACCESS_FINE_LOCATION"] === RESULTS.GRANTED
      );
    }
  }

  ToastAndroid.showWithGravity(
    "A permissão de Bluetooth não foi obtida.",
    3000,
    ToastAndroid.BOTTOM
  );

  return false;
}
