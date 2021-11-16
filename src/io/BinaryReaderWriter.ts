import fs from "fs";

import logger from "../logging";
import { Path } from "./Path";

export class BinaryReaderWriter {
  protected _file: Path;
  protected _fd: number | null;

  constructor(file: Path, flags: string) {
    this._file = file;
    this._fd = fs.openSync(this._file.fullPath, flags, 0o666);
  }

  public stats(): fs.Stats | null {
    if (this._fd === null) {
      logger.error(`Unable to get status of file ${this._file.fullPath} because the file descriptor is null!`);
      return null;
    }

    return fs.fstatSync(this._fd);
  }

  public isClosed() {
    return this._fd === null;
  }

  public close() {
    logger.debug(`Closing file descriptor of file ${this._file.fullPath}`);

    if (this._fd === null) {
      logger.error(`Unable to close file descriptor of file ${this._file.fullPath} because the it is already closed!`);
    } else {
      fs.closeSync(this._fd);
      this._fd = null;
    }
  }
}