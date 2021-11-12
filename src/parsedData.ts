import fs from "fs";
import { resolve } from "path";

import { Path } from "./ioTypes";
import { RPGMakerVersion } from "./rpgmakerTypes";

export interface ParsedData {
    //Actors.json
    //img/sv_actors/{}.png
    actorBattlerNames: string[],

    //Animations.json
    animationIds: number[],
    //MV only: img/animations/{}.png
    animationNames: string[],
    //MZ only: effects/{}.efkefc
    effectNames: string[],
    //MZ only: effects/{}
    effectResources: string[],

    //Enemies.json
    //img/enemies/{}.png
    enemyBattlerNames: string[],

    //Tilesets.json
    //img/tilesets/{}.png+{}.txt
    tilesetNames: string[],

    //System.json
    //img/titles1/{}.png
    title1Names: string[],
    //img/titles2/{}.png
    title2Names: string[],

    //other
    //img/characters/{}.png
    characterNames: string[],
    //img/faces/{}.png
    faceNames: string[],

    //audio/bgm/{}.m4a/ogg
    bgmNames: string[],
    //audio/bgs/{}.m4a/ogg
    bgsNames: string[],
    //audio/me/{}.m4a/ogg
    meNames: string[],
    //audio/se/{}.m4a/ogg
    seNames: string[],

    //img/pictures/{}.png
    pictureNames: string[],
    //movies/{}.webm
    movieNames: string[],

    //img/battlebacks1/{}.png
    battleback1Names: string[],
    //img/battlebacks2/{}.png
    battleback2Names: string[],
    //img/parallaxes/{}.png
    parallaxNames: string[],
}

export function parseData(dataPath: Path, version: RPGMakerVersion): ParsedData {
  const res: ParsedData = {
    actorBattlerNames: [],
    animationIds: [],
    animationNames: [],
    battleback1Names: [],
    battleback2Names: [],
    bgmNames: [],
    bgsNames: [],
    characterNames: [],
    effectNames: [],
    effectResources: [],
    enemyBattlerNames: [],
    faceNames: [],
    meNames: [],
    movieNames: [],
    parallaxNames: [],
    pictureNames: [],
    seNames: [],
    tilesetNames: [],
    title1Names: [],
    title2Names: []
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
    // TODO:
  }

  parseAnimations(animationsPath, res, version);

  if (version === RPGMakerVersion.MZ) {
    // TODO: effect parsing
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
      * - 212: show animation, [2] is index of animation
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
      res.faceNames.push(params[0]);
    } else if (code === 132 || code === 133 || code === 139 || code === 241 || code === 245 || code === 249 || code === 250) {
      const params = event.parameters as Array<{ name: string }>;
      const audioName = params[0].name;

      if (code === 132 || code === 241) {
        res.bgmNames.push(audioName);
      } else if (code === 133 || code === 139 || code === 249) {
        res.meNames.push(audioName);
      } else if (code === 245) {
        res.bgsNames.push(audioName);
      } else if (code === 250) {
        res.seNames.push(audioName);
      }
    } else if (code === 140) {
      const params = event.parameters as Array<{ name: string }>;
      res.bgmNames.push(params[1].name);
    } else if (code === 212 || code === 337) {
      const params = event.parameters as number[];
      res.animationIds.push(code === 212 ? params[2] : params[1]);
    } else if (code === 231) {
      const params = event.parameters as string[];
      res.pictureNames.push(params[1]);
    } else if (code === 261) {
      const params = event.parameters as string[];
      res.movieNames.push(params[0]);
    } else if (code === 283) {
      const params = event.parameters as string[];
      res.battleback1Names.push(params[0]);
      res.battleback2Names.push(params[1]);
    } else if (code === 284) {
      const params = event.parameters as string[];
      res.parallaxNames.push(params[0]);
    } else if (code === 322) {
      const params = event.parameters as string[];
      res.faceNames.push(params[1]);
      res.characterNames.push(params[3]);
      res.actorBattlerNames.push(params[5]);
    } else if (code === 323) {
      const params = event.parameters as string[];
      res.faceNames.push(params[1]);
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
        res.characterNames.push(extraParams[0]);
      } else if (extraCode === 44) {
        const extraParams = extraObj.parameters as Array<{
          name: string
        }>;

        res.seNames.push(extraParams[0].name);
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

    res.actorBattlerNames.push(item.battlerName);
    res.characterNames.push(item.characterName);
    res.faceNames.push(item.faceName);
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

    res.enemyBattlerNames.push(item.battlerName);
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

    res.animationIds.push(item.animationId);
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

    res.animationIds.push(item.animationId);
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

  res.bgmNames.push(system.airship.bgm.name);
  res.bgmNames.push(system.boat.bgm.name);
  res.bgmNames.push(system.ship.bgm.name);

  res.characterNames.push(system.airship.characterName);
  res.characterNames.push(system.boat.characterName);
  res.characterNames.push(system.ship.characterName);

  res.battleback1Names.push(system.battleback1Name);
  res.battleback2Names.push(system.battleback2Name);

  res.enemyBattlerNames.push(system.battlerName);

  res.title1Names.push(system.title1Name);
  res.title2Names.push(system.title2Name);

  res.bgmNames.push(system.battleBgm.name);
  res.bgmNames.push(system.titleBgm.name);

  res.meNames.push(system.defeatMe.name);
  res.meNames.push(system.gameoverMe.name);
  res.meNames.push(system.victoryMe.name);

  for (const sound of system.sounds) {
    if (sound === null) continue;
    res.seNames.push(sound.name);
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
      res.tilesetNames.push(name);
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

    res.animationIds.push(item.animationId);
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

  res.battleback1Names.push(map.battleback1Name);
  res.battleback2Names.push(map.battleback2Name);

  res.bgmNames.push(map.bgm.name);
  res.bgsNames.push(map.bgs.name);

  res.parallaxNames.push(map.parallaxName);

  for (const event of map.events) {
    if (event === null) continue;
    for (const page of event.pages) {
      if (page === null) continue;

      res.characterNames.push(page.image.characterName);

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
    if (!res.animationIds.includes(id)) continue;

    if (version === RPGMakerVersion.MV) {
      const mvAnimation = animation as MVAnimation;

      res.animationNames.push(mvAnimation.animation1Name);
      res.animationNames.push(mvAnimation.animation2Name);

      for (const timing of mvAnimation.timings) {
        if (timing === null) continue;
        if (timing.se === null) continue;
        res.seNames.push(timing.se.name);
      }
    } else if (version === RPGMakerVersion.MZ) {
      const mzAnimation = animation as MZAnimation;

      res.effectNames.push(mzAnimation.effectName);

      for (const timing of mzAnimation.soundTimings) {
        if (timing === null) continue;
        if (timing.se === null) continue;
        res.seNames.push(timing.se.name);
      }
    }
  }
}