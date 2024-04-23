@echo off
rem Checks that R and sh (UNIX tools) are present under Windows
set envproblem=0

where /q sh
if ERRORLEVEL 1 (
  set envproblem=1
  ECHO --
  ECHO -- PROBLEM: UNIX emulation environment is missing.
  ECHO -- To fix add this directory to PATH environment variable
  ECHO -- C:\Rtools42\usr\bin
  ECHO -- For access to g++ and related tools also add
  ECHO -- C:\Rtools42\x86_64-mingw32.static.posix\bin
  ECHO -- Precise path can vary depending on R version installed.
  ECHO -- Avoid mixing different versions of MSYS Unix emulation,
  ECHO -- for example, Rtools, Cygwin, Chocolatey, Haskell Tool Stack,
  ECHO -- and other frameworks included MSYS. Place desired
  ECHO -- version first in PATH, or remove undesired versions.
  ECHO --
  )
  
where /q R
if ERRORLEVEL 1 (
  set envproblem=1
  ECHO --
  ECHO -- PROBLEM: Cannot find R executable.
  ECHO -- This is normally fixed by adding to environment variable PATH
  ECHO -- C:\Program Files\R\R-4.2.2\bin\x64
  ECHO -- Adjust R version as needed.
  ECHO -- Note that only x64 builds are supported.
  ECHO --
  )

exit %envproblem%
