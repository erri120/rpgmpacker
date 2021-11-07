#!/usr/bin/env node

import yargs from "yargs";
import { hideBin } from "yargs/helpers";

import fs from "fs";

import { getMD5Hash } from "./encryption";
import { isSameDevice, walkDirectoryRecursively } from "./ioUtils";
import logger, { Level } from "./logging";
import { createOptionsFromYargs } from "./options";
import { RPGMakerPlatform, RPGMakerVersion } from "./rpgmakerTypes";
import { getTemplateFolderName, identifyRPGMakerVersion } from "./rpgmakerUtils";

function main() {
  const yargsResult = yargs(hideBin(process.argv))
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

  const options = createOptionsFromYargs(yargsResult);
  if (options === null) {
    logger.error("Unable to parse options!");
    return;
  }

  if (options.Debug) {
    logger.setMinLevel(Level.DEBUG);
  } else {
    logger.setMinLevel(Level.INFO);
  }

  logger.debug(JSON.stringify(options, undefined, 2));

  const rpgmakerVersion = identifyRPGMakerVersion(options.Input);
  if (rpgmakerVersion === null) {
    logger.error("Unable to identify RPG Maker Version!");
    return;
  }

  // MV does not export to Mobile, MZ does but I don't have a version of it :(
  if (options.Platforms.indexOf(RPGMakerPlatform.Mobile) !== -1) {
    logger.error("This tool does not support Mobile as a target!");
    return;
  }

  // check if we can hardlink
  let canHardlinkRPGMakerToOutput = false;
  let canHardlinkInputToOutput = false;
  if (options.UseHardlinks) {
    canHardlinkRPGMakerToOutput = isSameDevice(options.RPGMaker, options.Output);
    canHardlinkInputToOutput = isSameDevice(options.Input, options.Output);

    if (!canHardlinkRPGMakerToOutput && !canHardlinkInputToOutput) {
      logger.warn("Can not hardlink between the RPG Maker or Input and Output folder because they are not on the same device. Hardlinking will be disabled.");
      options.UseHardlinks = false;
    } else {
      logger.debug(`Can hardlink from RPG Maker to Output folder: ${canHardlinkRPGMakerToOutput}`);
      logger.debug(`Can hardlink from Input to Output folder: ${canHardlinkInputToOutput}`);
    }
  }

  // create hash
  let hash: Buffer | undefined;
  if (options.EncryptionOptions) {
    hash = getMD5Hash(options.EncryptionOptions.EncryptionKey).digest();
  }

  logger.log(`Building output for ${options.Platforms.length} targets`);
  for (let i = 0; i < options.Platforms.length; i++) {
    const p = options.Platforms[i];
    const platformOutputPath = options.Output.join(p);
    logger.debug(`Current Platform: ${p}, Output path: ${platformOutputPath}`);

    fs.mkdirSync(platformOutputPath.fullPath, { recursive: true });

    // the browser does not need a template folder
    const templateFolderName = getTemplateFolderName(rpgmakerVersion, p);
    if (templateFolderName !== null) {
      const templateFolderPath = options.RPGMaker.join(templateFolderName);
      if (!templateFolderPath.exists()) {
        logger.error(`The template folder ${templateFolderPath} does not exist!`);
        return;
      }

      logger.debug(`Template folder is ${templateFolderPath}`);

      for (const path of walkDirectoryRecursively(templateFolderPath)) {
        logger.debug(path.fullPath);
      }
    }
  }
}

main();