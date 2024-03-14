import { GestureResponderEvent } from "react-native";
import { useComponentRect } from "../../../hooks/useComponentSize";
import { clamp } from "../../../utils/clamp";
import { hapticFeedbackControl } from "../../../haptics/HapticFeedback";
import { useRef } from "react";

interface UseSlideGestureProps {
  maximumValue: number;
  currentValue: number;
  onSlide(newValue: number): void;
}

export function useSlideGesture({ currentValue, maximumValue, onSlide }: UseSlideGestureProps) {
  const [rect, viewRef] = useComponentRect();
  const touchOffsetY = useRef<number>(0);
  const touchStartValue = useRef<number>(0);

  function onTouchStart(event: GestureResponderEvent) {
    hapticFeedbackControl();
    touchOffsetY.current = event.nativeEvent.pageY;
    touchStartValue.current = currentValue;
  }

  function onTouchMove(event: GestureResponderEvent) {
    if (!rect) {
      return;
    }

    const valuePerPx = maximumValue / rect.height / 2;
    const distancePx = touchOffsetY.current - event.nativeEvent.pageY;
    const deltaValue = distancePx * valuePerPx;
    const newValue = touchStartValue.current + deltaValue;

    onSlide(clamp(0, newValue, maximumValue));
  }

  const sliderProps = {
    onStartShouldSetResponder: (evt: GestureResponderEvent) => true,
    onMoveShouldSetResponder: (evt: GestureResponderEvent) => true,
    onResponderGrant: onTouchStart,
    onResponderMove: onTouchMove,
    ref: viewRef
  };

  return { sliderProps };
}
