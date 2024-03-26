import { useEffect, useState } from "react";
import { fullUUID, BleError, BleErrorCode, Characteristic } from "react-native-ble-plx";
import { Buffer } from "buffer";
import { useBluetoothConnection } from "./Context";

const SERVICE_UUID = fullUUID("ab04");
const STATUS_UUID = fullUUID("ff01");
const CONTROL_UUID = fullUUID("ff0f");

interface StatusPacket {
  pwm: number;
  weightL: number;
  weightR: number;
  collectedWeight: number;
  mese: number;
  meseMax: number;
  setpoint: number;
}

const ControlCodes = {
  /**
   * Invoca um reset no gateway e no estimulador.
   */
  FirmwareInvokeReset: 0x00,

  /**
   * Enviado pelo app ao salvar a parametrização para aquela sessão.
   */
  ParameterSetup_Complete: 0x2f,

  Parallel_GoBackToParameterSetup: 0x10,
  Parallel_RegisterWeight: 0x11,
  Parallel_Complete: 0x1f,

  MESECollecter_GoBackToParallel: 0x20,
  MESECollecter_IncreaseOnce: 0x21,
  MESECollecter_DecreaseOnce: 0x22,
  MESECollecter_RegisterMESE: 0x24,
  MESECollecter_Complete: 0x2f,

  MainOperation_GoBackToMESECollecter: 0x30,
  MainOperation_SetSetpoint: 0x31,
  MainOperation_IncreaseMESEMaxOnce: 0x32,
  MainOperation_DecreaseMESEMaxOnce: 0x33,

  _Sync: 0x00,
  ResetPwmImmediate: 0x80,
  DecreasePwmStep: 0x81,
  IncreasePwmStep: 0x82,
  ResetPwmGradual: 0x83,
  CollectAverageWeight: 0x8a,
  SaveMese: 0x90,
  DecreaseMeseMaxStep: 0xa1,
  IncreaseMeseMaxStep: 0xa2,
  SetSetpoint: 0xb1,
  SetTrigger: 0xc0
} as const;

type ControlCodeDispatcher = (options: {
  controlCode: keyof typeof ControlCodes;
  waitForResponse: boolean;
  data?: number;
}) => Promise<boolean>;

function parseStatusPacket(packet: Buffer): StatusPacket {
  return {
    pwm: packet.readUint16LE(0),
    weightL: packet.readUint8(2),
    weightR: packet.readUint8(3),
    collectedWeight: packet.readUint16LE(4),
    mese: packet.readUint16LE(6),
    meseMax: packet.readUint16LE(8),
    setpoint: packet.readUint16LE(10)
  };
}

export function useFirmwareStatus(): [StatusPacket, ControlCodeDispatcher] {
  const ble = useBluetoothConnection();
  const [value, _setValue] = useState<StatusPacket>({
    pwm: 0,
    weightL: 0,
    weightR: 0,
    collectedWeight: 0,
    mese: 0,
    meseMax: 0,
    setpoint: 0
  });

  useEffect(() => {
    const peripheral = ble.device;
    if (peripheral === null) {
      return;
    }

    function statusListener(error: BleError | null, characteristic: Characteristic | null) {
      if (error?.errorCode === BleErrorCode.OperationCancelled) {
        // silently stop
        return;
      }

      if (error !== null) {
        console.error(`Error while monitoring status characteristic :`, error);
        return;
      }

      if (characteristic === null) {
        console.error(`Error while monitoring status characteristic: characteristic is null`);
        return;
      }

      if (characteristic.value === null) {
        console.error(`Error while monitoring status characteristic: characteristic value is null`);
        return;
      }

      const payload = Buffer.from(characteristic.value, "base64");
      const parsed = parseStatusPacket(payload);

      console.debug("Firmware Status: " + payload.toString("hex"));

      _setValue(parsed);
    }

    console.log("Subscribing to Status characteristic.");

    const statusSubscription = peripheral.monitorCharacteristicForService(
      SERVICE_UUID,
      STATUS_UUID,
      statusListener
    );

    return () => {
      console.log("Unsubscribing from Status characteristic.");
      statusSubscription.remove();
    };
  }, [ble.device]);

  const dispatchControlCode: ControlCodeDispatcher = async function ({
    controlCode,
    data,
    waitForResponse
  }) {
    const peripheral = ble.device;
    if (peripheral === null) {
      return false;
    }

    data = data ?? 0;
    waitForResponse = waitForResponse ?? false;

    const payload = Buffer.alloc(2);
    const pl = ((ControlCodes[controlCode] & 0xff) << 8) | (data & 0xff);

    payload.writeUint16BE(pl, 0);

    console.debug("Send control packet: " + payload.toString("hex"));

    try {
      if (waitForResponse) {
        await peripheral.writeCharacteristicWithResponseForService(
          SERVICE_UUID,
          CONTROL_UUID,
          payload.toString("base64")
        );
      } else {
        await peripheral.writeCharacteristicWithoutResponseForService(
          SERVICE_UUID,
          CONTROL_UUID,
          payload.toString("base64")
        );
      }

      return true;
    } catch (error) {
      console.error(`Error writing to control characteristic.`);
      console.error(payload.toString("hex"));
      console.error(error);
      return false;
    }
  };

  return [value, dispatchControlCode];
}
