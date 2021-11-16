import { dirname, basename, resolve, extname } from "path";
import { accessSync, constants, statSync } from "fs";

export class Path {
  private _fullPath: string;
  private _dirName: string;
  private _baseName: string;
  private _extension: string;
  private _fileName: string;

  constructor(path: string, dirName?: string, baseName?: string, extension?: string) {
    this._fullPath = resolve(path);
    this._dirName = dirName === undefined ? dirname(this._fullPath) : dirName;
    this._extension = (extension === undefined ? extname(this._fullPath) : extension).toLowerCase();
    this._baseName = baseName === undefined ? basename(this._fullPath, this._extension) : baseName;
    this._fileName = this._baseName + this._extension;
  }

  public exists(): boolean {
    try {
      accessSync(this._fullPath, constants.W_OK);
      return true;
    } catch {
      return false;
    }
  }

  public isFile(): boolean {
    const stats = statSync(this._fullPath);
    return stats.isFile();
  }

  public isDir(): boolean {
    const stats = statSync(this._fullPath);
    return stats.isDirectory();
  }

  public replaceExtension(newExtension: string): Path {
    const p = this.clone();
    p._extension = newExtension.toLowerCase();
    p._fileName = p._baseName + p._extension;
    p._fullPath = resolve(p._dirName, p._fileName);
    return p;
  }

  public replaceName(newName: string): Path {
    const p = this.clone();
    p._baseName = newName;
    p._fileName = p._baseName + p._extension;
    p._fullPath = resolve(p._dirName, p._fileName);
    return p;
  }

  public replaceFileName(newFileName: string): Path {
    const p = this.clone();
    p._extension = extname(newFileName);
    p._baseName = basename(newFileName, p._extension);
    p._fileName = newFileName;
    p._fullPath = resolve(p._dirName, p._fileName);
    return p;
  }

  public join(...pathSegments: string[]): Path {
    const newPath = resolve(this._fullPath, ...pathSegments);
    return new Path(newPath);
  }

  // public changeDirectory(newDirectory: string): Path {
  //   const parent = new Path(newDirectory);
  //   return parent.join(this._fileName);
  // }

  public isInDirectory(dir: Path): boolean {
    if (this._dirName === dir._fullPath) return true;
    return this._fullPath.startsWith(dir._fullPath);
  }

  public relativeTo(other: Path): string {
    // +1 because of path separator
    return this._fullPath.substring(other._fullPath.length + 1);
  }

  public equals(other: Path): boolean {
    return this._fullPath === other._fullPath;
  }

  public clone(): Path {
    return new Path(this._fullPath, this._dirName, this._baseName, this._extension);
  }

  public get fullPath(): string { return this._fullPath; }
  public get dirName(): string { return this._dirName; }
  public get baseName(): string { return this._baseName; }
  public get extension(): string { return this._extension; }
  public get fileName(): string { return this._fileName; }

  public getParent(): Path {
    // dirname("C:\\") -> "C:\\"
    // dirname("/") -> "/"
    if (this._dirName === this._fullPath) {
      return this;
    }

    return new Path(this._dirName);
  }
}