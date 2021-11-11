import { dirname, basename, resolve, extname } from "path";
import { accessSync, constants, statSync } from "fs";

export class Path {
  fullPath: string;
  parent: Path;
  baseName: string;
  extension: string;
  fileName: string;

  constructor(path: string, parent?: Path, baseName?: string, extension?: string) {
    this.fullPath = resolve(path);

    // dirname("C:\\") -> "C:\\"
    // dirname("/") -> "/"
    const dirName = dirname(this.fullPath);
    if (dirName === this.fullPath) {
      this.parent = this;
    } else {
      this.parent = parent === undefined ? new Path(dirname(this.fullPath)) : parent;
    }

    this.extension = (extension === undefined ? extname(this.fullPath) : extension).toLowerCase();
    this.baseName = baseName === undefined ? basename(this.fullPath, this.extension) : baseName;
    this.fileName = this.baseName + this.extension;

    Object.prototype.toString = () => this.fullPath;
  }

  isFile(): boolean {
    const stats = statSync(this.fullPath);
    return stats.isFile();
  }

  isDir(): boolean {
    const stats = statSync(this.fullPath);
    return stats.isDirectory();
  }

  clone(): Path {
    return new Path(this.fullPath, this.parent, this.baseName, this.extension);
  }

  replaceExtension(newExtension: string): Path {
    if (newExtension[0] !== ".") {
      throw new Error("Extensions have to start with a dot!");
    }

    const p = this.clone();
    p.extension = newExtension.toLowerCase();
    p.fileName = p.baseName + p.extension;
    p.fullPath = resolve(p.parent.fullPath, p.fileName);
    return p;
  }

  changeDirectory(newDirectory: string): Path {
    const parent = new Path(newDirectory);
    return parent.join(this.fileName);
    // const p = this.clone();
    // p.parent = new Path(newDirectory);
    // p.fullPath = resolve(p.dirName, p.fileName);
    // return p;
  }

  isInDirectory(dir: Path): boolean {
    if (this.parent.equals(dir)) return true;
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
    if (!this.isDir()) {
      throw new Error("Unable to join paths because this is not a directory!");
    }

    const newPath = resolve(this.fullPath, s);
    return new Path(newPath);
  }

  relativeTo(other: Path): string {
    // +1 because of path separator
    return this.fullPath.substring(other.fullPath.length + 1);
  }

  equals(other: Path): boolean {
    return this.fullPath === other.fullPath;
  }
}