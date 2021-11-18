import { describe } from "mocha";
import { expect } from "chai";

import { getTemplateFolderName } from "../../src/rpgmakerTypes/TemplateFolderName";
import { RPGMakerVersion } from "../../src/rpgmakerTypes/RPGMakerVersion";
import RPGMakerPlatform from "../../src/rpgmakerTypes/RPGMakerPlatform";

describe("TemplateFolderName", () => {
  describe("getTemplateFolderName", () => {
    it("for MV", () => {
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Windows)).to.equal("nwjs-win");
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.OSX)).to.equal("nwjs-osx-unsigned");
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Linux)).to.equal("nwjs-lnx");
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Browser)).to.be.null;
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Mobile)).to.be.null;
    });

    it("for MZ", () => {
      expect(getTemplateFolderName(RPGMakerVersion.MZ, RPGMakerPlatform.Windows)).to.equal("nwjs-win");
      expect(getTemplateFolderName(RPGMakerVersion.MZ, RPGMakerPlatform.OSX)).to.equal("nwjs-mac");
      expect(getTemplateFolderName(RPGMakerVersion.MZ, RPGMakerPlatform.Linux)).to.equal("nwjs-linux");
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Browser)).to.be.null;
      expect(getTemplateFolderName(RPGMakerVersion.MV, RPGMakerPlatform.Mobile)).to.be.null;
    });
  });
});