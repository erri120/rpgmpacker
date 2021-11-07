import { describe, beforeEach } from "mocha";
import { expect } from "chai";
import sinon from "sinon";

import fs from "fs";
import { getTemplateFolderName, identifyRPGMakerVersion } from "../src/rpgmakerUtils";
import { RPGMakerPlatform, RPGMakerVersion } from "../src/rpgmakerTypes";
import logger, { Level } from "../src/logging";
import { Path } from "../src/ioTypes";

describe("rpgmakerUtils", () => {
  before(() => {
    logger.minLevel = Level.SILENT;
  });

  describe("getTemplateFolderNames", () => {
    it("should return correct values for correct inputs", () => {
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Windows)).to.equal("nwjs-win");
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.OSX)).to.equal("nwjs-osx-unsigned");
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Linux)).to.equal("nwjs-lnx");

      expect(getTemplateFolderName(RPGMakerVersion.MZ, RPGMakerPlatform.Windows)).to.equal("nwjs-win");
      expect(getTemplateFolderName(RPGMakerVersion.MZ, RPGMakerPlatform.OSX)).to.equal("nwjs-mac");
      expect(getTemplateFolderName(RPGMakerVersion.MZ, RPGMakerPlatform.Linux)).to.equal("nwjs-linux");
    });

    it("should return correct values for bad inputs", () => {
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Mobile)).to.be.null;
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Browser)).to.be.null;

      expect(getTemplateFolderName(RPGMakerVersion.MZ, RPGMakerPlatform.Mobile)).to.be.null;
      expect(getTemplateFolderName(RPGMakerVersion.MZ, RPGMakerPlatform.Browser)).to.be.null;
    });
  });

  describe("identifyRPGMakerVersion", () => {
    beforeEach(() => {
      sinon.restore();
    });

    function mockReadDir(fileName: string) {
      const fsStub = sinon.stub(fs, "readdirSync");
      const dirent = new fs.Dirent();
      const mock = sinon.mock(dirent);
      mock.expects("isFile").returns(true);
      dirent.name = fileName;
      fsStub.returns([dirent]);
    }

    it("should return RPGMakerVersion.MV", () => {
      mockReadDir("Game.rpgproject");
      const res = identifyRPGMakerVersion(new Path("./"));
      expect(res).to.equal(RPGMakerVersion.MV);
    });

    it("should return RPGMakerVersion.MZ", () => {
      mockReadDir("Game.rmmzproject");
      const res = identifyRPGMakerVersion(new Path("./"));
      expect(res).to.equal(RPGMakerVersion.MZ);
    });

    it("should return null", () => {
      const res = identifyRPGMakerVersion(new Path("./"));
      expect(res).to.be.null;
    });
  });
});