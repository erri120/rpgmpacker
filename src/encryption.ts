import fs from "fs";
import { createHash, Hash } from "crypto";

import logger from "./logging";
import { RPGMakerVersion } from "./rpgmakerTypes";
import { Path } from "./ioTypes";

export function getMD5Hash(input: string): Hash {
  const hash = createHash("md5");
  hash.update(input, "utf-8");
  return hash;
  // return hash.digest("hex");
}

const extensions = {
  ".ogg": (version: RPGMakerVersion) => {
    switch (version) {
    case RPGMakerVersion.MV: return ".rpgmvo";
    case RPGMakerVersion.MZ: return ".ogg_";
    default: return null;
    }
  },
  ".m4a": (version: RPGMakerVersion) => {
    switch (version) {
    case RPGMakerVersion.MV: return ".rpgmvm";
    case RPGMakerVersion.MZ: return ".m4a_";
    default: return null;
    }
  },
  ".png": (version: RPGMakerVersion) => {
    switch (version) {
    case RPGMakerVersion.MV: return ".rpgmvp";
    case RPGMakerVersion.MZ: return ".png_";
    default: return null;
    }
  },
};

const superTopSecretEncryptionHeader = new Uint8Array([0x52, 0x50, 0x47, 0x4D, 0x56, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00]);

export function encryptFile(from: Path, to: Path, hash: Buffer, useCache: boolean, useHardlink: boolean, version: RPGMakerVersion): Path | null {
  if (hash.length !== 16) {
    throw new Error("Hash Buffer does not have a length of 16!");
  }

  let newExt: string | null = null;
  switch (from.extension) {
  case ".ogg": newExt = extensions[".ogg"](version); break;
  case ".m4a": newExt = extensions[".m4a"](version); break;
  case ".png": newExt = extensions[".png"](version); break;
  default: newExt = null; break;
  }

  if (newExt === null) {
    logger.error(`Unknown file extension ${from.extension} of file ${from}`);
    return null;
  }

  to = to.replaceExtension(newExt);
  logger.debug(`Encrypting file from ${from} to ${to}`);

  const fromFd = fs.openSync(from.fullPath, "r", 0o666);
  const toFd = fs.openSync(to.fullPath, "w+", 0o666);

  const stats = fs.fstatSync(fromFd);
  const filesize = stats.size;

  if (filesize < 16) {
    logger.error(`Input file ${from} is less than 16 bytes long!`);
    return null;
  }

  let bytesWritten = fs.writeSync(toFd, superTopSecretEncryptionHeader, 0, superTopSecretEncryptionHeader.length);
  if (bytesWritten !== superTopSecretEncryptionHeader.length) {
    logger.error(`Tried writing ${superTopSecretEncryptionHeader.length} bytes but only wrote ${bytesWritten} to file ${to}`);
    fs.closeSync(fromFd);
    fs.closeSync(toFd);
    return null;
  }

  const buffer = new Uint8Array(16);
  let bytesRead = fs.readSync(fromFd, buffer, { length: buffer.length });
  if (bytesRead !== buffer.length) {
    logger.error(`Tried reading ${buffer.length} bytes but only read ${bytesRead} from file ${from}`);
    fs.closeSync(fromFd);
    fs.closeSync(toFd);
    return null;
  }

  for (let i = 0; i < 16; i++) {
    const c1 = buffer[i];
    const c2 = hash[i];
    const result = c1 ^ c2;
    buffer[i] = result;
  }

  bytesWritten = fs.writeSync(toFd, buffer, 0, buffer.length);
  if (bytesWritten !== buffer.length) {
    logger.error(`Tried writing ${buffer.length} bytes but only wrote ${bytesWritten} to file ${to}`);
    fs.closeSync(fromFd);
    fs.closeSync(toFd);
    return null;
  }

  const remaining = new Uint8Array(filesize - 16);
  bytesRead = fs.readSync(fromFd, remaining, { length: remaining.byteLength });
  if (bytesRead !== remaining.length) {
    logger.error(`Tried reading ${buffer.length} bytes but only read ${bytesRead} from file ${from}`);
    fs.closeSync(fromFd);
    fs.closeSync(toFd);
    return null;
  }

  bytesWritten = fs.writeSync(toFd, remaining, 0, remaining.length);
  if (bytesWritten !== remaining.length) {
    logger.error(`Tried writing ${remaining.length} bytes but only wrote ${bytesWritten} to file ${to}`);
    fs.closeSync(fromFd);
    fs.closeSync(toFd);
    return null;
  }

  fs.closeSync(fromFd);
  fs.closeSync(toFd);
  return to;
}

export function updateSystemJson(from: Path, to: Path, encryptAudio: boolean, encryptImages: boolean, hash: Hash) {
  logger.debug(`Updating System.json from ${from} to ${to} with encryption data`);

  let json = fs.readFileSync(from.fullPath, { encoding: "utf-8" });
  const system = JSON.parse(json);
  system["encryptionKey"] = hash.digest("hex");
  system["hasEncryptedAudio"] = encryptAudio;
  system["hasEncryptedImages"] = encryptImages;

  json = JSON.stringify(system);
  fs.writeFileSync(to.fullPath, json);
}