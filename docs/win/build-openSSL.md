Building OpenSSL from sources on Windows
=================

[Install ActivePerl](http://www.activestate.com/activeperl/downloads).

cd to the project folder, then

```
cd third_party\openssl
perl Configure VC-WIN32
nmake
```

Instead of do_nasm (use NASM) you can also use do_ms (no asm at all), or do_masm (use MASM). 
NASM can be downloaded here: http://www.nasm.us/pub/nasm/releasebuilds/?C=M;O=D