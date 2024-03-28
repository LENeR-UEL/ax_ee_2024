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
  mainOperationState:
    | null
    | {
        state: "START_WAIT_FOR_ZERO";
        currentWeightClass: number;
      }
    | { state: "START_WAIT_FOR_WEIGHT_SETPOINT" }
    | { state: "GRADUAL_INCREMENT" }
    | { state: "TRANSITION"; currentWeightClass: number }
    | { state: "ACTION_CONTROL"; currentErrorValue: number }
    | { state: "GRADUAL_DECREMENT" }
    | { state: "STOPPED" };
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

enum MainOperationState {
  _NOT_IN_MAIN_OPERATION = 0,

  // Aguardar peso medido zerar (classe 0) para podermos iniciar a operação
  START_WAIT_FOR_ZERO = 5,
  // Voluntário sentado... Aguardar peso nas barras = setpoint
  START_WAIT_FOR_WEIGHT_SETPOINT,
  // Aumento gradual do PWM, em malha aberta, de 0 até MESE
  GRADUAL_INCREMENT,
  // Voluntário de pé, mas ainda se equilibrando com as barras. Aguardar classe 0 por 2 segundos, significando que ele está de pé e equilibrado
  TRANSITION,
  // Estimulação contínua, malha fechada, entre MESE e MESE_MAX. Aguardar fadiga do voluntário (classe 2 por 2s)
  ACTION_CONTROL,
  // Decremento gradual do PWM, do valor atual até 0
  GRADUAL_DECREMENT,
  // Operação finalizada
  STOPPED
}

type ControlCodeDispatcher = (options: {
  controlCode: keyof typeof ControlCodes;
  waitForResponse: boolean;
  data?: number;
}) => Promise<boolean>;

function parseStatusPacket(packet: Buffer): StatusPacket {
  let mainOpStateObj: StatusPacket["mainOperationState"];

  const mainOpState: MainOperationState = packet.readUint8(12);

  if (mainOpState === MainOperationState._NOT_IN_MAIN_OPERATION) {
    mainOpStateObj = null;
  } else if (mainOpState === MainOperationState.START_WAIT_FOR_ZERO) {
    mainOpStateObj = { state: "START_WAIT_FOR_ZERO", currentWeightClass: packet.readUint8(13) };
  } else if (mainOpState === MainOperationState.START_WAIT_FOR_WEIGHT_SETPOINT) {
    mainOpStateObj = { state: "START_WAIT_FOR_WEIGHT_SETPOINT" };
  } else if (mainOpState === MainOperationState.GRADUAL_INCREMENT) {
    mainOpStateObj = { state: "GRADUAL_INCREMENT" };
  } else if (mainOpState === MainOperationState.TRANSITION) {
    mainOpStateObj = { state: "TRANSITION", currentWeightClass: packet.readUint8(13) };
  } else if (mainOpState === MainOperationState.ACTION_CONTROL) {
    mainOpStateObj = { state: "ACTION_CONTROL", currentErrorValue: packet.readInt16BE(13) };
  } else if (mainOpState === MainOperationState.GRADUAL_DECREMENT) {
    mainOpStateObj = { state: "GRADUAL_DECREMENT" };
  } else if (mainOpState === MainOperationState.STOPPED) {
    mainOpStateObj = { state: "STOPPED" };
  } else {
    throw new Error(`Unexpected main operation state: ${mainOpState}`);
  }

  return {
    pwm: packet.readUint16LE(0),
    weightL: packet.readUint8(2),
    weightR: packet.readUint8(3),
    collectedWeight: packet.readUint16LE(4),
    mese: packet.readUint16LE(6),
    meseMax: packet.readUint16LE(8),
    setpoint: packet.readUint16LE(10),
    mainOperationState: mainOpStateObj
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
    setpoint: 0,
    mainOperationState: null
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
      console.debug(parsed.mainOperationState);

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
