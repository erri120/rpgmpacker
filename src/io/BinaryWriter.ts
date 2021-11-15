import fs from "fs";

import logger from "../logging";
import { BinaryReaderWriter } from "./BinaryReaderWriter";
import { Path } from "../ioTypes";

export class BinaryWriter extends BinaryReaderWriter {
  constructor(file: Path) {
    super(file, "w+");
  }

  public write(buffer: NodeJS.ArrayBufferView) {
    if (this._fd === null) {
      logger.error(`Unable to write to file ${this._file.fullPath} because the file descriptor is null!`);
      return false;
    }

    const bytesWritten = fs.writeSync(this._fd, buffer, 0, buffer.byteLength);

    if (bytesWritten != buffer.byteLength) {
      logger.error(`Tried to write ${buffer.byteLength} bytes to file ${this._file.fullPath} but wrote ${bytesWritten}`);
      this.close();
      return false;
    }

    return true;
  }
}