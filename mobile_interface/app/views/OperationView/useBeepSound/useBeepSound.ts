import { useEffect, useRef } from "react";
import { AVPlaybackStatusSuccess, Audio as ExpoAudio } from "expo-av";

interface MyAudioOptions {
  requiredAudioFile: any;
  willLoopAutomatically: boolean;
}

class MyAudio {
  private snd: ExpoAudio.Sound | null;

  public isCurrentlyPlaying: boolean;

  constructor(options: MyAudioOptions) {
    this.snd = null;
    this.isCurrentlyPlaying = false;

    ExpoAudio.Sound.createAsync(options.requiredAudioFile, {}, null, true).then(
      async ({ sound }) => {
        await sound.setIsLoopingAsync(options.willLoopAutomatically);
        sound.setOnPlaybackStatusUpdate((status) => {
          if (!status.isLoaded) return;
          this.onStatusUpdate(status);
        });
        this.snd = sound;
        this.onCreated();
      }
    );
  }

  private onCreated() {}

  private onStatusUpdate(status: AVPlaybackStatusSuccess) {
    this.isCurrentlyPlaying = status.isPlaying;
  }

  public play(forceReset: boolean) {
    if (this.isCurrentlyPlaying && !forceReset) {
      return;
    }

    this.snd!.playFromPositionAsync(0);
  }

  public stop() {
    if (!this.isCurrentlyPlaying) {
      return;
    }

    this.snd!.stopAsync();
  }
}

type SFX = "PercentualEEGAtingido" | "FESAtivado_hl2" | "FESDesligada";
const Soundboard: Map<SFX, MyAudio> = new Map();

Soundboard.set(
  "FESAtivado_hl2",
  new MyAudio({
    requiredAudioFile: require("./fesativado_hl2.ogg.wav"),
    willLoopAutomatically: false
  })
);

Soundboard.set(
  "FESDesligada",
  new MyAudio({
    requiredAudioFile: require("./fesdesativado.ogg.wav"),
    willLoopAutomatically: false
  })
);

Soundboard.set(
  "PercentualEEGAtingido",
  new MyAudio({
    requiredAudioFile: require("./percentualEEGatingido.ogg.wav"),
    willLoopAutomatically: false
  })
);

export function useBeepSound(id: SFX) {
  return Soundboard.get(id)!;
}
