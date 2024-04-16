import { StyleSheet, View } from "react-native";
import { Text } from "react-native-paper";
import { PropsWithChildren } from "react";

interface GroupBoxProps extends PropsWithChildren {
  title: string;
}

export default function GroupBox(props: GroupBoxProps) {
  return (
    <View style={styles.wrapper}>
      <Text style={styles.title}>{props.title}</Text>
      {props.children}
    </View>
  );
}

const styles = StyleSheet.create({
  wrapper: {
    borderColor: "#ccc",
    borderWidth: 1,
    padding: 12,
    paddingVertical: 18,
    position: "relative",
    borderRadius: 8
  },
  title: {
    position: "absolute",
    top: -12,
    left: 12,
    backgroundColor: "#f0f0f0",
    paddingHorizontal: 4
  }
});
