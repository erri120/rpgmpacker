import fs from "fs";
import path from "path";

import { Path } from "./ioTypes";
import logger from "./logging";
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

export function identifyRPGMakerVersion(input: Path): RPGMakerVersion | null {
  logger.debug(`Identifying RPG Maker version in ${input}`);

  const dirents = fs.readdirSync(input.fullPath, { encoding: "utf8", withFileTypes: true });
  for (let i = 0; i < dirents.length; i++) {
    const dirent = dirents[i];
    if (!dirent.isFile()) continue;

    const ext = path.extname(dirent.name).toLowerCase();
    if (ext === ".rpgproject") {
      logger.debug(`Found MV project file ${dirent.name}`);
      return RPGMakerVersion.MV;
    }

    if (ext === ".rmmzproject") {
      logger.debug(`Found MZ project file ${dirent.name}`);
      return RPGMakerVersion.MZ;
    }
  }

  logger.error(`Unable to find a RPG Maker project file in ${input}`);
  return null;
}