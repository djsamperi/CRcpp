The files in this directory are patched for use with the Microsoft C++
compiler (cl.exe). Tested with Visual Studio Community Edition 2022.
Warnings from the compiler have been suppressed because there are
many. MSVC-specific edits can be found by searning for _MSC_VER.
There is also an edit/patch that forces use of the version of
Rcpp pointed to by R_LIBS (required under MacOS).

To link to R (compiled using GCC toolchain) an import library R.lib
must be generated from the DLL R.dll. After R.lib is created, the
CMake makefile takes care of the link step automatically. It also
detects the appropriate MSVC toolchain to use provided it is 
run inside a Visual Studio 2022 terminal window.

To create R.lib we need a tool pexports.exe that is part of the
MinGW GNU compiler collection for Windows. But this tool is
not included with Rtools or with the standard MinGW install.
It could be compiled from source (search for pexports-0.43.zip
on SourceForge), but to avoid this, first install MinGW 
from SourceForge into the default location c:\MinGW (only the
development tools are needed, not the compilers). Then
issue the command:

C:\MinGW\bin\mingw-get install mingw32-pexports

This creates C:\MinGW\bin\pexports.exe, and this is how it
should be invoked. In particular, do not add C:\MinGW\bin to
PATH as this can cause conflicts with tools shipped with Rtools.

With pexports.exe installed, R.lib can be created by opening
a Visual Studio 2022 terminal window, and issuing commands
like the following:

> cd "c:\Program Files\R\R-4.5.1\bin\x64"
> c:\MinGW\bin\pexports R.dll > Rdll.def
> lib /def:Rdll.def /out:R.lib /machine:x64








