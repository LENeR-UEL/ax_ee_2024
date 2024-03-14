import { LegacyRef, Ref, useEffect, useRef, useState } from "react";
import { View } from "react-native";

type Rect = { width: number; height: number, x: number, y: number };

export const useComponentRect = (): [Rect | null, Ref<View>] => {
  const [rect, setRect] = useState<null | Rect>(null);
  const measureViewRef = useRef<View>(null);

  useEffect(() => {
    if (!measureViewRef.current) return;

    measureViewRef.current.measure((x, y, width, height, pageX, pageY) => {
      setRect({ x, y: pageY, width, height });
    });
  }, [measureViewRef.current]);

  return [rect, measureViewRef];
};
