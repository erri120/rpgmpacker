import fs from "fs";

import logger from "../logging";
import Path from "./Path";
import Stack from "../other/Stack";

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