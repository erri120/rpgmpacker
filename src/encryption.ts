import fs from "fs";
import { createHash, Hash } from "crypto";

import logger from "./logging";
import { RPGMakerVersion } from "./rpgmakerTypes";
import { Path } from "./ioTypes";
import { BinaryReader } from "./io/BinaryReader";
import { BinaryWriter } from "./io/BinaryWriter";

export function getMD5Hash(input: string): Hash {
  const hash = createHash("md5");
  hash.update(input, "utf-8");
  return hash;
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
    logger.error(`Unknown file extension ${from.extension} of file ${from.fullPath}`);
    return null;
  }

  to = to.replaceExtension(newExt);
  logger.debug(`Encrypting file from ${from.fullPath} to ${to.fullPath}`);

  const br = new BinaryReader(from);

  const stats = br.stats();
  if (stats === null) return null;

  const filesize = stats.size;

  const bw = new BinaryWriter(to);

  if (!bw.write(superTopSecretEncryptionHeader)) {
    br.close();
    return null;
  }

  const buffer = br.read(16);
  if (buffer === null) {
    bw.close();
    return null;
  }

  for (let i = 0; i < 16; i++) {
    const c1 = buffer[i];
    const c2 = hash[i];
    const result = c1 ^ c2;
    buffer[i] = result;
  }

  if (!bw.write(buffer)) {
    br.close();
    return null;
  }

  const remaining = br.read(filesize - 16);
  if (remaining === null) {
    bw.close();
    return null;
  }

  if (!bw.write(remaining)) {
    br.close();
    return null;
  }

  if (!br.isClosed()) {
    br.close();
  }

  if (!bw.isClosed()) {
    bw.close();
  }

  return to;
}

export function updateSystemJson(from: Path, to: Path, encryptAudio: boolean, encryptImages: boolean, hash: Buffer) {
  logger.debug(`Updating System.json from ${from.fullPath} to ${to.fullPath} with encryption data`);

  let json = fs.readFileSync(from.fullPath, { encoding: "utf-8" });
  const system = JSON.parse(json);
  system["encryptionKey"] = hash.toString("hex");
  system["hasEncryptedAudio"] = encryptAudio;
  system["hasEncryptedImages"] = encryptImages;

  json = JSON.stringify(system);
  fs.writeFileSync(to.fullPath, json);
}