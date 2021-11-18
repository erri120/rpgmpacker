# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog][Keep a Changelog] and this project adheres to [Semantic Versioning][Semantic Versioning].

## [Unreleased]

## [Released]

## [2.0.1] - 2021-11-18

Fixing build problems...

## [2.0.0] - 2021-11-18

This release features a complete rework of the tool. I decided to move away from C++ and remade the entire thing in TypeScript. You can read more about this decision [here](https://erri120.github.io/posts/2021-11-18/). This rework carries over all features from the previous version and adds more features on top:

- Plugins can now be parsed for the exclude-unused feature
- Paths are handled better and multiple filter-related issues have been fixed with it
- the Side-View and Front-View setting is now used when excluding unused assets

Here is a list of arguments that got removed, changed or added:

- `--config`: removed
- `--input`: removed `-i`
- `--output`: removed `-o`
- `--platforms`: removed `-p`, possible values also changed to `"Windows", "OSX", "Linux", "Browser", "Mobile"`
- `--cache`: removed
- `--threads`: removed
- `--noempty`: added, this will remove all empty folders from the final output

All of the short-hand arguments (single character) have been removed. The names of the platforms have also changed and you need to put the platform argument at the end of your command.

Please open an issue if you have any problems going from 1.x to 2.0.0.

## [1.6.2] - 2021-04-07

### Added

- Added `--version` argument for printing the program version ([#40](https://github.com/erri120/rpgmpacker/issues/40))

## [1.6.1] - 2021-04-05

### Added

- Change file `nw` to `Game` on Linux for MZ ([#39](https://github.com/erri120/rpgmpacker/issues/39))

## [1.6.0] - 2021-04-05

This release changed the structure of the project so it is easier to work with. I split the project into 3 sub-projects:

1) A main executable project
2) A lib project containing everything without the CLI
3) A test project that uses the functions in the lib project

This change does not change anything for the end-consumer of the tool but is a huge change on the development side of things so I wanted to include it in the changelog.

### Added

- Added Linux support for MZ. This requires manual configuration, see the README for more information.

### Changed

- Updated CPM from 0.31.1 to [0.32.0](https://github.com/cpm-cmake/CPM.cmake/releases/tag/v0.32.0) ([#36](https://github.com/erri120/rpgmpacker/issues/36))
- Updated toml11 from 3.6.0 to [3.6.1](https://github.com/ToruNiina/toml11/releases/tag/v3.6.1) ([#37](https://github.com/erri120/rpgmpacker/issues/37))
- Updated spdlog 1.8.2 to [1.8.5](https://github.com/gabime/spdlog/releases/tag/v1.8.5) ([#35](https://github.com/erri120/rpgmpacker/issues/35))
- Updated ghc::filesystem 1.5.2 to [1.5.4](https://github.com/gulrak/filesystem/releases/tag/v1.5.4) ([#34](https://github.com/erri120/rpgmpacker/issues/34))

### Fixed

- Fixed multiple bugs that appeared during testing, most of them are just bugs that came from specific combinations.

## [1.5.4] - 2021-02-28

### Fixed

- Fixed file exclusion for MV.
- Fixed line break problem when updating System.json. The tool will now find the last bracket instead of assuming its position.
- Fixed System.json not being updated due to string comparison failing.

### Changed

- Tool will now exit when output folder can not be cleaned.
- Updated [`ghc::filesystem`](https://github.com/gulrak/filesystem) to [1.5.2](https://github.com/gulrak/filesystem/releases/tag/v1.5.2) ([#26](https://github.com/erri120/rpgmpacker/issues/26))

## [1.5.3] - 2021-02-27

### Fixed

- Fixed character encoding for MZ Effect parsing on Linux ([#25](https://github.com/erri120/rpgmpacker/issues/25))

## [1.5.2] - 2021-02-26

### Fixed

- Hotfixes `sv_enemies` folder still being ignored ([#24](https://github.com/erri120/rpgmpacker/issues/24))

## [1.5.1] - 2021-02-25

### Fixed

- Fixed Side-View Enemy Battler Images not being excluded properly ([#22](https://github.com/erri120/rpgmpacker/issues/22))

## [1.5.0] - 2021-02-11

### Added

- OSX export for MZ ([#15](https://github.com/erri120/rpgmpacker/issues/15))

### Fixed

- Fixed memory allocation issue ([#12](https://github.com/erri120/rpgmpacker/issues/12))

## [1.4.0] - 2021-02-09

### Added

- Added support for a TOML config file ([#13](https://github.com/erri120/rpgmpacker/issues/13))
- Skip copying save files ([#11](https://github.com/erri120/rpgmpacker/issues/11))

### Fixed

- Fixed atomic values not being initialized

## [1.3.0] - 2021-02-09

### Added

- Added MZ Effect parsing for better unused file exclusion ([#9](https://github.com/erri120/rpgmpacker/issues/9))

### Fixed

- Fixed error log always displaying "Actors.json" during JSON exception

## [1.2.1] - 2021-02-08

### Fixed

- Fixed missing folders, replaced error with a warning (see [#6](https://github.com/erri120/rpgmpacker/issues/6))
- Fixed Exclude Unused Files feature for MZ ([#7](https://github.com/erri120/rpgmpacker/issues/7))

## [1.2.0] - 2021-02-08

### Added

- "Exclude Unused Files" support ([#1](https://github.com/erri120/rpgmpacker/issues/1))

## [1.1.0] - 2021-02-05

### Added

- macOS support

### Fixed

- Compilation with MinGW

## [1.0.0] - 2021-02-03

### Added

- Everything. This is the first release of this application.

<!-- Links -->
[Keep a Changelog]: https://keepachangelog.com/
[Semantic Versioning]: https://semver.org/

<!-- Versions -->
[Unreleased]: https://github.com/erri120/rpgmpacker/compare/v2.0.1...HEAD
[Released]: https://github.com/erri120/rpgmpacker/releases/
[2.0.1]: https://github.com/erri120/rpgmpacker/compare/v2.0.0...v2.0.1
[2.0.0]: https://github.com/erri120/rpgmpacker/compare/v1.6.2...v2.0.0
[1.6.2]: https://github.com/erri120/rpgmpacker/compare/v1.6.1...v1.6.2
[1.6.1]: https://github.com/erri120/rpgmpacker/compare/v1.6.0...v1.6.1
[1.6.0]: https://github.com/erri120/rpgmpacker/compare/v1.5.4...v1.6.0
[1.5.4]: https://github.com/erri120/rpgmpacker/compare/v1.5.3...v1.5.4
[1.5.3]: https://github.com/erri120/rpgmpacker/compare/v1.5.2...v1.5.3
[1.5.2]: https://github.com/erri120/rpgmpacker/compare/v1.5.1...v1.5.2
[1.5.1]: https://github.com/erri120/rpgmpacker/compare/v1.5.0...v1.5.1
[1.5.0]: https://github.com/erri120/rpgmpacker/compare/v1.4.0...v1.5.0
[1.4.0]: https://github.com/erri120/rpgmpacker/compare/v1.3.0...v1.4.0
[1.3.0]: https://github.com/erri120/rpgmpacker/compare/v1.2.1...v1.3.0
[1.2.1]: https://github.com/erri120/rpgmpacker/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/erri120/rpgmpacker/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/erri120/rpgmpacker/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/erri120/rpgmpacker/releases/v1.0.0
