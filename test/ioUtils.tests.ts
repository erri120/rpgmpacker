import { describe } from "mocha";
import { expect } from "chai";
import { Path } from "../src/ioTypes";
import { shouldEncryptFile, shouldFilterFile } from "../src/ioUtils";
import { FolderType } from "../src/fileOperations";
import { RPGMakerPlatform, RPGMakerVersion } from "../src/rpgmakerTypes";

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
});