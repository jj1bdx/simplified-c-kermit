# Simplified C-Kermit

* C-Kermit 10.0.416-jj1bdx-simplified Dev.1
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

See the following summary documents:

* [doc/BASE_20260616.md](doc/BASE_20260616.md)
* [doc/BASE_20260709.md](doc/BASE_20260709.md)
* [doc/BASE_20260711.md](doc/BASE_20260711.md)

### For removing possible network vulnerabilities

* For Linux: use `make linux-nonet` or `make linux-notcp` for removing unwanted network support code from the executable.
* For macOS: use `make macos-nonet` or `make macos-notcp` for removing unwanted network support code from the executable.

## License

Simplified 3-Clause BSD License

## Reference

* See <https://www.kermitproject.org/ckdaily.html#changelog> for the details of the original C-Kermit Project.
* My C-Kermit test repository: <https://github.com/jj1bdx/c-kermit-alpha/>

## AI Usage

* The entire codebase is thoroughly edited with Claude Code.

