import { AVPlaybackStatusSuccess, Audio as ExpoAudio } from "expo-av";

interface MyAudioOptions {
  requiredAudioFile: any;
  willLoopAutomatically: boolean;
  /**
   * Número entre 0.0 (silêncio) e 1.0 (volume máximo)
   */
  volume?: number;
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
        if (options.volume !== undefined) {
          await sound.setVolumeAsync(options.volume);
        }
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

  public play(forceReset: boolean = false) {
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

type SFX =
  | "PesoAcimaSetpoint"
  | "EstimulacaoContinua"
  | "PercentualEEGAtingido"
  | "FESAtivado_hl2"
  | "FESDesligada";
const Soundboard: Map<SFX, MyAudio> = new Map();

Soundboard.set(
  "PesoAcimaSetpoint",
  new MyAudio({
    requiredAudioFile: require("./beep.wav"),
    willLoopAutomatically: true,
    volume: 0.01
  })
);

Soundboard.set(
  "EstimulacaoContinua",
  new MyAudio({
    requiredAudioFile: require("./EstimulacaoContinua.wav"),
    willLoopAutomatically: true
  })
);

Soundboard.set(
  "FESAtivado_hl2",
  new MyAudio({
    requiredAudioFile: require("./FESAtivado.wav"),
    willLoopAutomatically: false
  })
);

Soundboard.set(
  "FESDesligada",
  new MyAudio({
    requiredAudioFile: require("./FESDesativado.wav"),
    willLoopAutomatically: false
  })
);

Soundboard.set(
  "PercentualEEGAtingido",
  new MyAudio({
    requiredAudioFile: require("./PercentualEEGatingido.wav"),
    willLoopAutomatically: false
  })
);

export function useBeepSound(id: SFX) {
  return Soundboard.get(id)!;
}
