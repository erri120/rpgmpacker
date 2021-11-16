import { before, it, describe } from "mocha";
import { expect } from "chai";

import logger, { Level } from "../../src/logging";
import Path from "../../src/io/Path";
import BinaryReader from "../../src/io/BinaryReader";
import BinaryWriter from "../../src/io/BinaryWriter";

describe("BinaryReaderWriter", () => {
  before(() => {
    logger.setMinLevel(Level.SILENT);
  });

  it("can write and read a buffer", () => {
    const path = new Path("./test-output/BinaryReaderWriterOutputBuffer.bin");

    const stringInput = "Hello World";
    const buffer = Buffer.from(stringInput, "ascii");

    const bw = new BinaryWriter(path);
    expect(bw.write(buffer)).to.be.true;
    bw.close();
    expect(bw.isClosed()).to.be.true;

    const br = new BinaryReader(path);
    const readBuffer = br.read(buffer.byteLength);
    expect(readBuffer).to.be.not.null;
    br.close();
    expect(br.isClosed()).to.be.true;

    const stringOutput = readBuffer!.toString("ascii");
    expect(stringInput).to.equal(stringOutput);
  });

  it("can write and read UInt32", () => {
    const path = new Path("./test-output/BinaryReaderWriterOutputUInt32.bin");

    const bw = new BinaryWriter(path);
    expect(bw.writeUInt32(0x1)).to.be.true;
    bw.close();
    expect(bw.isClosed()).to.be.true;

    const br = new BinaryReader(path);
    expect(br.readUInt32()).to.equal(0x1);
    br.close();
    expect(br.isClosed()).to.be.true;
  });
});