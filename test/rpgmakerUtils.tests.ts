import { describe } from 'mocha';
import { expect } from 'chai';
import { getTemplateFolderName } from '../src/rpgmakerUtils';
import { RPGMakerPlatform, RPGMakerVersion } from '../src/rpgmakerTypes';

describe("rpgmakerUtils", () => {
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
    })
  });
});