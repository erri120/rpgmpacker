import fs from "fs";
import { resolve, sep } from "path";

import logger from "./logging";
import Path from "./io/Path";
import { RPGMakerVersion } from "./rpgmakerTypes/RPGMakerVersion";

export enum BattleSystem {
  FrontView,
  BackView
}

export interface ParsedData {
  //Actors.json
  //img/sv_actors/{}.png
  actorBattlerNames: Set<string>,

  //Animations.json
  animationIds: Set<number>,
  //MV only: img/animations/{}.png
  animationNames: Set<string>,
  //MZ only: effects/{}.efkefc
  effectNames: Set<string>,
  //MZ only: effects/{}
  effectResources: Path[],

  //Enemies.json
  //img/enemies/{}.png
  enemyBattlerNames: Set<string>,

  useSideView: boolean,

  //Tilesets.json
  //img/tilesets/{}.png+{}.txt
  tilesetNames: Set<string>,

  //System.json
  //img/titles1/{}.png
  title1Names: Set<string>,
  //img/titles2/{}.png
  title2Names: Set<string>,

  //other
  //img/characters/{}.png
  characterNames: Set<string>,
  //img/faces/{}.png
  faceNames: Set<string>,

  //audio/bgm/{}.m4a/ogg
  bgmNames: Set<string>,
  //audio/bgs/{}.m4a/ogg
  bgsNames: Set<string>,
  //audio/me/{}.m4a/ogg
  meNames: Set<string>,
  //audio/se/{}.m4a/ogg
  seNames: Set<string>,

  //img/pictures/{}.png
  pictureNames: Set<string>,
  //movies/{}.webm
  movieNames: Set<string>,

  //img/battlebacks1/{}.png
  battleback1Names: Set<string>,
  //img/battlebacks2/{}.png
  battleback2Names: Set<string>,
  //img/parallaxes/{}.png
  parallaxNames: Set<string>,

  pluginPaths: Path[] | undefined,
}

export function parseData(dataPath: Path, version: RPGMakerVersion): ParsedData | null {
  const res: ParsedData = {
    actorBattlerNames: new Set(),
    animationIds: new Set(),
    animationNames: new Set(),
    battleback1Names: new Set(),
    battleback2Names: new Set(),
    bgmNames: new Set(),
    bgsNames: new Set(),
    characterNames: new Set(),
    effectNames: new Set(),
    effectResources: [],
    enemyBattlerNames: new Set(),
    faceNames: new Set(),
    meNames: new Set(),
    movieNames: new Set(),
    parallaxNames: new Set(),
    pictureNames: new Set(),
    seNames: new Set(),
    tilesetNames: new Set(),
    title1Names: new Set(),
    title2Names: new Set(),
    useSideView: false,
    pluginPaths: undefined,
  };

  const items = fs.readdirSync(dataPath.fullPath, { encoding: "utf8" });
  for (const p of items) {
    const path = new Path(resolve(dataPath.fullPath, p));
    if (!path.isFile()) {
      continue;
    }

    const name = path.fileName;
    if (name === "Actors.json") {
      parseActors(path, res);
    } else if (name === "CommonEvents.json") {
      parseCommonEvents(path, res);
    } else if (name === "Enemies.json") {
      parseEnemies(path, res);
    } else if (name === "Items.json") {
      parseItems(path, res);
    } else if (name === "Skills.json") {
      parseSkills(path, res);
    } else if (name === "System.json") {
      parseSystem(path, res);
    } else if (name === "Tilesets.json") {
      parseTilesets(path, res);
    } else if (name === "Troops.json") {
      parseTroops(path, res);
    } else if (name === "Weapons.json") {
      parseWeapons(path, res);
    }

    if (name.length == 11 && name.startsWith("Map")) {
      parseMap(path, res);
    }
  }

  // parsing Animations.json last because we need the animationIds from other files
  const animationsPath = dataPath.join("Animations.json");
  if (!animationsPath.exists()) {
    logger.error(`Animations.json at ${animationsPath.fullPath} does not exist!`);
    return null;
  }

  parseAnimations(animationsPath, res, version);

  if (version === RPGMakerVersion.MZ) {
    // MZ uses Effekseer (https://github.com/effekseer/Effekseer) for the effects
    // we have to read and parse all .efkefc to find which files are unused

    const effectsPath = dataPath.getParent().join("effects");
    logger.debug(`Parsing effects in ${effectsPath.fullPath}`);

    const items = fs.readdirSync(effectsPath.fullPath, { encoding: "utf8" });
    for (const item of items) {
      const p = new Path(resolve(effectsPath.fullPath, item));
      if (!p.isFile()) continue;
      if (p.extension !== ".efkefc") continue;
      if (!res.effectNames.has(p.baseName)) continue;

      if (!parseEffect(p, effectsPath, res)) {
        logger.error(`Error parsing effect ${p}`);
        return null;
      }
    }
  }

  return res;
}

interface Event {
  code: number
  parameters: Array<unknown>
}

function parseEvents(events: Array<Event>, res: ParsedData) {
  for (const event of events) {
    const code = event.code;

    if (code === 0) continue;

    /*
      * RPG Maker code list (only important ones):
      * - 101: show text with actor face, [0] is face name
      *
      * - 132: change battle bgm, [0].name is bgm name
      * - 133: change victory me, [0].name is me name
      * - 139: change defeat me, [0].name is me name
      * - 140: change vehicle bgm, [1].name is bgm name
      *
      * - 205: set movement route (see 505)
      *
      * - 212: show animation, [1] is index of animation
      * - 231: show picture, [1] is picture name
      *
      * - 241: play bgm, [0].name is bgm name
      * - 245: play bgs, [0].name is bgs name
      * - 249: play me, [0].name is me name
      * - 250: play se, [0].name is se name
      * - 261: play movie, [0] is movie name
      *
      * - 282: change tileset, [0] is index of tileset (maybe not needed)
      * - 283: change battle back, [0] is battlebacks1 and [1] is battlebacks2
      * - 284: change parallax, [0] is image name
      *
      * - 322: change actor images, [1] is face name, [3] is character name, [5] is battler name
      * - 323: change vehicle image, [1] is face name
      *
      * - 337: show battle animation, [1] is index of animation
      *
      * - 505: argument of 205 where each item in parameters also has code+parameters fields:
      *      - 41: change character image, [0] is character name
      *      - 44: play se, [0].name is se name
    */

    if (code === 101) {
      const params = event.parameters as string[];
      specialPush(res.faceNames, params[0]);
    } else if (code === 132 || code === 133 || code === 139 || code === 241 || code === 245 || code === 249 || code === 250) {
      const params = event.parameters as Array<{ name: string }>;
      const audioName = params[0].name;

      if (code === 132 || code === 241) {
        specialPush(res.bgmNames, audioName);
      } else if (code === 133 || code === 139 || code === 249) {
        specialPush(res.meNames, audioName);
      } else if (code === 245) {
        specialPush(res.bgsNames, audioName);
      } else if (code === 250) {
        specialPush(res.seNames, audioName);
      }
    } else if (code === 140) {
      const params = event.parameters as Array<{ name: string }>;
      specialPush(res.bgmNames, params[1].name);
    } else if (code === 212 || code === 337) {
      const params = event.parameters as number[];
      if (params[1] !== -1)
        res.animationIds.add(params[1]);
    } else if (code === 231) {
      const params = event.parameters as string[];
      specialPush(res.pictureNames, params[1]);
    } else if (code === 261) {
      const params = event.parameters as string[];
      specialPush(res.movieNames, params[0]);
    } else if (code === 283) {
      const params = event.parameters as string[];
      specialPush(res.battleback1Names, params[0]);
      specialPush(res.battleback2Names, params[1]);
    } else if (code === 284) {
      const params = event.parameters as string[];
      specialPush(res.parallaxNames, params[0]);
    } else if (code === 322) {
      const params = event.parameters as string[];
      specialPush(res.faceNames, params[1]);
      specialPush(res.characterNames, params[3]);
      specialPush(res.actorBattlerNames, params[5]);
    } else if (code === 323) {
      const params = event.parameters as string[];
      specialPush(res.faceNames, params[1]);
    } else if (code === 505) {
      const params = event.parameters as Array<{
        code: number,
        parameters: unknown[]
      }>;

      const extraObj = params[0];
      const extraCode = extraObj.code;

      if (extraCode !== 41 && extraCode !== 44) continue;

      if (extraCode === 41) {
        const extraParams = extraObj.parameters as string[];
        specialPush(res.characterNames, extraParams[0]);
      } else if (extraCode === 44) {
        const extraParams = extraObj.parameters as Array<{
          name: string
        }>;

        specialPush(res.seNames, extraParams[0].name);
      }
    }
  }
}

interface Actor {
  battlerName: string,
  characterName: string,
  faceName: string
}

function parseActors(path: Path, res: ParsedData) {
  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const arr: Array<Actor | null> = JSON.parse(contents);

  for (const item of arr) {
    if (item === null) continue;

    /*
      * Actors.json:
      * battlerName => img/sv_actors/{battlerName}.png
      * characterName (w/ index) => img/characters/{characterName}.png
      * faceName (w/ index) => img/faces/{faceName}.png
    */

    specialPush(res.actorBattlerNames, item.battlerName);
    specialPush(res.characterNames, item.characterName);
    specialPush(res.faceNames, item.faceName);
  }
}

interface CommonEvent {
  list: Event[]
}

function parseCommonEvents(path: Path, res: ParsedData) {
  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const arr: Array<CommonEvent | null> = JSON.parse(contents);

  for (const item of arr) {
    if (item === null) continue;

    parseEvents(item.list, res);
  }
}

interface Enemey {
  battlerName: string
}

function parseEnemies(path: Path, res: ParsedData) {
  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const arr: Array<Enemey | null> = JSON.parse(contents);

  for (const item of arr) {
    if (item === null) continue;

    /*
      * Enemies.json:
      * battlerName => img/enemies/{battlerName}.png
    */

    specialPush(res.enemyBattlerNames, item.battlerName);
  }
}

interface Item {
  animationId: number
}

function parseItems(path: Path, res: ParsedData) {
  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const arr: Array<Item | null> = JSON.parse(contents);

  for (const item of arr) {
    if (item === null) continue;

    /*
      * Items.json:
      * animationId
    */

    if (item.animationId !== -1)
      res.animationIds.add(item.animationId);
  }
}

interface Skill {
  animationId: number
}

function parseSkills(path: Path, res: ParsedData) {
  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const arr: Array<Skill | null> = JSON.parse(contents);

  for (const item of arr) {
    if (item === null) continue;

    /*
      * Skills.json:
      * animationId
    */

    if (item.animationId !== -1)
      res.animationIds.add(item.animationId);
  }
}

interface System {
  airship: {
    bgm: {
      name: string
    },
    characterName: string
  },

  boat: {
    bgm: {
      name: string,
    },
    characterName: string
  },

  ship: {
    bgm: {
      name: string
    },
    characterName: string
  },

  battleback1Name: string,
  battleback2Name: string,

  battlerName: string,

  optSideView: boolean,

  title1Name: string,
  title2Name: string,

  sounds: Array<{
    name: string
  } | null>,

  battleBgm: {
    name: string
  },
  titleBgm: {
    name: string
  },

  defeatMe: {
    name: string
  },
  gameoverMe: {
    name: string
  },
  victoryMe: {
    name: string
  }
}

function parseSystem(path: Path, res: ParsedData) {
  /*
    * System.json:
    *
    * airship.bgm.name => audio/bgm/{}.m4a/ogg
    * airship.characterName => img/characters/{}.png
    *
    * boat.bgm.name => audio/bgm/{}.m4a/ogg
    * boat.characterName => img/characters/{}.png
    *
    * ship.bgm.name => audio/bgm/{}.m4a/ogg
    * ship.characterName => img/character/{}.png
    *
    * battleback1Name => img/battlebacks1/{}.png
    * battleback2Name => img/battlebacks2/{}.png
    *
    * battlerName => img/enemies/{}.png
    *
    * title1Name => img/titles1/{}.png
    * title2Name => img/titles2/{}.png
    *
    * sounds[n].name => audio/se/{}.m4a/ogg
    *
    * battleBgm.name => audio/bgm/{}.m4a/ogg
    * titleBgm.name => audio/bgm/{}.m4a/ogg
    *
    * defeatMe.name => audio/me/{}.m4a/ogg
    * gameoverMe.name => audio/me/{}.m4a/ogg
    * victoryMe.name => audio/me/{}.m4a/ogg
    *
  */

  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const system: System = JSON.parse(contents);

  specialPush(res.bgmNames, system.airship.bgm.name);
  specialPush(res.bgmNames, system.boat.bgm.name);
  specialPush(res.bgmNames, system.ship.bgm.name);

  specialPush(res.characterNames, system.airship.characterName);
  specialPush(res.characterNames, system.boat.characterName);
  specialPush(res.characterNames, system.ship.characterName);

  specialPush(res.battleback1Names, system.battleback1Name);
  specialPush(res.battleback2Names, system.battleback2Name);

  specialPush(res.enemyBattlerNames, system.battlerName);

  res.useSideView = system.optSideView;

  specialPush(res.title1Names, system.title1Name);
  specialPush(res.title2Names, system.title2Name);

  specialPush(res.bgmNames, system.battleBgm.name);
  specialPush(res.bgmNames, system.titleBgm.name);

  specialPush(res.meNames, system.defeatMe.name);
  specialPush(res.meNames, system.gameoverMe.name);
  specialPush(res.meNames, system.victoryMe.name);

  for (const sound of system.sounds) {
    if (sound === null) continue;
    specialPush(res.seNames, sound.name);
  }
}

interface Tileset {
  tilesetNames: string[]
}

function parseTilesets(path: Path, res: ParsedData) {
  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const arr: Array<Tileset | null> = JSON.parse(contents);

  for (const item of arr) {
    if (item === null) continue;

    /*
      * Tilesets.json:
      * tilesetNames[n] => img/tilesets/{}.png+{}.txt
    */

    for (const name of item.tilesetNames) {
      specialPush(res.tilesetNames, name);
    }
  }
}

interface Troop {
  pages: Array<CommonEvent | null>
}

function parseTroops(path: Path, res: ParsedData) {
  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const arr: Array<Troop | null> = JSON.parse(contents);

  for (const item of arr) {
    if (item === null) continue;

    /*
      * Troops.json:
      *
      * pages[n]:
      *  - list[n] => same as CommonEvents.json
    */

    for (const page of item.pages) {
      if (page === null) continue;

      parseEvents(page.list, res);
    }
  }

  return;
}

interface Weapon {
  animationId: number
}

function parseWeapons(path: Path, res: ParsedData) {
  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const arr: Array<Weapon | null> = JSON.parse(contents);

  for (const item of arr) {
    if (item === null) continue;

    /*
      * Weapons.json:
      * animationId
    */

    if (item.animationId !== -1)
      res.animationIds.add(item.animationId);
  }
}

interface Map {
  battleback1Name: string,
  battleback2Name: string,

  bgm: {
    name: string
  },
  bgs: {
    name: string
  },

  parallaxName: string,

  events: Array<{
    pages: Array<{
      image: {
        characterName: string
      },
      list: Event[]
    } | null>
  } | null>
}

function parseMap(path: Path, res: ParsedData) {
  /*
    * Map.json:
    *
    * battleback1Name => img/battlebacks1/{}.png
    * battleback2Name =>img/battlebacks2/{}.png
    *
    * bgm.name => audio/bgm/{}.m4a/ogg
    * bgs.name => audio/bgs/{}.m4a/ogg
    *
    * parallaxName => img/parallaxes/{}.png
    *
    * events[n].pages[n]:
    *  - image.characterName => img/characters/{}.png
    *  - list[n] => same as CommonEvents.json
  */

  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const map: Map = JSON.parse(contents);

  specialPush(res.battleback1Names, map.battleback1Name);
  specialPush(res.battleback2Names, map.battleback2Name);

  specialPush(res.bgmNames, map.bgm.name);
  specialPush(res.bgsNames, map.bgs.name);

  specialPush(res.parallaxNames, map.parallaxName);

  for (const event of map.events) {
    if (event === null) continue;
    for (const page of event.pages) {
      if (page === null) continue;

      specialPush(res.characterNames, page.image.characterName);

      parseEvents(page.list, res);
    }
  }
}

interface BaseAnimation {
  id: number,
}

interface MVAnimation extends BaseAnimation {
  animation1Name: string,
  animation2Name: string,
  timings: Array<{
    se: {
      name: string
    } | null
  } | null>
}

interface MZAnimation extends BaseAnimation {
  effectName: string,
  soundTimings: Array<{
    se: {
      name: string
    } | null
  } | null>
}

function parseAnimations(path: Path, res: ParsedData, version: RPGMakerVersion) {
  /*
    * Animations.json:
    *
    * MV:
    *  animation1Name => img/animations/{animation1Name}.png
    *  animation2Name => img/animations/{animation2Name}.png
    *  timings[i].se.name => audio/se/{name}.m4a/ogg
    * MZ:
    *  effectName => effects/{}.efkefc
    *  soundTimings[i].se.name => audio/se/{name}.m4a/ogg
  */

  const contents = fs.readFileSync(path.fullPath, { encoding: "utf-8" });
  const animations: Array<BaseAnimation | null> = JSON.parse(contents);

  for (const animation of animations) {
    if (animation === null) continue;

    const id = animation.id;
    if (!res.animationIds.has(id)) continue;

    if (version === RPGMakerVersion.MV) {
      const mvAnimation = animation as MVAnimation;

      specialPush(res.animationNames, mvAnimation.animation1Name);
      specialPush(res.animationNames, mvAnimation.animation2Name);

      for (const timing of mvAnimation.timings) {
        if (timing === null) continue;
        if (timing.se === null) continue;
        specialPush(res.seNames, timing.se.name);
      }
    } else if (version === RPGMakerVersion.MZ) {
      const mzAnimation = animation as MZAnimation;

      specialPush(res.effectNames, mzAnimation.effectName);

      for (const timing of mzAnimation.soundTimings) {
        if (timing === null) continue;
        if (timing.se === null) continue;
        specialPush(res.seNames, timing.se.name);
      }
    }
  }
}

function readResource(fd: number, effectsPath: Path, count: number, res: ParsedData, p: Path) {
  for (let i = 0; i < count; i++) {
    const lengthBuf = new Uint32Array(1);
    let bytesRead = fs.readSync(fd, lengthBuf, { length: lengthBuf.byteLength });
    if (bytesRead != lengthBuf.byteLength) {
      logger.error(`Tried reading ${lengthBuf.length} bytes but read ${bytesRead} from file ${p.fullPath}`);
      return false;
    }

    // name is utf16le encoded and null terminated
    // length is the amount of characters not bytes
    const nameBuf = new Uint16Array(lengthBuf[0]);
    bytesRead = fs.readSync(fd, nameBuf, { length: nameBuf.byteLength });
    if (bytesRead != nameBuf.byteLength) {
      logger.error(`Tried reading ${nameBuf.length} bytes but read ${bytesRead} from file ${p.fullPath}`);
      return false;
    }

    const name = String.fromCharCode(... nameBuf.slice(0, nameBuf.length - 1));
    const effectResPath = effectsPath.join(name);
    // specialPush(res.effectResources, effectResPath);
    res.effectResources.push(effectResPath);
  }

  return true;
}

function parseEffect(p: Path, effectsPath: Path, res: ParsedData): boolean {
  const fd = fs.openSync(p.fullPath, "r", 0o666);
  const stats = fs.fstatSync(fd);
  const filesize = stats.size;

  const headerBuf = new Uint32Array(2);
  let bytesRead = fs.readSync(fd, headerBuf, { length: headerBuf.byteLength });
  if (bytesRead != headerBuf.byteLength) {
    logger.error(`Tried reading ${headerBuf.length} bytes but read ${bytesRead} from file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  // 0x454B4645 = EFKE
  if (headerBuf[0] != 0x454B4645) {
    logger.error(`Invalid header in effect file ${p}: ${headerBuf[0]}`);
    fs.closeSync(fd);
    return false;
  }

  // version number
  if (headerBuf[1] != 0) {
    logger.error(`Invalid version in effect file ${p}: ${headerBuf[1]}`);
    fs.closeSync(fd);
    return false;
  }

  const infoChunkBuf = new Uint32Array(3);
  bytesRead = fs.readSync(fd, infoChunkBuf, { length: infoChunkBuf.byteLength });
  if (bytesRead != infoChunkBuf.byteLength) {
    logger.error(`Tried reading ${infoChunkBuf.length} bytes but read ${bytesRead} from file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  // 0x4F464E49 = INFO
  if (infoChunkBuf[0] != 0x4F464E49) {
    logger.error(`Invalid info chunk in effect file ${p}: ${infoChunkBuf[0]}`);
    fs.closeSync(fd);
    return false;
  }

  if (infoChunkBuf[1] >= filesize) {
    logger.error(`Info chunk in effect file ${p} is bigger than file itself: ${infoChunkBuf[1]}`);
    fs.closeSync(fd);
    return false;
  }

  // Textures
  const numTextureElementsBuf = new Uint32Array(1);
  bytesRead = fs.readSync(fd, numTextureElementsBuf, { length: numTextureElementsBuf.byteLength });
  if (bytesRead != numTextureElementsBuf.byteLength) {
    logger.error(`Tried reading ${numTextureElementsBuf.length} bytes but read ${bytesRead} from file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  if (!readResource(fd, effectsPath, numTextureElementsBuf[0], res, p)) {
    logger.error(`Error parsing resources in effect file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  const unknownBuf = new Uint32Array(1);
  bytesRead = fs.readSync(fd, unknownBuf, { length: unknownBuf.byteLength });
  if (bytesRead != unknownBuf.byteLength) {
    logger.error(`Tried reading ${unknownBuf.length} bytes but read ${bytesRead} from file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  // Alpha
  const numAlphaElementsBuf = new Uint32Array(1);
  bytesRead = fs.readSync(fd, numAlphaElementsBuf, { length: numAlphaElementsBuf.byteLength });
  if (bytesRead != numAlphaElementsBuf.byteLength) {
    logger.error(`Tried reading ${numAlphaElementsBuf.length} bytes but read ${bytesRead} from file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  if (!readResource(fd, effectsPath, numAlphaElementsBuf[0], res, p)) {
    logger.error(`Error parsing resources in effect file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  // Models
  const numModelElementsBuf = new Uint32Array(1);
  bytesRead = fs.readSync(fd, numModelElementsBuf, { length: numModelElementsBuf.byteLength });
  if (bytesRead != numModelElementsBuf.byteLength) {
    logger.error(`Tried reading ${numModelElementsBuf.length} bytes but read ${bytesRead} from file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  if (!readResource(fd, effectsPath, numModelElementsBuf[0], res, p)) {
    logger.error(`Error parsing resources in effect file ${p.fullPath}`);
    fs.closeSync(fd);
    return false;
  }

  fs.closeSync(fd);
  return true;
}

function specialPush(container: Set<string>, s: string) {
  if (s === undefined) return;
  if (s === null) return;
  if (s.length === 0) return;
  container.add(normalizeName(s));
}

function normalizeName(s: string) {
  if (sep === "\\")
    return s.replace("/", sep);
  return s.replace("\\", sep);
}