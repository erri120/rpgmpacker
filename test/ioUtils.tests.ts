import { describe } from "mocha";
import { expect } from "chai";

import { Path } from "../src/ioTypes";
import { isSameDevice, shouldEncryptFile, shouldFilterFile, transferFile } from "../src/ioUtils";
import { FolderType } from "../src/fileOperations";
import { RPGMakerPlatform, RPGMakerVersion } from "../src/rpgmakerTypes";
import { accessSync, constants } from "fs";

describe("ioUtils", () => {
  describe("shouldFilterFile", () => {
    it("should filter project files", () => {
      expect(shouldFilterFile(new Path("Game.rpgproject"), FolderType.ProjectFolder, { Platform: RPGMakerPlatform.Windows, Version: RPGMakerVersion.MV })).to.be.true;
      expect(shouldFilterFile(new Path("Game.rmmzproject"), FolderType.ProjectFolder, { Platform: RPGMakerPlatform.Windows, Version: RPGMakerVersion.MV })).to.be.true;
    });

    it("should filter audio files for Desktop", () => {
      expect(shouldFilterFile(new Path("Music.ogg"), FolderType.ProjectFolder, { Platform: RPGMakerPlatform.Windows, Version: RPGMakerVersion.MV })).to.be.false;
      expect(shouldFilterFile(new Path("Music.m4a"), FolderType.ProjectFolder, { Platform: RPGMakerPlatform.Windows, Version: RPGMakerVersion.MV })).to.be.true;
    });

    it("should filter audio files for Mobile", () => {
      expect(shouldFilterFile(new Path("Music.ogg"), FolderType.ProjectFolder, { Platform: RPGMakerPlatform.Mobile, Version: RPGMakerVersion.MV })).to.be.true;
      expect(shouldFilterFile(new Path("Music.m4a"), FolderType.ProjectFolder, { Platform: RPGMakerPlatform.Mobile, Version: RPGMakerVersion.MV })).to.be.false;
    });

    it("should not filter audio files for Browser", () => {
      expect(shouldFilterFile(new Path("Music.ogg"), FolderType.ProjectFolder, { Platform: RPGMakerPlatform.Browser, Version: RPGMakerVersion.MV })).to.be.false;
      expect(shouldFilterFile(new Path("Music.m4a"), FolderType.ProjectFolder, { Platform: RPGMakerPlatform.Browser, Version: RPGMakerVersion.MV })).to.be.false;
    });
  });

  describe("shouldEncryptFile", () => {
    it("should encrypt audio files", () => {
      expect(shouldEncryptFile(new Path("Music.ogg"), true, false)).to.be.true;
      expect(shouldEncryptFile(new Path("Music.m4a"), true, false)).to.be.true;
    });

    it("should encrypt image files", () => {
      expect(shouldEncryptFile(new Path("Music.png"), false, true)).to.be.true;
    });

    it("should not encrypt audio files", () => {
      expect(shouldEncryptFile(new Path("Music.ogg"), false, false)).to.be.false;
      expect(shouldEncryptFile(new Path("Music.m4a"), false, false)).to.be.false;
    });

    it("should not encrypt image files", () => {
      expect(shouldEncryptFile(new Path("Music.png"), false, false)).to.be.false;
    });
  });

  describe("transferFile", () => {
    it("should copy a file", () => {
      const src = new Path("./test-files/erri120.png");
      const dest = new Path("./test-output/erri120.png");
      transferFile(src, dest, false);
      accessSync(dest.fullPath, constants.R_OK);
    });

    it("should hardlink a file", () => {
      const src = new Path("./test-files/erri120.png");
      const dest = new Path("./test-output/erri120.png");
      transferFile(src, dest, true);
      accessSync(dest.fullPath, constants.R_OK);
    });
  });

  describe("isSameDevice", () => {
    it("should return true", () => {
      const a = new Path("./test-files/erri120.png");
      const b = a.clone();

      expect(isSameDevice(a, b)).to.be.true;
    });
  });
});