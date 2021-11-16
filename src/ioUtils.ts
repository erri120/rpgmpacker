import fs from "fs";

import logger from "./logging";
import { Path } from "./io/Path";
import { FolderType } from "./fileOperations";
import { RPGMakerInfo, RPGMakerPlatform, RPGMakerVersion } from "./rpgmakerTypes";
import { Stack } from "./other/Stack";
import { ProjectPathRegistry, TemplatePathRegistry } from "./rpgmaker/pathRegistries";

export function shouldFilterFile(from: Path, folder: FolderType, rpgmakerInfo: RPGMakerInfo, pathRegistry: ProjectPathRegistry, templatePathRegistry: TemplatePathRegistry | undefined): boolean {
  switch (folder) {
  case FolderType.TemplateFolder: {
    if (rpgmakerInfo.Version !== RPGMakerVersion.MZ) {
      return false;
    }

    if (templatePathRegistry === undefined) {
      // TODO: error or something
      return false;
    }

    if (rpgmakerInfo.Platform === RPGMakerPlatform.Windows) {
      // portable native client is apparently not used
      if (from.isInDirectory(templatePathRegistry.pnacl)) {
        return true;
      }

      // other chromium related files are also ignored
      if (from.getParent().equals(templatePathRegistry.top)) {
        if (from.fileName === "chromedriver.exe") return true;
        if (from.fileName === "nacl_irt_x86_64.nexe") return true;
        if (from.fileName === "nwjc.exe") return true;
        if (from.fileName === "payload.exe") return true;
      }
    } else if (rpgmakerInfo.Platform === RPGMakerPlatform.OSX) {
      if (from.getParent().equals(templatePathRegistry.top)) {
        if (from.fileName === "chromedriver") return true;
        if (from.fileName === "libffmpeg.dylib") return true;
        if (from.fileName === "minidump_stackwalk") return true;
        if (from.fileName === "nwjc") return true;
        if (from.fileName === "payload") return true;
        if (from.fileName === "v8_context_snapshot.bin") return true;
      }
    }

    return false;
  }
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

export function transferFile(from: Path, to: Path, useHardlink: boolean) {
  if (useHardlink) {
    logger.debug(`Creating hardlink from ${from.fullPath} to ${to.fullPath}`);
    if (to.exists()) {
      fs.rmSync(to.fullPath);
    }
    fs.linkSync(from.fullPath, to.fullPath);
  } else {
    logger.debug(`Copying file from ${from.fullPath} to ${to.fullPath}`);
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

  while (stack.peak()) {
    const dir = stack.pop();
    try {
      const items = fs.readdirSync(dir.fullPath, { encoding: "utf8" });
      for (const item of items) {
        const path = dir.join(item);
        if (path.isDir()) {
          stack.push(path);
        }

        yield path;
      }
    } catch {
      // ignored
    }
  }
}

export function removeEmptyFolders(path: Path) {
  for (const p of walkDirectoryRecursively(path)) {
    if (p.exists() && p.isDir()) {
      const items = fs.readdirSync(p.fullPath, { encoding: "utf8" });
      if (items.length === 0) {
        fs.rmdirSync(p.fullPath);
      }
    }
  }
}