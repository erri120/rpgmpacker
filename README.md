# RPGMPacker

## Usage

```txt
  RPGMPacker [OPTION...]

  -i, --input arg          (REQUIRED) Input folder containing the .rpgproj
                           file
  -o, --output arg         (REQUIRED) Output folder
      --rpgmaker arg       (REQUIRED) RPG Maker installation folder
  -p, --platforms arg      (REQUIRED) Platforms to build for, this can take a
                           list of platforms delimited with a comma or just
                           one value. Possible values: win, osx, linux,
                           browser, mobile
      --encryptImages      Enable Image Encryption using encryptionKey.
                           Default: false
      --encryptAudio       Enable Audio Encryption using encryptionKey.
                           Default: false
      --encryptionKey arg  Encryption Key for Images or Audio, either
                           encryptImages or encryptAudio have to be set
      --hardlinks          Use hardlinks instead of creating copies. Default:
                           false
  -d, --debug              Enable debugging. Default: false
  -h, --help               Print usage
```

It is recommended to use the hardlink option for faster speeds and less disk usage. You can't hardlink across different drives and the program will check if hardlinks can be used beforehand. If you don't know what hardlinks are then take a look at the [Wikipedia article](https://en.wikipedia.org/wiki/Hard_link), the [Win32 docs](https://docs.microsoft.com/en-us/windows/win32/fileio/hard-links-and-junctions) or an [article from Linux Handbook](https://linuxhandbook.com/hard-link/).

### Example

```ps1
.\RPGMPacker.exe -i "E:\\Projects\\RPGMakerTest\\src\\Project1" -o "E:\\Projects\\RPGMakerTest\\out-c" --rpgmaker "M:\\SteamLibrary\\steamapps\\common\\RPG Maker MV" --platforms win,linux,osx --hardlinks
```

## How it works

RPG Maker has a very _simple_ way of deploying your game which is based on the fact that the games are in Javascript and [NW.js](https://nwjs.io/) is used as a runtime. The first thing to do is copy over the template files from the RPG Maker installation folder. If you open that folder you will find `nwjs-win`, `nwjs-lnx` and `nwjs-osx-unsigned`. The contents of those folders will be copied over to the output directory. Next up are your project files from which some are filtered out depending on platform (eg for audio: desktop has only `.ogg` and mobile only `.m4a` files).

## Libraries used

Managed with [CPM](https://github.com/TheLartians/CPM.cmake).

- [cxxopts](https://github.com/jarro2783/cxxopts) ([MIT](https://github.com/jarro2783/cxxopts/blob/master/LICENSE))
- [spdlog](https://github.com/gabime/spdlog) ([MIT](https://github.com/gabime/spdlog/blob/v1.x/LICENSE))
- [ghc::filesystem](https://github.com/gulrak/filesystem) ([MIT](https://github.com/gulrak/filesystem/blob/master/LICENSE))
