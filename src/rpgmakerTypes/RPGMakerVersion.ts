import fs from "fs";
import path from "path";

import logger from "../logging";
import Path from "../io/Path";

export enum RPGMakerVersion {
  MV = "RPG Maker MV",
  MZ = "RPG Maker MZ"
}

export function identifyRPGMakerVersion(input: Path): RPGMakerVersion | null {
  logger.debug(`Identifying RPG Maker version in ${input.fullPath}`);

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

  logger.error(`Unable to find a RPG Maker project file in ${input.fullPath}`);
  return null;
}