# RPGMPacker

![CI CMake Build](https://github.com/erri120/rpgmpacker/workflows/CI%20CMake%20Build/badge.svg)
![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/erri120/rpgmpacker)

Simple CLI program for packaging RPG Maker games to use in a CI/CD workflow.

- Supported RPG Maker versions:
  - RPG Maker MV
    - Windows
    - OSX
    - Linux
    - Browser
    - Mobile: **not supported**
  - RPG Maker MZ
    - Windows
    - OSX: **not supported**
    - Browser/Mobile
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
                           (default: false)
      --encryptAudio       Enable Audio Encryption using encryptionKey.
                           (default: false)
      --encryptionKey arg  Encryption Key for Images or Audio, either
                           encryptImages or encryptAudio have to be set
      --hardlinks          Use hardlinks instead of creating copies.
                           (default: false)
      --cache              Use a path cache for already encrypted files when
                           multi-targeting and using hardlinks. (default:
                           false)
      --threads arg        Amount of worker threads to use. Min: 1, Max: 10
                           (default: 2)
  -d, --debug              Enable debugging output (very noisy). (default:
                           false)
  -h, --help               Print usage
```

The output directory will be cleaned before execution and each platform will get it's own sub directory:

```txt
/output
/output/Windows
/output/Linux
/output/Browser
```

It is recommended to use the hardlink option for faster speeds and less disk usage. You can't hardlink across different drives and the program will check if hardlinks can be used beforehand. If you don't know what hardlinks are then take a look at the [Wikipedia article](https://en.wikipedia.org/wiki/Hard_link), the [Win32 docs](https://docs.microsoft.com/en-us/windows/win32/fileio/hard-links-and-junctions) or an [article from Linux Handbook](https://linuxhandbook.com/hard-link/).

The cache option is recommend to use when you encrypt your files, use the hardlink option and target multiple platforms. It works by encrypting the files only once, caching the path and subsequently encryptions will just hardlink to the already encrypted file reducing operation speed and disk usage.

Since this application is mostly just doing IO stuff, I added parallel execution that you can configure with the threads option which will set the amount of worker threads to use. I recommend something between 2 and 4 as anything above 4 is not having that much of an impact:

| Worker Threads | Time (first platform) | Time (second platform) |
|----------------|-----------------------|------------------------|
| 1 | `4.84sec` | `0.16sec` |
| 2 | `3.28sec` | `0.11sec` |
| 4 | `1.84sec` | `0.09sec` |
| 6 | `1.66sec` | `0.10sec` |
| 8 | `1.31sec` | `0.09sec` |

For testing I used the following options:

- Input: `E:\\Projects\\RPGMakerTest\\src\\MZProject1`
- Output: `E:\\Projects\\RPGMakerTest\\out-c`
- RPG Maker: `C:\\Program Files\\KADOKAWA\\RPGMZ`
- Encryption Key: `1337`
- Encrypt Images: `true`
- Encrypt Audio: `true`
- Platforms: `win,browser`
- Debug: `false`
- Use Hardlinks: `true`
- Use Cache: `true`
- RPGMaker Version: `MZ`
- Can use hardlinking from RPGMaker Folder to Output: `false`
- Can use hardlinking from Input Folder to Output: `true`

### Example

```ps1
.\RPGMPacker.exe -i "E:\\Projects\\RPGMakerTest\\src\\Project1" -o "E:\\Projects\\RPGMakerTest\\out-c" --rpgmaker "M:\\SteamLibrary\\steamapps\\common\\RPG Maker MV" --platforms win,linux,osx --encryptImages --encryptAudio --encryptionKey="1337" --hardlinks --cache
```

## How it works

RPG Maker has a very _simple_ way of deploying your game due to the fact that the game is in Javascript and no compilation is needed. What the program does is copy your project files as well as the runtime to the output directory. The runtime can be found in the RPG Maker installation folder and some project files are filtered out before copying (eg only `.ogg` audio files on Desktop).

### Encryption

The encryption method used by RPG Maker is a joke and only makes sense if you don't want every user to easily access the audio and image files.

RPG Maker starts by writing a new header:

```txt
52 50 47 4D 56 00 00 00 00 03 01 00 00 00 00 00
```

The file signature is 8 bytes long: `52 50 47 4D 56 00 00 00 00` (`52 50 47 4D 56` = `RPGMV`), then 3 bytes for the version number: `00 03 01` and the rest is just filler (This file signature is the same in MV and MZ for whatever reason).

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
- [Taskflow](https://github.com/taskflow/taskflow) ([MIT](https://github.com/taskflow/taskflow/blob/master/LICENSE))

## License

See [LICENSE](LICENSE).
