import { useRef, useState } from "react";
import { useInterval } from "./useInterval";

export function useCountdown(whenDone: () => void, interval: number) {
  const [count, setCount] = useState<number>(0);

  useInterval(() => {
    setCount(count - 1);

    if (count - 1 === 0) {
      whenDone();
    }
  }, count > 0 ? interval : null);
  
  const start = (value: number) => {
    setCount(value);
  }

  const cancel = () => {
    setCount(0);
  }

  return { count, setCount: start, cancel, isCounting: count > 0 };
}