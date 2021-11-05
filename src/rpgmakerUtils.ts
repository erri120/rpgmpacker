import { RPGMakerPlatform, RPGMakerVersion, TemplateFolderName } from "./rpgmakerTypes";

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
        case RPGMakerPlatform.Linux: return "nwjs-linux";
        default: return null;
      }
    }
    default: return null;
  }
}