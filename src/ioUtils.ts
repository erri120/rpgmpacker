import fs, { constants } from "fs";

import logger from "./logging";
import { FolderType } from "./fileOperations";
import { Path } from "./ioTypes";
import { RPGMakerInfo, RPGMakerPlatform } from "./rpgmakerTypes";

export function shouldFilterFile(from: Path, folder: FolderType, rpgmakerInfo: RPGMakerInfo): boolean {
  switch (folder) {
  // TODO:
  case FolderType.TemplateFolder: return false;
  case FolderType.ProjectFolder: {
    // Desktop: only ogg
    // Mobile: only m4a
    // Browser: both

    if (rpgmakerInfo.Platform === RPGMakerPlatform.Mobile && from.extension === ".ogg") {
      return true;
    }

    if (from.extension === ".m4a" && rpgmakerInfo.Platform !== RPGMakerPlatform.Mobile && rpgmakerInfo.Platform !== RPGMakerPlatform.Browser) {
      return true;
    }

    // skip project files
    if (from.extension === ".rpgproject" || from.extension === ".rmmzproject") {
      return true;
    }
  }
  }

  return false;
}

export function shouldEncryptFile(from: Path, encryptAudio: boolean, encryptImages: boolean): boolean {
  const ext = from.extension;
  if (ext !== ".png" && ext !== ".ogg" && ext !== ".m4a") {
    return false;
  }

  if (encryptAudio && (ext === ".ogg" || ext === ".m4a")) {
    return true;
  }

  if (encryptImages && ext === ".png") {
    return true;
  }

  return false;
}

export function transferFile(from: Path, to: Path, useHardlink: boolean) {
  if (useHardlink) {
    logger.debug(`Creating hardlink from ${from} to ${to}`);
    if (to.exists()) {
      fs.rmSync(to.fullPath);
    }
    fs.linkSync(from.fullPath, to.fullPath);
  } else {
    logger.debug(`Copying file from ${from} to ${to}`);
    fs.copyFileSync(from.fullPath, to.fullPath);
  }
}