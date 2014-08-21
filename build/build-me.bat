@setlocal
@set TMPPRJ=MMFIO
@set TMPVER=
@set TMPBGN=%TIME%
@set TMPLOG=bldlog-1.txt

@set TMPSRC=..

@set TMPOPTS=
@REM set TMPOPTS=-DCMAKE_INSTALL_PREFIX=F:\Projects\software
@REM Change order - software is GDAL 1.11...
@REM set TMPOPTS=%TMPOPTS% -DCMAKE_PREFIX_PATH:PATH=F:\Projects\software;F:\FG\18\3rdParty;C:\FG\18\3rdParty;X:\3rdParty

:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD

@if NOT EXIST %TMPSRC%\nul goto NOSRC
@if NOT EXIST %TMPSRC%\CMakeLists.txt goto NOCM

@call chkmsvc %TMPPRJ%

@echo Bgn %TMPPRJ% %DATE% %TIME% to %TMPLOG%
@echo Bgn %TMPPRJ% %DATE% %TIME% > %TMPLOG%

@if EXIST build-cmake.bat (
@call build-cmake > %TMPLOG%
@if ERRORLEVEL 1 goto BCMERR
)

@echo Doing: 'cmake %TMPSRC% %TMPOPTS%'
@echo Doing: 'cmake %TMPSRC% %TMPOPTS%' >> %TMPLOG%
@cmake %TMPSRC% %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

@echo Doing: 'cmake --build . --config Debug'
@echo Doing: 'cmake --build . --config Debug' >> %TMPLOG%
@cmake --build . --config Debug >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR2

@echo Doing: 'cmake --build . --config Release'
@echo Doing: 'cmake --build . --config Release' >> %TMPLOG%
@cmake --build . --config Release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3

@call elapsed %TMPBGN%
@echo Appears successful...
@echo.
@echo No install of this project...
@goto END

@echo Continue with INSTALL?
@echo *** CONTINUE? *** Only Ctrl+c aborts...
@pause

@REM echo Doing 'cmake --build . --config Debug --target INSTALL'
@REM echo Doing 'cmake --build . --config Debug --target INSTALL' >> %TMPLOG%
@REM cmake --build . --config Debug --target INSTALL >> %TMPLOG% 2>&1
@REM if ERRORLEVEL 1 goto ERR4

@echo Doing 'cmake --build . --config Release --target INSTALL'
@echo Doing 'cmake --build . --config Release --target INSTALL' >> %TMPLOG%
@cmake --build . --config Release --target INSTALL >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR5

@fa4 " -- " %TMPLOG%

@call elapsed %TMPBGN%
@echo All done...

@goto END

:ERR1
@echo CMake config/gen ERROR
@goto ISERR

:ERR2
@echo build debug ERROR
@goto ISERR

:ERR3
@echo build release ERROR
@goto ISERR

:ERR4
@echo install debug ERROR
@goto ISERR

:ERR5
@echo install release ERROR
@goto ISERR

:NOSRC
@echo Error: Can NOT locate source %TMPSRC%! *** FIX ME ***
@goto ISERR

:NOCM
@echo Error: Can NOT locate source %TMPSRC%\CMakeLists.txt! *** FIX ME ***
@goto ISERR

:BCMERR
@echo Error updating the CMakeLists.txt...
@goto ISERR

:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
