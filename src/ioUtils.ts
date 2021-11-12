import fs from "fs";

import logger from "./logging";
import { FolderType } from "./fileOperations";
import { Path } from "./ioTypes";
import { RPGMakerInfo, RPGMakerPlatform } from "./rpgmakerTypes";
import { Stack } from "./javascriptDoesNotHaveAFuckingStack";
import { PathRegistry } from "./paths";

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

export function shouldEncryptFile(from: Path, encryptAudio: boolean, encryptImages: boolean, pathRegistry: PathRegistry): boolean {
  const ext = from.extension;
  if (ext !== ".png" && ext !== ".ogg" && ext !== ".m4a") {
    return false;
  }

  if (encryptAudio && (ext === ".ogg" || ext === ".m4a")) {
    return true;
  }

  if (encryptImages && ext === ".png") {
    if (from.parent.equals(pathRegistry.img_system)) {
      // these are for some reason not encrypted in MV
      if (from.fileName === "Loading.png") return false;
      if (from.fileName === "Window.png") return false;
      return true;
    }

    // game icon
    if (from.parent.equals(pathRegistry.icon) && from.fileName === "icon.png") {
      return false;
    }

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

export function isSameDevice(a: Path, b: Path): boolean {
  // works on both Windows and Unix
  const statsA = fs.statSync(a.fullPath);
  const statsB = fs.statSync(b.fullPath);

  const devA = statsA.dev;
  const devB = statsB.dev;

  return devA === devB;
}

export function* walkDirectoryRecursively(directory: Path) {
  const stack: Stack<Path> = new Stack();
  const items = fs.readdirSync(directory.fullPath, { encoding: "utf8" });
  for (const item of items) {
    const path = directory.join(item);
    if (path.isDir()) {
      stack.push(path);
    }

    yield path;
  }

  if (stack.size === 0) {
    return;
  }

  while (stack.peak) {
    const dir = stack.pop;
    const items = fs.readdirSync(dir.fullPath, { encoding: "utf8" });
    for (const item of items) {
      const path = dir.join(item);
      if (path.isDir()) {
        stack.push(path);
      }

      yield path;
    }
  }
}