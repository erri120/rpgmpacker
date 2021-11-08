import { Path } from "./ioTypes";

export interface PathRegistry {
  Audio: Path,
  Data: Path,
  Fonts: Path,
  Icon: Path,

  Img: Path,
  Img_System: Path,

  JS: Path,
  Movies: Path,
  Save: Path
}

export function createPathRegistry(p: Path): PathRegistry {
  const img = p.join("img");

  const res: PathRegistry = {
    Audio: p.join("audio"),
    Data: p.join("data"),
    Fonts: p.join("fonts"),
    Icon: p.join("icon"),

    Img: img,
    Img_System: img.join("system"),

    JS: p.join("js"),
    Movies: p.join("movies"),
    Save: p.join("save")
  };

  return res;
}