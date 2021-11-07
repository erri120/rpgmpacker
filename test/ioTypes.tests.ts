import { describe } from "mocha";
import { expect } from "chai";
import { Path } from "../src/ioTypes";

describe("ioTypes", () => {
  describe("Path class", () => {
    describe("contructor", () => {
      it("should be correct", () => {
        const p = new Path("C:\\README.md");
        expect(p.fullPath).to.equal("C:\\README.md");
        expect(p.dirName).to.equal("C:\\");
        expect(p.baseName).to.equal("README");
        expect(p.extension).to.equal(".md");
        expect(p.isFile).to.be.true;
        expect(p.isDir).to.be.false;
      });
    });

    describe("clone", () => {
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
    });

    describe("replaceExtension", () => {
      it("should replace extension correctly", () => {
        let p = new Path("C:\\README.md");
        p = p.replaceExtension(".txt");
        expect(p.fullPath).to.equal("C:\\README.txt");
        expect(p.extension).to.equal(".txt");
      });
    });

    describe("changeDirectory", () => {
      it("should change directory correctly", () => {
        let p = new Path("C:\\README.md");
        p = p.changeDirectory("D:\\");
        expect(p.fullPath).to.equal("D:\\README.md");
        expect(p.dirName).to.equal("D:\\");
      });
    });

    describe("isInDirectory", () => {
      it("should return true", () => {
        const p1 = new Path("./src");
        const p2 = new Path("./src/index.ts");
        expect(p2.isInDirectory(p1)).to.be.true;
      });

      it("should return false", () => {
        const p1 = new Path("./src/ioTypes.ts");
        const p2 = new Path("./test/ioTypes.tests.s");
        expect(p2.isInDirectory(p1)).to.be.false;
      });
    });

    describe("exists", () => {
      it("should return true", () => {
        const p = new Path("./src");
        expect(p.exists()).to.be.true;
      });

      it("should return false", () => {
        const p = new Path("./aisjdoiajijdpiwqjpidjwqpijdip");
        expect(p.exists()).to.be.false;
      });
    });

    describe("join", () => {
      it("should join correctly", () => {
        const p1 = new Path("./src");
        const p2 = p1.join("index.ts");
        const p3 = new Path("./src/index.ts");
        expect(p2.fullPath).to.equal(p3.fullPath);
      });
    });

    describe("relativeTo", () => {
      it("should return correct values", () => {
        const p1 = new Path("./src");
        const p2 = new Path("./src/index.ts");
        const s = p2.relativeTo(p1);
        expect(s).to.equal("index.ts");
      });
    });
  });
});