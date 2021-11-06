export enum RPGMakerVersion {
  MV = "RPG Maker MV",
  MZ = "RPG Maker MZ"
}

export enum RPGMakerPlatform {
  Windows = "Windows",
  OSX = "OSX",
  Linux = "Linux",
  Browser = "Browser",
  Mobile = "Mobile"
}

export interface RPGMakerInfo {
  Version: RPGMakerVersion,
  Platform: RPGMakerPlatform
}

export type TemplateFolderName = "nwjs-win" | "nwjs-osx-unsigned" | "nwjs-lnx" | "nwjs-mac" | "nwjs-linux";