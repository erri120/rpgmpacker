#!/usr/bin/env node

import yargs from "yargs";
import { hideBin } from "yargs/helpers";
import { createOptionsFromYargs } from "./options";
import { RPGMakerPlatform, RPGMakerVersion } from "./rpgmakerTypes";

function main() {
  const opt = yargs(hideBin(process.argv))
    .option("input", {
      // alias: "i",
      type: "string",
      description: "Path to the input folder",
      demandOption: true
    })
    .option("output", {
      // alias: "o",
      type: "string",
      description: "Path to the output folder",
      demandOption: true
    })
    .option("rpgmaker", {
      type: "string",
      description: "Path to the RPG Maker installation folder",
      demandOption: true
    })
    .option("platforms", {
      type: "array",
      array: true,
      choices: [RPGMakerPlatform.Windows, RPGMakerPlatform.OSX, RPGMakerPlatform.Linux, RPGMakerPlatform.Browser, RPGMakerPlatform.Mobile],
      description: "Platforms to build for",
      demandOption: true,
    })
    .option("encryptAudio", {
      type: "boolean",
      description: "Encrypt audio files with the provided encryption key",
      default: false
    })
    .option("encryptImages", {
      type: "boolean",
      description: "Encrypt images with the provided encryption key",
      default: false
    })
    .option("encryptionKey", {
      type: "string",
      description: "Encryption key"
    })
    .option("exclude", {
      type: "boolean",
      description: "Exclude unused files",
      default: false
    })
    .option("hardlinks", {
      type: "boolean",
      description: "Use hardlinks instead of copying files"
    })
    .option("threads", {
      type: "number",
      description: "Number of threads to use",
      default: 2
    })
    .option("debug", {
      alias: "d",
      type: "boolean",
      description: "Activate debug mode",
    })
    .parseSync();

  const options = createOptionsFromYargs(opt);
  console.log(options);
}

main();