import { Path } from "./io/Path";
import { ParsedData } from "./parsedData";
import { ProjectPathRegistry } from "./rpgmaker/pathRegistries";
import { RPGMakerVersion } from "./rpgmakerTypes";

function specialInclude(container: Set<string>, path: Path, topPath: Path): boolean {
  const name = path.baseName;

  if (path.getParent().equals(topPath)) {
    return container.has(name);
  }

  // you can actually have sub-directories in various asset folders which makes the above check necessary
  // we need to check if we are directly in the asset dir, eg in img/characters or in img/characters/foo

  // eg:
  // in a Map.json file:
  // "characterName": "abc" would mean the file is img/characters/abc.png
  // "characterName": "foo/abc" would mean the file is img/characters/foo/abc.png

  // making "../img/characters/foo/bar/baz/../abc.png" into "foo/bar/baz/../abc"
  const relative = path.relativeTo(topPath);
  const actualName = relative.slice(0, relative.length - path.extension.length);
  return container.has(actualName);
}

export function filterUnusedFiles(path: Path, parsedData: ParsedData, pathRegistry: ProjectPathRegistry, version: RPGMakerVersion): boolean {
  // Plugin files
  if (parsedData.pluginPaths !== undefined) {
    if (parsedData.pluginPaths.some(p => p.equals(path)))
      return false;
  }

  // Audio
  if (path.isInDirectory(pathRegistry.audio_bgm)) {
    return !specialInclude(parsedData.bgmNames, path, pathRegistry.audio_bgm);
  }

  if (path.isInDirectory(pathRegistry.audio_bgs)) {
    return !specialInclude(parsedData.bgsNames, path, pathRegistry.audio_bgs);
  }

  if (path.isInDirectory(pathRegistry.audio_me)) {
    return !specialInclude(parsedData.meNames, path, pathRegistry.audio_me);
  }

  if (path.isInDirectory(pathRegistry.audio_se)) {
    return !specialInclude(parsedData.seNames, path, pathRegistry.audio_se);
  }

  // Images
  if (path.isInDirectory(pathRegistry.img_animations)) {
    return !specialInclude(parsedData.animationNames, path, pathRegistry.img_animations);
  }

  if (path.isInDirectory(pathRegistry.img_battlebacks1)) {
    return !specialInclude(parsedData.battleback1Names, path, pathRegistry.img_battlebacks1);
  }

  if (path.isInDirectory(pathRegistry.img_battlebacks2)) {
    return !specialInclude(parsedData.battleback2Names, path, pathRegistry.img_battlebacks2);
  }

  if (path.isInDirectory(pathRegistry.img_characters)) {
    return !specialInclude(parsedData.characterNames, path, pathRegistry.img_characters);
  }

  // there are two display modes: front-view and side-view
  // this dictates what images are used
  // font-view will use the images in img/enemies
  // side-view will use the images in img/sv_enemies
  // weirdly enough, this doesn't matter for actors
  if (parsedData.useSideView) {
    if (path.isInDirectory(pathRegistry.img_enemies)) {
      return true;
    }

    if (path.isInDirectory(pathRegistry.img_sv_enemies)) {
      return !specialInclude(parsedData.enemyBattlerNames, path, pathRegistry.img_sv_enemies);
    }
  } else {
    if (path.isInDirectory(pathRegistry.img_sv_enemies)) {
      return true;
    }

    if (path.isInDirectory(pathRegistry.img_enemies)) {
      return !specialInclude(parsedData.enemyBattlerNames, path, pathRegistry.img_enemies);
    }
  }

  if (path.isInDirectory(pathRegistry.img_faces)) {
    return !specialInclude(parsedData.faceNames, path, pathRegistry.img_faces);
  }

  if (path.isInDirectory(pathRegistry.img_parallaxes)) {
    return !specialInclude(parsedData.parallaxNames, path, pathRegistry.img_parallaxes);
  }

  if (path.isInDirectory(pathRegistry.img_pictures)) {
    return !specialInclude(parsedData.pictureNames, path, pathRegistry.img_pictures);
  }

  if (path.isInDirectory(pathRegistry.img_sv_actors)) {
    return !specialInclude(parsedData.actorBattlerNames, path, pathRegistry.img_sv_actors);
  }

  if (path.isInDirectory(pathRegistry.img_tilesets)) {
    return !specialInclude(parsedData.tilesetNames, path, pathRegistry.img_tilesets);
  }

  if (path.isInDirectory(pathRegistry.img_titles1)) {
    return !specialInclude(parsedData.title1Names, path, pathRegistry.img_titles1);
  }

  if (path.isInDirectory(pathRegistry.img_titles2)) {
    return !specialInclude(parsedData.title2Names, path, pathRegistry.img_titles2);
  }

  // Other
  if (path.isInDirectory(pathRegistry.movies)) {
    return !specialInclude(parsedData.movieNames, path, pathRegistry.movies);
  }

  // MZ effects
  if (version === RPGMakerVersion.MZ) {
    if (path.isInDirectory(pathRegistry.effects)) {
      if (specialInclude(parsedData.effectNames, path, pathRegistry.effects)) return false;
      // if (parsedData.effectNames.includes(name)) return false;
      if (parsedData.effectResources.some(x => x.equals(path))) return false;
      return true;
    }
  }

  return false;
}