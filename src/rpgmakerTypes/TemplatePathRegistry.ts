import Path from "../io/Path";

export interface TemplatePathRegistry {
  top: Path,

  // chromium portable native client, Windows only
  pnacl: Path,

  // nwjs.app, OSX only
  nwjs_app: Path
}

export function createTemplatePathRegistry(p: Path): TemplatePathRegistry {
  const res: TemplatePathRegistry = {
    top: p,

    pnacl: p.join("pnacl"),

    nwjs_app: p.join("nwjs.app"),
  };

  return res;
}