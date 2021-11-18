#!/usr/bin/env node
import fs from "fs";

import yargs from "yargs";
import { hideBin } from "yargs/helpers";

import logger, { Level } from "./logging";
import { createOptionsFromYargs } from "./options";
import { encryptFile, getMD5Hash, updateSystemJson, shouldEncryptFile } from "./encryption";
import { isSameDevice, walkDirectoryRecursively, transferFile, removeEmptyFolders } from "./io/ioUtils";
import RPGMakerPlatform from "./rpgmakerTypes/RPGMakerPlatform";
import { RPGMakerVersion, identifyRPGMakerVersion } from "./rpgmakerTypes/RPGMakerVersion";
import { getTemplateFolderName } from "./rpgmakerTypes/TemplateFolderName";
import { FileOperation, FolderType, OperationType } from "./fileOperations";
import { createProjectPathRegistry } from "./rpgmakerTypes/ProjectPathRegistry";
import { createTemplatePathRegistry, TemplatePathRegistry } from "./rpgmakerTypes/TemplatePathRegistry";
import { parseData, ParsedData } from "./parsedData";
import { parsePlugins } from "./pluginUtils";
import { isUseless, isUnused } from "./filtering";

function main() {
  const yargsResult = yargs(hideBin(process.argv))
    .option("input", {
      type: "string",
      description: "Path to the input folder",
      demandOption: true
    })
    .option("output", {
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
    .option("noempty", {
      type: "boolean",
      description: "Remove empty folders after execution"
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

  if (!options.Output.exists()) {
    fs.mkdirSync(options.Output.fullPath, { recursive: true });
  }

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

  let parsedData: ParsedData | undefined;
  if (options.ExcludeUnused) {
    const tempParsedData = parseData(options.Input.join("data"), rpgmakerVersion);
    if (tempParsedData === null) {
      logger.error("Unable to parse data!");
      return;
    }

    parsedData = tempParsedData;

    const pluginPaths = parsePlugins(options.Input.join("js"), options.Input);
    if (pluginPaths === null) {
      logger.error("Unable to parse plugins!");
      return;
    }

    parsedData.pluginPaths = pluginPaths;
  }

  const pathRegistry = createProjectPathRegistry(options.Input);

  logger.log(`Building output for ${options.Platforms.length} targets`);
  for (let i = 0; i < options.Platforms.length; i++) {
    const p = options.Platforms[i];
    const platformOutputPath = options.Output.join(p);
    logger.debug(`Current Platform: ${p}, Output path: ${platformOutputPath.fullPath}`);

    if (platformOutputPath.exists()) {
      logger.debug(`Removing old files in ${platformOutputPath.fullPath}`);
      fs.rmSync(platformOutputPath.fullPath, { recursive: true });
    }

    fs.mkdirSync(platformOutputPath.fullPath, { recursive: true });

    const fileOperations: FileOperation[] = [];

    let templatePathRegistry: TemplatePathRegistry | undefined;

    // the browser does not need a template folder
    const templateFolderName = getTemplateFolderName(rpgmakerVersion, p);
    if (templateFolderName !== null) {
      const templateFolderPath = options.RPGMaker.join(templateFolderName);
      if (!templateFolderPath.exists()) {
        logger.error(`The template folder ${templateFolderPath.fullPath} does not exist!`);
        return;
      }

      logger.debug(`Template folder is ${templateFolderPath.fullPath}`);

      templatePathRegistry = createTemplatePathRegistry(templateFolderPath);

      for (const path of walkDirectoryRecursively(templateFolderPath)) {
        const relativePart = path.relativeTo(templateFolderPath);
        let itemOutputPath = platformOutputPath.join(relativePart);

        if (rpgmakerVersion === RPGMakerVersion.MZ) {
          if (p === RPGMakerPlatform.Windows) {
            if (path.fileName === "nw.exe" && path.getParent().equals(templatePathRegistry.top)) {
              itemOutputPath = itemOutputPath.replaceFileName("Game.exe");
            }
          } else if (p === RPGMakerPlatform.OSX) {
            if (path.isInDirectory(templatePathRegistry.nwjs_app)) {
              const nwjsRelative = path.relativeTo(templatePathRegistry.nwjs_app);
              itemOutputPath = platformOutputPath.join("Game.app").join(nwjsRelative);
            }
          } else if (p === RPGMakerPlatform.Linux) {
            if (path.fileName === "nw" && path.getParent().equals(templatePathRegistry.top)) {
              itemOutputPath = itemOutputPath.replaceFileName("Game");
            }
          }
        }

        if (path.isDir()) {
          if (!itemOutputPath.exists())
            fs.mkdirSync(itemOutputPath.fullPath);
          continue;
        }

        if (isUseless(path, FolderType.TemplateFolder, p, rpgmakerVersion, pathRegistry, templatePathRegistry)) {
          continue;
        }

        fileOperations.push({
          Folder: FolderType.TemplateFolder,
          Operation: OperationType.Copy,
          From: path,
          To: itemOutputPath
        });
      }
    }

    // MV has a www folder, MZ does not
    // on OSX the stuff always goes into "Game.app/Contents/Resources/app.nw"
    const wwwPath = p === RPGMakerPlatform.OSX
      ? platformOutputPath.join("Game.app/Contents/Resources/app.nw")
      : rpgmakerVersion === RPGMakerVersion.MV
        ? platformOutputPath.join("www")
        : platformOutputPath;

    logger.debug(`www Path is ${wwwPath.fullPath}`);
    if (!wwwPath.exists())
      fs.mkdirSync(wwwPath.fullPath, { recursive: true });

    for (const path of walkDirectoryRecursively(options.Input)) {
      const relativePart = path.relativeTo(options.Input);
      const itemOutputPath = wwwPath.join(relativePart);

      if (path.isDir()) {
        if (!itemOutputPath.exists())
          fs.mkdirSync(itemOutputPath.fullPath);
        continue;
      }

      if (options.ExcludeUnused) {
        if (isUnused(path, parsedData!, pathRegistry, rpgmakerVersion)) {
          continue;
        }
      }

      if (isUseless(path, FolderType.ProjectFolder, p, rpgmakerVersion, pathRegistry, templatePathRegistry)) {
        continue;
      }

      const operation: FileOperation = {
        Folder: FolderType.ProjectFolder,
        Operation: OperationType.Copy,
        From: path,
        To: itemOutputPath
      };

      if (options.EncryptionOptions) {
        if (shouldEncryptFile(operation.From, options.EncryptionOptions.EncryptAudio, options.EncryptionOptions.EncryptImages, pathRegistry, rpgmakerVersion)) {
          operation.Operation = OperationType.Encrypt;
        }

        if (path.fileName === "System.json") {
          updateSystemJson(path, itemOutputPath, options.EncryptionOptions.EncryptAudio, options.EncryptionOptions.EncryptImages, hash!);
          continue;
        }
      }

      fileOperations.push(operation);
    }

    for (const operation of fileOperations) {
      const canUseHardlinks = operation.Folder === FolderType.TemplateFolder
        ? canHardlinkRPGMakerToOutput
        : canHardlinkInputToOutput;

      if (operation.Operation === OperationType.Copy) {
        transferFile(operation.From, operation.To, canUseHardlinks);
      } else if (operation.Operation === OperationType.Encrypt) {
        encryptFile(operation.From, operation.To, hash!, true, canUseHardlinks, rpgmakerVersion);
      }
    }

    if (options.RemoveEmpty) {
      logger.debug("Removing empty folders from output path");
      removeEmptyFolders(platformOutputPath);
    }

    logger.log(`Finished with Platform ${p}`);
  }
}

main();