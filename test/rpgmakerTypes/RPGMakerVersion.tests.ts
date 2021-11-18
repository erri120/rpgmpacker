import { describe, before } from "mocha";
import { expect } from "chai";

import fs from "fs";

import logger, { Level } from "../../src/logging";
import Path from "../../src/io/Path";
import { RPGMakerVersion, identifyRPGMakerVersion } from "../../src/rpgmakerTypes/RPGMakerVersion";

describe("RPGMakerVersion", () => {
  before(() => {
    logger.setMinLevel(Level.SILENT);
  });

  describe("identifyRPGMakerVersion", () => {
    it("identifies MV", () => {
      const path = new Path("./test-output/identifyRPGMakerVersion-MV");
      fs.mkdirSync(path.fullPath, { recursive: true });
      fs.writeFileSync(path.join("Game.rpgproject").fullPath, "Hi!");
      expect(identifyRPGMakerVersion(path)).to.equal(RPGMakerVersion.MV);
    });

    it("identifies MZ", () => {
      const path = new Path("./test-output/identifyRPGMakerVersion-MZ");
      fs.mkdirSync(path.fullPath, { recursive: true });
      fs.writeFileSync(path.join("Game.rmmzproject").fullPath, "Hi!");
      expect(identifyRPGMakerVersion(path)).to.equal(RPGMakerVersion.MZ);
    });
  });
});