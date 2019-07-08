Building OpenSSL from sources on Windows
=================

Open `third_party\poco\openssl\buildall.cmd`. Tweak the config if needed.

Run `buildall.cmd` from Developer Command Prompt for VS 2019.

The resulting binaries are in `VS_140` folder if `buildall.cmd` config was not changed. Replace the contents of `build\win32` and/or `build\win64` with the resulting binaries from `VS_140` folder.