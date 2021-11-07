import { dirname, basename, resolve, extname } from "path";
import { accessSync, constants } from "fs";

export class Path {
  fullPath: string;
  dirName: string;
  baseName: string;
  extension: string;
  isFile: boolean;
  isDir: boolean;

  constructor(path: string, dirName?: string, baseName?: string, extension?: string) {
    this.fullPath = resolve(path);
    this.dirName = dirName === undefined ? dirname(this.fullPath) : dirName;
    this.extension = (extension === undefined ? extname(this.fullPath) : extension).toLowerCase();
    this.baseName = baseName === undefined ? basename(this.fullPath, this.extension) : baseName;

    this.isFile = this.extension !== "";
    this.isDir = this.extension === "";

    Object.prototype.toString = () => this.fullPath;
  }

  clone(): Path {
    return new Path(this.fullPath, this.dirName, this.baseName, this.extension);
  }

  replaceExtension(newExtension: string): Path {
    if (newExtension[0] !== ".") {
      throw new Error("Extensions have to start with a dot!");
    }

    const p = this.clone();
    p.extension = newExtension.toLowerCase();
    p.fullPath = resolve(p.dirName, p.baseName + p.extension);
    return p;
  }

  changeDirectory(newDirectory: string): Path {
    const p = this.clone();
    p.dirName = resolve(newDirectory);
    p.fullPath = resolve(p.dirName, p.baseName + p.extension);
    return p;
  }

  isInDirectory(dir: Path): boolean {
    if (this.dirName === dir.fullPath) return true;
    return this.fullPath.startsWith(dir.fullPath);
  }

  exists(): boolean {
    try {
      accessSync(this.fullPath, constants.W_OK);
      return true;
    } catch {
      return false;
    }
  }

  join(s: string): Path {
    if (!this.isDir) {
      throw new Error("Unable to join paths because this is not a directory!");
    }

    const newPath = resolve(this.fullPath, s);
    return new Path(newPath);
  }

  relativeTo(other: Path): string {
    // +1 because of path separator
    return this.fullPath.substring(other.fullPath.length + 1);
  }
}