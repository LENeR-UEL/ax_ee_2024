import { useNavigation as _useNavigation } from "@react-navigation/native";
import type { StackNavigation } from "../Routing";

export const useNavigation = () => _useNavigation<StackNavigation>();
