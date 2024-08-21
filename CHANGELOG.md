# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Build of packages for Fedora 34 platform.

### Changed

### Fixed

## [1.1.1] - 2022-04-11

### Fixed

- Don't terminate the memcached expire fiber in read-only mode.

## [1.1.0] - 2022-12-07

### Added

- Check on tarantool box initialization before init (now it returns a nice
  error). (Issue: #76 PR: #78).
- Possibility to pass port to `memcached.create()` as a number.
  (Issue: #107 PR: #108).

### Changed

- Updated an upper value for `readahead` parameter. (Issue: #103 PR: #109).
- Switched to using the module's own slab arena. For information
  `memcached.slab.info()` can be used. (Issue: #96 PR: #102).
- Switched to using bundled msgpuck and small libraries. (PR: #113).

### Fixed

- Stuck on processing GET request. (Issue: #73 PR: #94).
- Segfault on tarantool debug build. (Issue: #59 PR: #111).
