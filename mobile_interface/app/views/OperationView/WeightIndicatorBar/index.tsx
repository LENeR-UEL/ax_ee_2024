import { View, StyleSheet, ViewStyle, StyleProp } from "react-native";
import { Text } from "react-native-paper";
import { run } from "../../../utils/run";
import { useSlideGesture } from "./useSlideGesture";

export interface OverlayBar {
  label?: string;
  showAtWeight: number;
  color: string;
}

export interface OverlayRectangle {
  weightRange: [number, number];
  color: string;
}

interface Props {
  style?: StyleProp<ViewStyle> | StyleProp<ViewStyle>[];
  textTop?: string;
  maximumValue: number;
  value: number;
  bars: OverlayBar[];
  rectangles: OverlayRectangle[];
  setpointValue: number;
  onSetpointChange(newValue: number): void;
  hideBarText?: boolean;
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
      {props.rectangles.map((rectangle, index) => {
        const y =
          rectangle.weightRange[1] > rectangle.weightRange[0]
            ? rectangle.weightRange[0]
            : rectangle.weightRange[1];
        const height = Math.abs(rectangle.weightRange[0] - rectangle.weightRange[1]);

        return (
          <View
            key={index}
            //@ts-ignore
            style={{
              position: "absolute",
              bottom: (y / props.maximumValue) * 100 + "%",
              left: 0,
              width: "100%",
              height: (Math.abs(height) / props.maximumValue) * 100 + "%",
              backgroundColor: rectangle.color,
              overflow: "hidden",
              display: "flex",
              justifyContent: "center",
              alignItems: "center"
            }}>
            <Text>Valor do Erro: {height} kg</Text>
          </View>
        );
      })}
      {props.bars.map((bar, index) => {
        const setpointHeight = (bar.showAtWeight / props.maximumValue) * 100;

        return (
          <View
            key={index}
            style={StyleSheet.compose(styles.bar, {
              bottom: `${setpointHeight}%`,
              backgroundColor: bar.color
            })}
            pointerEvents="none">
            {props.hideBarText && bar.label && (
              <Text
                style={StyleSheet.compose(styles.setpointLabelText, {
                  color: bar.color
                })}>
                {bar.label}
              </Text>
            )}
            {!props.hideBarText && (
              <Text
                style={StyleSheet.compose(styles.setpointValueText, {
                  color: bar.color
                })}>
                {(Math.round(bar.showAtWeight * 100) / 100).toFixed(0)} kg
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
    borderRadius: 8,
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
  bar: {
    position: "absolute",
    left: 0,
    bottom: 0,
    width: "100%",
    height: 1,
    backgroundColor: "red"
  },
  setpointLabelText: {
    position: "absolute",
    left: 4,
    top: -16,
    fontFamily: "monospace"
  },
  setpointValueText: {
    position: "absolute",
    right: 4,
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
