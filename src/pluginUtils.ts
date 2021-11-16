import vm from "vm";
import fs from "fs";
import path from "path";

import logger from "./logging";
import { Path } from "./io/Path";

export interface PluginOptions {
  name: string,
  status: boolean,
  description: string,
  parameters: {
    [key: string]: string
  }
}

export function parsePlugins(jsFolder: Path, inputFolder: Path): Path[] | null {
  logger.debug(`Parsing plugins in ${jsFolder.fullPath}`);

  const pluginsFile = jsFolder.join("plugins.js");
  if (!pluginsFile.exists()) {
    logger.error(`plugins.js does not exist at ${pluginsFile.fullPath}`);
    return null;
  }

  const contents = fs.readFileSync(pluginsFile.fullPath, { encoding: "utf-8" });
  const context = vm.createContext({});

  vm.runInContext(contents, context);
  const plugins = context.$plugins as Array<PluginOptions>;

  const pluginsPath = jsFolder.join("plugins");

  const comments: { [name: string]: PluginComment } = {};

  for (const plugin of plugins) {
    const pluginFile = pluginsPath.join(plugin.name + ".js");
    const pluginComment = parsePluginFile(pluginFile);
    if (pluginComment === null) {
      logger.error(`Error parsing plugin file ${pluginFile.fullPath}`);
      return null;
    }

    comments[plugin.name] = pluginComment;
  }

  const res: Path[] = [];

  // TODO: make this better
  const getExtension = (relativePart: string) => {
    if (relativePart.startsWith("img"))
      return ".png";
    if (relativePart.startsWith("audio"))
      return ".ogg";
    return "";
  };

  for (const pluginName in comments) {
    const comment = comments[pluginName];
    // requiredAssets are relative to the input folder: "img/foo/bar.png"
    res.push(... comment.requiredAssets.map(s => inputFolder.join(s)));

    const settings = plugins.find(p => p.name === pluginName);
    if (settings === undefined) {
      // no settings so we just use the default values if there are any
      res.push(... comment.parameters
        .filter(p => p.required && p.default !== undefined)
        .map(p => {
          if (p.dir !== undefined) {
            return inputFolder.join(p.dir + p.default + getExtension(p.dir));
          } else {
            // dir is not required for animations
            return inputFolder.join("img/animations" + path.sep + p.default + getExtension("img"));
          }
        }));
    } else {
      for (const param of comment.parameters) {
        const paramSettings = settings.parameters[param.name].trim();
        if (paramSettings.length === 0 && param.default === undefined) {
          continue;
        }

        if (paramSettings.length === 0 && param.default !== undefined) {
          logger.warn(`Parameter ${param.name} is empty for plugin ${pluginName} but not undefined in comment!`);
          continue;
        }

        if (param.dir !== undefined) {
          res.push(inputFolder.join(param.dir + paramSettings + getExtension(param.dir)));
        } else {
          res.push(inputFolder.join("img/animations" + path.sep + paramSettings + getExtension("img")));
        }
      }
    }
  }

  return res;
}

export enum PluginParameterType {
  File = "file",
  Animation = "animation"
}

export interface PluginParameter {
  name: string,
  default: string | undefined,
  required: boolean,
  // not required for animations
  dir: string | undefined,
  type: PluginParameterType
}

export interface PluginComment {
  requiredAssets: string[],
  parameters: PluginParameter[]
}

function parsePluginFile(pluginFile: Path): PluginComment | null {
  if (!pluginFile.exists()) {
    logger.error(`Plugin at ${pluginFile} does not exist!`);
    return null;
  }

  const res: PluginComment = {
    parameters: [],
    requiredAssets: []
  };

  const contents = fs.readFileSync(pluginFile.fullPath, { encoding: "utf-8" });

  const startIndex = contents.indexOf("/*:");
  const endIndex = contents.indexOf("*/");

  const pluginParamsText = contents
    .slice(startIndex, endIndex)
    .split("\n")
    .slice(1)
    .map(s => s.replace("*", "").trim())
    .filter(s => s.length !== 0)
    .map(s => {
      const spaceIndex = s.indexOf(" ");
      const type = s.slice(0, spaceIndex);
      const value = s.slice(spaceIndex + 1);
      return {type, value};
    });

  let inParam = false;
  let paramName: string | undefined;
  let paramDefault: string | undefined;
  let paramRequire = false;
  let paramDir: string | undefined;
  let paramType: PluginParameterType | undefined;
  for (let i = 0; i < pluginParamsText.length; i++) {
    const { type, value } = pluginParamsText[i];

    if (type === "@requiredAssets") {
      res.requiredAssets.push(value);
      continue;
    }

    if (type === "@param") {
      if (inParam) {
        if (paramName !== undefined) {
          if (paramType !== undefined) {
            res.parameters.push({
              name: paramName,
              default: paramDefault,
              required: paramRequire,
              dir: paramDir,
              type: paramType
            });
          }
        } else {
          logger.warn(`Parameter has no name in file ${pluginFile.fullPath}`);
        }

        paramName = undefined;
        paramDefault = undefined;
        paramRequire = false;
        paramDir = undefined;
        paramType = undefined;
      }

      inParam = true;
      paramName = value;
      continue;
    }

    if (!inParam) continue;

    if (type === "@default") {
      paramDefault = value;
    } else if (type === "@require") {
      paramRequire = value === "1";
    } else if (type === "@dir") {
      paramDir = value;
    } else if (type === "@type") {
      const lowerCaseValue = value.toLowerCase();
      if (lowerCaseValue === "file") {
        paramType = PluginParameterType.File;
      } else if (lowerCaseValue === "animation") {
        paramType = PluginParameterType.Animation;
      } else {
        logger.warn(`Unknown type: ${lowerCaseValue} in ${pluginFile.fullPath}`);
      }
    }
  }

  return res;
}