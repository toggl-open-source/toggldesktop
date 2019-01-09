# openssl
OpenSSL build scripts and binaries

#### Usage:
```
build.ps1 [-openssl_release 1.0.0 | 1.1.0]
          [-vs_version 150 | 140 | 120 | 110 | 100 | 90]
          [-config     release | debug | both]
          [-platform   Win32 | x64] # TODO WinCE/WCE
          [-library    shared | static | both]
```

#### Example:
```
powershell -ExecutionPolicy RemoteSigned -File build.ps1 -openssl_release 1.1.0g -vs_version 120 -config both -platform x64 -library both
```

The above command will build 64-bit release/debug and shared/static libraries.

The rest of this document applies to building using pre-built OpenSL binaries. If you are building from source, you can skip it.

#### Notes:

OpenSSL 1.0.x and 1.1.x use similar, but not entirely same build systems. Building OpenSSL <=1.0.x is deprecated and not actively maintained.
The script logic is still present, but it may be removed at any time.

OpenSSL >=1.1.0 binaries used by all POCO VS builds are built using VS 2013 (MSVC 120). While having a one set of prebuilt binaries for all
VS versions is convenient, there are some challenges related to static linking (/MT) from VS version != 2013.

These could not be solved using OpenSSL build systems (eg. `no-sse2` or `no-asm` options) and have been resolved in an unconventional way:

1) LNK2019: unresolved external symbol ___report_rangecheckfailure

Occurs in VS versions < 2013.
Solved using [this method](https://cubicspot.blogspot.com/2015/07/solving-unresolved-external-symbol.html).


2) LNK2019: unresolved external symbol __dtoul3

Occurs in VS versions < 2013, 32-bit only.
Solved using [this method](https://stackoverflow.com/a/19556596/205386).

Explanation:

The obj file containing the missing function is extracted from the VS 2012 msvcrt.lib, and a separate
library (libMissingPreVS2013CRT.lib) is created from it. The static-linked executables (/MT)compiled
with VS <2012 must link to this library (in addition to linking to libcrypto.lib and/or libssl.lib).

3) LNK2001: unresolved external symbol __iob_func

Occurs in VS versions >= 2015.
Solved using this method: https://stackoverflow.com/a/33830712/205386


Given the above, starting with POCO >=1.9.0 and >=2.0, using these binaries is transparent - all VS projects
are preconfigured to use these binaries out-of-the-box.
