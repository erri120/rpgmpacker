import { before, describe } from "mocha";
import { expect } from "chai";
import { encryptFile, getMD5Hash } from "../src/encryption";
import { RPGMakerVersion } from "../src/rpgmakerTypes";
import { getFileHash } from "./testutils";
import logger, { Level } from "../src/logging";
import { Path } from "../src/ioTypes";

describe("encryption", () => {
  before(() => {
    logger.minLevel = Level.SILENT;
  });

  describe("getMD5Hash", () => {
    it("should return correct hash", () => {
      expect(getMD5Hash("1337").digest("hex")).to.equal("e48e13207341b6bffb7fb1622282247b");
    });
  });

  describe("encryptFile", () => {
    const hash = getMD5Hash("1337");
    const digset = hash.digest();
    const fileHash = "b6c0d042f67d50de7dceb01c4fe81fa60cb64dd847a74113e590981669526092";

    it("should encrypt png files", () => {
      const input = new Path("./test-files/erri120.png");
      const output = new Path("./test-output/erri120.png");
      encryptFile(input, output, digset, false, false, RPGMakerVersion.MV);
      expect(getFileHash("./test-output/erri120.rpgmvp")).to.equal(fileHash);

      encryptFile(input, output, digset, false, false, RPGMakerVersion.MZ);
      expect(getFileHash("./test-output/erri120.png_")).to.equal(fileHash);
    });

    it("should encrypt ogg files", () => {
      const input = new Path("./test-files/erri120.ogg");
      const output = new Path("./test-output/erri120.ogg");
      encryptFile(input, output, digset, false, false, RPGMakerVersion.MV);
      expect(getFileHash("./test-output/erri120.rpgmvo")).to.equal(fileHash);

      encryptFile(input, output, digset, false, false, RPGMakerVersion.MZ);
      expect(getFileHash("./test-output/erri120.ogg_")).to.equal(fileHash);
    });

    it("should encrypt m4a files", () => {
      const input = new Path("./test-files/erri120.m4a");
      const output = new Path("./test-output/erri120.m4a");
      encryptFile(input, output, digset, false, false, RPGMakerVersion.MV);
      expect(getFileHash("./test-output/erri120.rpgmvm")).to.equal(fileHash);

      encryptFile(input, output, digset, false, false, RPGMakerVersion.MZ);
      expect(getFileHash("./test-output/erri120.m4a_")).to.equal(fileHash);
    });
  });
});