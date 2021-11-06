import { dirname, basename, resolve, extname } from "path";

export class Path {
  fullPath: string;
  dirName: string;
  baseName: string;
  extension: string;

  constructor(path: string, dirName?: string, baseName?: string, extension?: string) {
    this.fullPath = resolve(path);
    this.dirName = dirName === undefined ? dirname(this.fullPath) : dirName;
    this.extension = extension === undefined ? extname(this.fullPath) : extension;
    this.baseName = baseName === undefined ? basename(this.fullPath, this.extension) : baseName;

    Object.prototype.toString = () => this.fullPath;
  }

  clone() {
    return new Path(this.fullPath, this.dirName, this.baseName, this.extension);
  }

  replaceExtension(newExtension: string) {
    if (newExtension[0] !== ".") {
      throw new Error("Extensions have to start with a dot!");
    }

    const p = this.clone();
    p.extension = newExtension;
    p.fullPath = resolve(p.dirName, p.baseName + p.extension);
    return p;
  }

  changeDirectory(newDirectory: string) {
    const p = this.clone();
    p.dirName = resolve(newDirectory);
    p.fullPath = resolve(p.dirName, p.baseName + p.extension);
    return p;
  }
}