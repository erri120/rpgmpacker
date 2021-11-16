import { describe, it } from "mocha";
import { expect } from "chai";

import { sep } from "path";

import Path from "../../src/io/Path";

function getReadmeFile() {
  if (sep === "\\")
    return new Path("C:\\README.md");
  else
    return new Path("/README.md");
}

describe("Path", () => {
  const isUnix = sep === "/";
  const readmeFile = getReadmeFile();
  const testDir = new Path("./test");
  const testFile = new Path("./test/io/Path.tests.ts");

  it("constructor", () => {
    const path = new Path(isUnix ? "/README.md" : "C:\\README.md");

    if (isUnix) {
      expect(path.fullPath).to.equal("/README.md");
      expect(path.dirName).to.equal("/");
    } else {
      expect(path.fullPath).to.equal("C:\\README.md");
      expect(path.dirName).to.equal("C:\\");
    }

    expect(path.fileName).to.equal("README.md");
    expect(path.baseName).to.equal("README");
    expect(path.extension).to.equal(".md");
  });

  it("exists", () => {
    expect(testDir.exists()).to.be.true;
    expect(testFile.exists()).to.be.true;

    const p = new Path("./sifjiejpijfpiajipjipawjipdjpijwi");
    expect(p.exists()).to.be.false;
  });

  it("isFile", () => {
    expect(testFile.isFile()).to.be.true;
    expect(testDir.isFile()).to.be.false;
  });

  it("isDir", () => {
    expect(testDir.isDir()).to.be.true;
    expect(testFile.isDir()).to.be.false;
  });

  it("replaceExtension", () => {
    const p = readmeFile.replaceExtension(".txt");
    expect(p.fullPath).to.equal(isUnix ? "/README.md" : "C:\\README.txt");
    expect(p.fileName).to.equal("README.txt");
    expect(p.baseName).to.equal("README");
    expect(p.extension).to.equal(".txt");
  });

  it("replaceName", () => {
    const p = readmeFile.replaceName("DONTREADME");
    expect(p.fullPath).to.equal(isUnix ? "/DONTREADME.md" : "C:\\DONTREADME.md");
    expect(p.fileName).to.equal("DONTREADME.md");
    expect(p.baseName).to.equal("DONTREADME");
    expect(p.extension).to.equal(".md");
  });

  it("replaceFileName", () => {
    const p = readmeFile.replaceFileName("DONTREADME.txt");
    expect(p.fullPath).to.equal(isUnix ? "/DONTREADME.txt" : "C:\\DONTREADME.txt");
    expect(p.fileName).to.equal("DONTREADME.txt");
    expect(p.baseName).to.equal("DONTREADME");
    expect(p.extension).to.equal(".txt");
  });

  it("join", () => {
    if (isUnix) {
      const p1 = new Path("/");
      const p2 = p1.join("files", "README.md");
      expect(p2.fullPath).to.equal("/files/README.md");
    } else {
      const p1 = new Path("C:\\");
      const p2 = p1.join("files", "README.md");
      expect(p2.fullPath).to.equal("C:\\files\\README.md");
    }
  });

  it("isInDirectory", () => {
    expect(testFile.isInDirectory(testDir)).to.be.true;
    expect(readmeFile.isInDirectory(testDir)).to.be.false;
  });

  it("relativeTo", () => {
    expect(testFile.relativeTo(testDir)).to.equal("io\\Path.tests.ts");
  });

  it("getParent", () => {
    const p1 = readmeFile.getParent();
    if (isUnix) {
      expect(p1.fullPath).to.equal("/");
    } else {
      expect(p1.fullPath).to.equal("C:\\");
    }

    const p2 = p1.getParent();
    expect(p1.equals(p2)).to.be.true;
  });
});