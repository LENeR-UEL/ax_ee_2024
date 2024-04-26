import { useEffect, useState } from "react";
import { fullUUID, BleError, BleErrorCode, Characteristic } from "react-native-ble-plx";
import { Buffer } from "buffer";
import { useBluetoothConnection } from "./Context";
import { BufferReader } from "./bufferReader";

const SERVICE_UUID = fullUUID("ab04");
const STATUS_UUID = fullUUID("ff01");
const CONTROL_UUID = fullUUID("ff0f");

export enum FirmwareState {
  Disconnected,
  ParameterSetup,
  ParallelWeight,
  MESECollecter,
  OperationStart,
  OperationGradualIncrease,
  OperationTransition,
  OperationMalhaFechada,
  OperationStop
}

interface StatusPacket {
  pwm: number;
  weightL: number;
  weightR: number;
  collectedWeight: number;
  mese: number;
  meseMax: number;
  setpoint: number;
  isOVBoxFlagSet: boolean;
  mainOperationState:
    | null
    | {
        state: FirmwareState.OperationStart;
      }
    | { state: FirmwareState.OperationGradualIncrease; pwmIncreaseTimeDelta: number }
    | {
        state: FirmwareState.OperationTransition;
        timeDelta: number;
      }
    | {
        state: FirmwareState.OperationMalhaFechada;
        currentErrorValue: number;
        errorPositiveTimer: number;
      }
    | { state: FirmwareState.OperationStop; pwmDecreaseTimeDelta: number };
  parameters: {
    gradualIncreaseTime: number;
    transitionTime: number;
    gradualDecreaseTime: number;
    malhaFechadaAboveSetpointTime: number;
  };
}

const ControlCodes = {
  /**
   * Invoca um reset no gateway e no estimulador.
   */
  FirmwareInvokeReset: 0x00,

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
  MainOperation_EmergencyStop: 0x38,

  ParameterSetup_SetGradualIncreaseTime: 0x61,
  ParameterSetup_SetTransitionTime: 0x63,
  ParameterSetup_SetGradualDecreaseTime: 0x64,
  ParameterSetup_SetMalhaFechadaAboveSetpointTime: 0x66,
  ParameterSetup_Reset: 0x6d,
  ParameterSetup_Save: 0x6e,
  ParameterSetup_Complete: 0x6f
} as const;

type ControlCodeDispatcher = (options: {
  controlCode: keyof typeof ControlCodes;
  waitForResponse: boolean;
  data?: number;
}) => Promise<boolean>;

function parseStatusPacket(packet: Buffer): StatusPacket {
  console.log(packet.byteLength + " bytes");
  const reader = new BufferReader(packet);

  const pwm = reader.readUnsignedShortLE();
  const weightL = reader.readUnsignedChar();
  const weightR = reader.readUnsignedChar();
  const collectedWeight = reader.readUnsignedShortLE();
  const mese = reader.readUnsignedShortLE();
  const meseMax = reader.readUnsignedShortLE();
  const setpoint = reader.readUnsignedShortLE();
  const isOVBoxFlagSet = reader.readUnsignedChar() === 1;

  const parameters: StatusPacket["parameters"] = {
    gradualIncreaseTime: reader.readUnsignedShortLE(),
    transitionTime: reader.readUnsignedShortLE(),
    gradualDecreaseTime: reader.readUnsignedShortLE(),
    malhaFechadaAboveSetpointTime: reader.readUnsignedShortLE()
  };

  const state = reader.readUnsignedChar() as FirmwareState;
  let mainOpStateObj: StatusPacket["mainOperationState"];

  switch (state) {
    case FirmwareState.OperationStart: {
      mainOpStateObj = {
        state: FirmwareState.OperationStart
      };
      break;
    }
    case FirmwareState.OperationGradualIncrease: {
      mainOpStateObj = {
        state: FirmwareState.OperationGradualIncrease,
        pwmIncreaseTimeDelta: reader.readUnsignedShortLE()
      };
      break;
    }
    case FirmwareState.OperationTransition: {
      mainOpStateObj = {
        state: FirmwareState.OperationTransition,
        timeDelta: reader.readUnsignedShortLE()
      };
      break;
    }
    case FirmwareState.OperationMalhaFechada: {
      mainOpStateObj = {
        state: FirmwareState.OperationMalhaFechada,
        currentErrorValue: reader.readShortLE(),
        errorPositiveTimer: reader.readUnsignedShortLE()
      };
      break;
    }
    case FirmwareState.OperationStop: {
      mainOpStateObj = {
        state: FirmwareState.OperationStop,
        pwmDecreaseTimeDelta: reader.readUnsignedShortLE()
      };
      break;
    }
    default:
      mainOpStateObj = null;
  }

  return {
    pwm,
    weightL,
    weightR,
    collectedWeight,
    mese,
    meseMax,
    setpoint,
    isOVBoxFlagSet,
    parameters,
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
    isOVBoxFlagSet: false,
    mainOperationState: null,
    parameters: {
      gradualIncreaseTime: 0,
      transitionTime: 0,
      gradualDecreaseTime: 0,
      malhaFechadaAboveSetpointTime: 0
    }
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
