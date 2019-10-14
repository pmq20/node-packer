# Libautoupdate Changelog

## v0.2.0

- Auto-update shall only run once in every 24 hours with help of the file `~/.libautoupdate`
- add argument `force` to `autoupdate()` in order to force an auto-update check
- add CI to test `autoupdate()`
- fix failures to replace itself when TMPDIR and current file is not on the same volume
  - https://github.com/pmq20/libautoupdate/issues/1

## v0.1.0

Initial release.
