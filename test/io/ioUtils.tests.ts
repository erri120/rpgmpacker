import { describe, it } from "mocha";
import { expect } from "chai";

import fs from "fs";

import Path from "../../src/io/Path";
import { isSameDevice, transferFile } from "../../src/io/ioUtils";

describe("ioUtils", () => {
  describe("transferFile", () => {
    it("with hardlinking", () => {
      const p1 = new Path("./test-files/dummy.txt");
      const p2 = new Path("./test-output/ioUtils-transferFile-hardlink.txt");
      if (p2.exists()) {
        fs.rmSync(p2.fullPath);
      }

      transferFile(p1, p2, true);
      expect(p2.exists()).to.be.true;
    });

    it("without hardlinking", () => {
      const p1 = new Path("./test-files/dummy.txt");
      const p2 = new Path("./test-output/ioUtils-transferFile-copy.txt");
      if (p2.exists()) {
        fs.rmSync(p2.fullPath);
      }

      transferFile(p1, p2, false);
      expect(p2.exists()).to.be.true;
    });
  });

  describe("isSameDevice", () => {
    it("is true", () => {
      const p1 = new Path("./test-files/dummy.txt");
      const p2 = new Path("./README.md");
      expect(isSameDevice(p1, p2)).to.be.true;
    });
  });
});