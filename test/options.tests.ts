import { describe } from "mocha";
import { expect } from "chai";

import { createOptionsFromYargs } from "../src/options";

describe("options", () => {
  describe("createOptionsFromYargs", () => {
    it("should return not null", () => {
      const opt = createOptionsFromYargs({
        input: "./src",
        output: "./src",
        rpgmaker: "./src",
        debug: true,
        encryptAudio: true,
        encryptImages: true,
        encryptionKey: "1337",
        exclude: true,
        hardlinks: true,
        threads: 4,
        platforms: [0,1,2,3,4]
      });

      expect(opt).to.not.be.null;
    });
  });
});