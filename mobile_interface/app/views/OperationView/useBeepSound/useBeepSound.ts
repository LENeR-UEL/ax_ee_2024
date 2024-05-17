import { useEffect, useRef } from "react";
import { Audio } from "expo-av";

type SFX = "PercentualEEGAtingido" | "FESAtivado_hl2" | "FESDesligada";
const Soundboard: Map<SFX, Audio.Sound> = new Map();

Audio.Sound.createAsync(require("./fesativado_hl2.ogg.wav"), {}, null, true).then(({ sound }) => {
  sound.setIsLoopingAsync(false);
  Soundboard.set("FESAtivado_hl2", sound);
});

Audio.Sound.createAsync(require("./fesdesativado.ogg.wav"), {}, null, true).then(({ sound }) => {
  sound.setIsLoopingAsync(false);
  Soundboard.set("FESDesligada", sound);
});

Audio.Sound.createAsync(require("./percentualEEGatingido.ogg.wav"), {}, null, true).then(
  ({ sound }) => {
    sound.setIsLoopingAsync(false);
    Soundboard.set("PercentualEEGAtingido", sound);
  }
);

export function useBeepSound(id: SFX) {
  const isAlreadyPlaying = useRef(false);

  useEffect(() => {
    return () => {
      if (!Soundboard.has(id)) return;
      Soundboard.get(id)!.stopAsync();
    };
  }, []);

  function play(immediate = false) {
    if (!Soundboard.has(id)) return;
    if (isAlreadyPlaying.current && immediate === false) return;

    isAlreadyPlaying.current = true;
    Soundboard.get(id)!.playFromPositionAsync(0);
  }

  function stop() {
    if (!Soundboard.has(id)) return;
    isAlreadyPlaying.current = false;
    Soundboard.get(id)!.stopAsync();
  }

  return { play, stop };
}
