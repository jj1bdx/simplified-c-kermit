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
* Removed unusable files
* Streamlined include files 
* Applied clang-format with the flags InsertBraces and SortIncludes enabled
* Performed basic vulnerability check and the fixes
* Remove FTP support (equivalent to `-DNOFTP`)

## License

Simplified 3-Clause BSD License

## Reference

* See <https://www.kermitproject.org/ckdaily.html#changelog> for the details of the original C-Kermit Project.
* My C-Kermit test repository: <https://github.com/jj1bdx/c-kermit-alpha/>

## AI Usage

* The entire codebase is thoroughly edited with Claude Code.

