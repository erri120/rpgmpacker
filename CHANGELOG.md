# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog][Keep a Changelog] and this project adheres to [Semantic Versioning][Semantic Versioning].

## [Unreleased]

### Added

- OSX export for MZ ([#15](https://github.com/erri120/rpgmpacker/issues/15))

### Fixed

- Fixed memory allocation issue ([#12](https://github.com/erri120/rpgmpacker/issues/12))

## [Released]

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
[Unreleased]: https://github.com/erri120/rpgmpacker/compare/v1.4.0...HEAD
[Released]: https://github.com/erri120/rpgmpacker/releases/
[1.4.0]: https://github.com/erri120/rpgmpacker/compare/v1.3.0...v1.4.0
[1.3.0]: https://github.com/erri120/rpgmpacker/compare/v1.2.1...v1.3.0
[1.2.1]: https://github.com/erri120/rpgmpacker/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/erri120/rpgmpacker/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/erri120/rpgmpacker/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/erri120/rpgmpacker/releases/v1.0.0
