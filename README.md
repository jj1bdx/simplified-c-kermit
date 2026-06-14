# Simplified C-Kermit

* Based on C-Kermit 10.0 Beta.12

## Objectives

* Remove unusable code pieces
* Remove unused code
* Improve readability for the supported systems

### Supported systems

* macOS
* Ubuntu
* Raspberry Pi OS

## Achievements

* Removed dead code (old UNIXes)
* Removed code for Android, Windows and VMS
* Removed SSL/TLS and Kerberos support
  * Use SSH for interactive Kermit sessions
* Removed unusable/dead makefile targets
* Removed non-ANSI / K&R code
* Applied clang-format in a limited basis
  * Disabled `InsertBraces` - break the code
  * Disabled `SortIncludes` - break the macro definition code

## License

Simplified 3-Clause BSD License

## Reference

See <https://www.kermitproject.org/ckdaily.html#changelog> for the details.

## AI Usage

* The entire codebase is thoroughly edited with Claude Code.

