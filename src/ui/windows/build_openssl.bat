call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
cd ..\..\..\third_party\openssl
perl Configure VC-WIN32
call ms\do_ms.bat
nmake -f ms\ntdll.mak
pause
