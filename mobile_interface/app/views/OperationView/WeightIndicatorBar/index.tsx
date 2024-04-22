import { View, StyleSheet, ViewStyle, StyleProp } from "react-native";
import { Text } from "react-native-paper";
import { run } from "../../../utils/run";
import { useSlideGesture } from "./useSlideGesture";

interface Props {
  style?: StyleProp<ViewStyle> | StyleProp<ViewStyle>[];
  textTop?: string;
  maximumValue: number;
  value: number;
  setpointValue: number;
  onSetpointChange(newValue: number): void;
  setpointColor: string;
  hideSetpointText: boolean;
  fillColor: string;
}

export function WeightIndicationBar(props: Props) {
  const { sliderProps } = useSlideGesture({
    currentValue: props.setpointValue,
    maximumValue: props.maximumValue,
    onSlide: props.onSetpointChange
  });

  const fillHeight = Math.round((props.value / props.maximumValue) * 100);

  return (
    <View style={StyleSheet.compose(styles.box, props.style)} {...sliderProps}>
      <View
        style={StyleSheet.compose(styles.boxFill, {
          height: `${fillHeight}%`,
          backgroundColor: props.fillColor
        })}
        pointerEvents="none"
      />
      {run(() => {
        const setpointHeight = (props.setpointValue / props.maximumValue) * 100;

        return (
          <View
            style={StyleSheet.compose(styles.setpointBar, {
              bottom: `${setpointHeight}%`,
              backgroundColor: props.setpointColor
            })}
            pointerEvents="none">
            {!props.hideSetpointText && (
              <Text
                style={StyleSheet.compose(styles.setpointText, {
                  color: props.setpointColor
                })}>
                {(Math.round(props.setpointValue * 100) / 100).toFixed(0)} kg
              </Text>
            )}
          </View>
        );
      })}
      <Text style={styles.labelTop}>{props.textTop}</Text>
      <Text style={styles.labelMain}>{props.value} kg</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  box: {
    backgroundColor: "#323232",
    elevation: 4,
    position: "relative",
    height: 180,
    width: 72,
    justifyContent: "center",
    alignItems: "center",
    borderRadius: 0,
    overflow: "hidden"
  },
  boxFill: {
    position: "absolute",
    bottom: 0,
    left: 0,
    width: "100%",
    height: 0,
    backgroundColor: "darkred"
  },
  setpointBar: {
    position: "absolute",
    left: 0,
    bottom: 0,
    width: "100%",
    height: 1,
    backgroundColor: "red"
  },
  setpointText: {
    position: "absolute",
    right: "0%",
    top: -16,
    fontFamily: "monospace"
  },
  labelTop: {
    position: "absolute",
    fontFamily: "monospace",
    fontSize: 16,
    color: "white",
    textAlign: "center",
    top: 16
  },
  labelMain: {
    position: "absolute",
    fontFamily: "monospace",
    fontSize: 24,
    color: "white",
    flexGrow: 1,
    textAlign: "center"
  }
});
