import { useEffect, useRef } from "react";
import { Audio } from "expo-av";

let $sound: Audio.Sound | null = null;

Audio.Sound.createAsync(require("./beep.wav"), {}, null, true)
  .then(({ sound }) => {
    sound.setIsLoopingAsync(true);
    $sound = sound;
  })
  .catch((error) => {
    console.error(error); ////
  });

export function useBeepSound() {
  const $playing = useRef(false);

  useEffect(() => {
    return () => {
      if ($sound === null) return;
      $sound.stopAsync();
    };
  }, []);

  function play() {
    console.log("Beep");
    if ($sound === null || $playing.current) return;
    $playing.current = true;
    $sound.playFromPositionAsync(0);
  }

  function stop() {
    if ($sound === null) return;
    $playing.current = false;
    $sound.stopAsync();
  }

  return { play, stop };
}
