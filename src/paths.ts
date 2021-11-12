import { Path } from "./ioTypes";

// TODO: MZ paths

export interface PathRegistry {
  audio: Path,
  audio_bgm: Path,
  audio_bgs: Path,
  audio_me: Path,
  audio_se: Path,

  data: Path,
  effects: Path, // MZ only
  fonts: Path,
  icon: Path,

  img: Path,
  img_animations: Path, // MV only
  img_battlebacks1: Path,
  img_battlebacks2: Path,
  img_characters: Path,
  img_enemies: Path,
  img_faces: Path,
  img_parallaxes: Path,
  img_pictures: Path,
  img_sv_actors: Path,
  img_sv_enemies: Path,
  img_system: Path,
  img_tilesets: Path,
  img_titles1: Path,
  img_titles2: Path,

  js: Path,
  movies: Path,
  save: Path
}

export function createPathRegistry(p: Path): PathRegistry {
  // TODO: maybe a better solution than this

  const img = p.join("img");
  const audio = p.join("audio");

  const res: PathRegistry = {
    audio,
    audio_bgm: audio.join("bgm"),
    audio_bgs: audio.join("bgs"),
    audio_me: audio.join("me"),
    audio_se: audio.join("se"),

    data: p.join("data"),
    effects: p.join("effects"),
    fonts: p.join("fonts"),
    icon: p.join("icon"),

    img,
    img_animations: img.join("animations"),
    img_battlebacks1: img.join("battlebacks1"),
    img_battlebacks2: img.join("battlebacks2"),
    img_characters: img.join("characters"),
    img_enemies: img.join("enemies"),
    img_faces: img.join("faces"),
    img_parallaxes: img.join("parallaxes"),
    img_pictures: img.join("pictures"),
    img_sv_actors: img.join("sv_actors"),
    img_sv_enemies: img.join("sv_enemies"),
    img_system: img.join("system"),
    img_tilesets: img.join("tilesets"),
    img_titles1: img.join("titles1"),
    img_titles2: img.join("titles2"),

    js: p.join("js"),
    movies: p.join("movies"),
    save: p.join("save")
  };

  return res;
}