@setlocal
@set TMPEXE=Release\MMFIO32.exe
@if NOT EXIST %TMPEXE% goto NOEXE
@set TMPCMD=
:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPCMD=%TMPCMD% %1
@shift
@goto RPT
:GOTCMD

%TMPEXE% %TMPCMD%

@goto END

:NOEXE
@echo Error: Can NOT locate %TMPEXE%! Has it been built?
@goto END

:END

