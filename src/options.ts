import { Path } from "./ioTypes";
import logger from "./logging";
import { RPGMakerPlatform } from "./rpgmakerTypes";

export interface Options {
  Input: Path;
  Output: Path;
  RPGMaker: Path;
  Platforms: RPGMakerPlatform[];
  EncryptionOptions?: EncryptionOptions;
  ExcludeUnused: boolean;
  UseHardlinks: boolean;
  NumThreads: number;
  Debug: boolean;
}

export interface EncryptionOptions {
  EncryptAudio: boolean;
  EncryptImages: boolean;
  EncryptionKey: string;
}

export function createOptionsFromYargs(opt: {
  input: string;
  output: string;
  rpgmaker: string;
  platforms: (string | number)[];
  encryptAudio: boolean;
  encryptImages: boolean;
  encryptionKey: string | undefined;
  exclude: boolean;
  debug: boolean | undefined,
  hardlinks: boolean | undefined,
  threads: number
}): Options | null {

  const inputPath = new Path(opt.input);
  if (!inputPath.exists()) {
    logger.error(`Input folder ${inputPath} does not exist!`);
    return null;
  }

  if (!inputPath.isDir) {
    logger.error(`Input path ${inputPath} is not a folder!`);
    return null;
  }

  const outputPath = new Path(opt.output);
  if (!outputPath.exists()) {
    logger.error(`Output folder ${outputPath} does not exist!`);
    return null;
  }

  if (!outputPath.isDir) {
    logger.error(`Output path ${inputPath} is not a folder!`);
    return null;
  }

  const rpgmakerPath = new Path(opt.rpgmaker);
  if (!rpgmakerPath.exists()) {
    logger.error(`RPG Maker folder ${rpgmakerPath} does not exist!`);
    return null;
  }

  if (!rpgmakerPath.isDir) {
    logger.error(`RPG Maker path ${inputPath} is not a folder!`);
    return null;
  }

  const platforms: RPGMakerPlatform[] = [];
  for (let i = 0; i < opt.platforms.length; i++) {
    const p = opt.platforms[i];
    switch (p) {
    case RPGMakerPlatform.Windows:
    case 0:
      platforms.push(RPGMakerPlatform.Windows);
      break;
    case RPGMakerPlatform.OSX:
    case 1:
      platforms.push(RPGMakerPlatform.OSX);
      break;
    case RPGMakerPlatform.Linux:
    case 2:
      platforms.push(RPGMakerPlatform.Linux);
      break;
    case RPGMakerPlatform.Browser:
    case 3:
      platforms.push(RPGMakerPlatform.Browser);
      break;
    case RPGMakerPlatform.Mobile:
    case 4:
      platforms.push(RPGMakerPlatform.Mobile);
      break;
    default:
      logger.error(`Unknown platform: ${p}`);
      return null;
    }
  }

  let encryptionOptions: EncryptionOptions | undefined;
  if (opt.encryptAudio || opt.encryptImages) {
    if (opt.encryptionKey === undefined || opt.encryptionKey.length === 0) {
      logger.error("Encryption enabled but no key was given!");
      return null;
    }

    encryptionOptions = {
      EncryptAudio: opt.encryptAudio,
      EncryptImages: opt.encryptImages,
      EncryptionKey: opt.encryptionKey
    };
  }

  return {
    Input: inputPath,
    Output: outputPath,
    RPGMaker: rpgmakerPath,
    Platforms: platforms,
    EncryptionOptions: encryptionOptions,
    ExcludeUnused: opt.exclude,
    NumThreads: opt.threads,
    UseHardlinks: opt.hardlinks === true,
    Debug: opt.debug === true
  };
}
