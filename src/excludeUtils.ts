import { Path } from "./ioTypes";
import { ParsedData } from "./parsedData";
import { PathRegistry } from "./paths";

export function filterUnusedFiles(path: Path, parsedData: ParsedData, pathRegistry: PathRegistry): boolean {
  const name = path.baseName;

  // Audio
  if (path.isInDirectory(pathRegistry.audio_bgm)) {
    return !parsedData.bgmNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.audio_bgs)) {
    return !parsedData.bgsNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.audio_me)) {
    return !parsedData.meNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.audio_se)) {
    return !parsedData.seNames.includes(name);
  }

  // Images
  if (path.isInDirectory(pathRegistry.img_animations)) {
    return !parsedData.animationNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_battlebacks1)) {
    return !parsedData.battleback1Names.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_battlebacks2)) {
    return !parsedData.battleback2Names.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_characters)) {
    return !parsedData.characterNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_enemies) || path.isInDirectory(pathRegistry.img_sv_enemies)) {
    return !parsedData.enemyBattlerNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_faces)) {
    return !parsedData.faceNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_parallaxes)) {
    return !parsedData.parallaxNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_pictures)) {
    return !parsedData.pictureNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_sv_actors)) {
    return !parsedData.actorBattlerNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_tilesets)) {
    return !parsedData.tilesetNames.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_titles1)) {
    return !parsedData.title1Names.includes(name);
  }

  if (path.isInDirectory(pathRegistry.img_titles2)) {
    return !parsedData.title2Names.includes(name);
  }

  // Other
  if (path.isInDirectory(pathRegistry.movies)) {
    return !parsedData.movieNames.includes(name);
  }

  return false;
}