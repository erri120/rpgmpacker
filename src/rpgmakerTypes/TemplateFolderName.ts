import RPGMakerPlatform from "./RPGMakerPlatform";
import { RPGMakerVersion } from "./RPGMakerVersion";

export type TemplateFolderName = "nwjs-win" | "nwjs-osx-unsigned" | "nwjs-lnx" | "nwjs-mac" | "nwjs-linux";

export function getTemplateFolderName(version: RPGMakerVersion, platform: RPGMakerPlatform): TemplateFolderName | null {
  switch (version) {
  case RPGMakerVersion.MV: {
    switch (platform) {
    case RPGMakerPlatform.Windows: return "nwjs-win";
    case RPGMakerPlatform.OSX: return "nwjs-osx-unsigned";
    case RPGMakerPlatform.Linux: return "nwjs-lnx";
    default: return null;
    }
  }
  case RPGMakerVersion.MZ: {
    switch (platform) {
    case RPGMakerPlatform.Windows: return "nwjs-win";
    case RPGMakerPlatform.OSX: return "nwjs-mac";
    // NOTE: MZ doesn't support Linux out of the box, look at the REAMDE for more info
    case RPGMakerPlatform.Linux: return "nwjs-linux";
    default: return null;
    }
  }
  default: return null;
  }
}