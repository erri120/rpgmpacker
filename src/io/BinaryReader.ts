import fs from "fs";

import logger from "../logging";
import BinaryReaderWriter from "./BinaryReaderWriter";
import Path from "./Path";

export default class BinaryReader extends BinaryReaderWriter {
  constructor(file: Path) {
    super(file, "r");
  }

  public read(size: number): Buffer | null {
    if (this._fd === null) {
      logger.error(`Unable to from file ${this._file.fullPath} because the file descriptor is null!`);
      return null;
    }

    // using allocUnsafe because we don't care about the contents being initialized with zeroes
    const buffer = Buffer.allocUnsafe(size);
    const bytesRead = fs.readSync(this._fd, buffer, { length: buffer.byteLength });

    if (bytesRead !== buffer.byteLength) {
      logger.error(`Tried to read ${buffer.byteLength} bytes from file ${this._file.fullPath} but read ${bytesRead}`);
      this.close();
      return null;
    }

    return buffer;
  }

  public readUInt32(): number | null {
    const buffer = this.read(4);
    return buffer === null ? null : buffer.readUInt32LE();
  }
}