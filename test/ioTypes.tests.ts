import { describe } from "mocha";
import { expect } from "chai";
import { Path } from "../src/ioTypes";

describe("ioTypes", () => {
  describe("Path class", () => {
    it("should construct correctly", () => {
      const p = new Path("C:\\README.md");
      expect(p.fullPath).to.equal("C:\\README.md");
      expect(p.dirName).to.equal("C:\\");
      expect(p.baseName).to.equal("README");
      expect(p.extension).to.equal(".md");
      expect(p.isFile).to.be.true;
      expect(p.isDir).to.be.false;
    });

    it("should clone correctly", () => {
      const p1 = new Path("C:\\README.md");
      const p2 = p1.clone();
      expect(p2.fullPath).to.equal(p1.fullPath);
      expect(p2.dirName).to.equal(p1.dirName);
      expect(p2.baseName).to.equal(p1.baseName);
      expect(p2.extension).to.equal(p1.extension);
      expect(p2.isFile).to.equal(p2.isFile);
      expect(p2.isDir).to.equal(p2.isDir);
    });

    it("should replace extension correctly", () => {
      let p = new Path("C:\\README.md");
      p = p.replaceExtension(".txt");
      expect(p.fullPath).to.equal("C:\\README.txt");
      expect(p.extension).to.equal(".txt");
    });

    it("should change directory correctly", () => {
      let p = new Path("C:\\README.md");
      p = p.changeDirectory("D:\\");
      expect(p.fullPath).to.equal("D:\\README.md");
      expect(p.dirName).to.equal("D:\\");
    });
  });
});