# RPGMPacker

Simple CLI program for packaging RPG Maker games to use in a CI/CD workflow.

- Supported RPG Maker versions:
  - RPG Maker MV
  - RPG Maker MZ
- Supported deployment features:
  - audio filtering depending on platform
  - image and audio encryption with an encryption key
  - using hardlinks instead of creating copies
  - deploying for multiple platforms at once

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

RPG Maker has a very _simple_ way of deploying your game due to the fact that the game is in Javascript and no compilation is needed. What the program does is copy your project files as well as the runtime to the output directory. The runtime can be found in the RPG Maker installation folder (`nwjs-win`, `nwjs-lnx`, `nwjs-osx-unsigned`) and some project files are filtered out before copying (eg only `.ogg` audio files on Desktop).

### Encryption

The encryption method used by RPG Maker is a joke and only makes sense if you don't want every user to easily access the audio and image files.

RPG Maker starts by writing a new header:

```txt
52 50 47 4D 56 00 00 00 00 03 01 00 00 00 00 00
```

The file signature is 8 bytes long: `52 50 47 4D 56 00 00 00 00` (`52 50 47 4D 56` = `RPGMV`), then 3 bytes for the version number: `00 03 01` and the rest is just filler.

The provided encryption key will be run through an MD5 algorithm: `1337` -> `e48e13207341b6bffb7fb1622282247b` and the first 16 bytes of the file will be "encrypted" in an iteration with `buffer[i] = buffer[i] ^ key[i]`. This XOR operation is only applied on the first 16 bytes and the rest of the file stays the same.

Finally the MD5 hash of the encryption key will be put into `data/System.json`:

```JSON
    "hasEncryptedImages": true,
    "hasEncryptedAudio": true,
    "encryptionKey": "e48e13207341b6bffb7fb1622282247b"
```

Since the game is in Javascript you can easily just go to `js/rpg_core.js` and find the decryption functions as those are not even minified.

## Libraries used

Managed with [CPM](https://github.com/TheLartians/CPM.cmake).

- [cxxopts](https://github.com/jarro2783/cxxopts) ([MIT](https://github.com/jarro2783/cxxopts/blob/master/LICENSE))
- [spdlog](https://github.com/gabime/spdlog) ([MIT](https://github.com/gabime/spdlog/blob/v1.x/LICENSE))
- [ghc::filesystem](https://github.com/gulrak/filesystem) ([MIT](https://github.com/gulrak/filesystem/blob/master/LICENSE))
- MD5 hash functions from [stbrumme/hash-library](https://github.com/stbrumme/hash-library) ([zlib](https://github.com/stbrumme/hash-library/blob/master/LICENSE))
