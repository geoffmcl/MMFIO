@setlocal

@set TMPPRJ=MMFIO
@set TMPLOG=bldlog-1.txt
@set CONTONERR=0
@set BUILD_RELDBG=0
@set TMPSRC=..
@REM ############################################
@REM NOTE: MSVC 10 INSTALL LOCATION
@REM Adjust to suit your environment
@REM ##########################################
@set GENERATOR=Visual Studio 10 Win64
@set VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio 10.0
@set VC_BAT=%VS_PATH%\VC\vcvarsall.bat
@if NOT EXIST "%VS_PATH%" goto NOVS
@if NOT EXIST "%VC_BAT%" goto NOBAT
@set BUILD_BITS=%PROCESSOR_ARCHITECTURE%
@IF /i %BUILD_BITS% EQU x86_amd64 (
    @set "RDPARTY_ARCH=x64"
    @set "RDPARTY_DIR=software.x64"
) ELSE (
    @IF /i %BUILD_BITS% EQU amd64 (
        @set "RDPARTY_ARCH=x64"
        @set "RDPARTY_DIR=software.x64"
    ) ELSE (
        @echo Appears system is NOT 'x86_amd64', nor 'amd64'
        @echo Can NOT build the 64-bit version! Aborting
        @exit /b 1
    )
)

@ECHO Setting environment - CALL "%VC_BAT%" %BUILD_BITS%
@CALL "%VC_BAT%" %BUILD_BITS%
@if ERRORLEVEL 1 goto NOSETUP

@REM set TMPINST=C:\FG\17\%RDPARTY_DIR%
@set TMPOPTS=
@REM set TMPOPTS=%TMPOPTS% -DCMAKE_INSTALL_PREFIX:PATH=%TMPINST%
@set TMPOPTS=%TMPOPTS% -G "%GENERATOR%"

@REM To help find Qt4
@REM Try adding path to qmake.exe to PATH
@REM set PATH=C:\QtSDK\Desktop\Qt\4.8.0\msvc2010\bin;%PATH%
@REM That WORKED ;=/
@REM Try adding QTDIR to environment
@REM set QTDIR=C:\QtSDK\Desktop\Qt\4.8.0\msvc2010\bin
@REM That ALSO works - now which to make PERMANENT?
@REM Ok, decide to add it to PATH
@REM And OK, that WORKS FINE
@REM 20131125 - started to FAIL again???? Try
@REM set Qt4_DIR=C:\QtSDK
@REM set Qt4_DIR=C:\QtSDK\Desktop\Qt\4.8.0\msvc2010\bin
@REM set Qt4_DIR=C:\Qt\4.8.6-x86\bin
@REM set Qt4_DIR=C:\Qt\4.8.6\bin
@REM call setupqt64
@REM set QTDIR=%Qt4_DIR%
@REM set QTDIR=C:\QtSDK\Desktop\Qt\4.8.0\msvc2010

@call chkmsvc %TMPPRJ%

@echo Built project %TMPORJ%... all ouput to %TMPLOG%

@set CMOPTS=%TMPOPTS%
@echo Commence build %DATE% at %TIME% > %TMPLOG%

@echo Doing: 'cmake %TMPSRC% %CMOPTS%
@echo Doing: 'cmake %TMPSRC% %CMOPTS% >> %TMPLOG%
@cmake %TMPSRC% %CMOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

@echo Doing: 'cmake --build . --config Debug'
@echo Doing: 'cmake --build . --config Debug' >> %TMPLOG%
@cmake --build . --config Debug >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR2

:DNDEBBLD

@if "%BUILD_RELDBG%x" == "0x" goto DNRELDBG
@echo Doing: 'cmake --build . --config RelWithDebInfo'
@echo Doing: 'cmake --build . --config RelWithDebInfo' >> %TMPLOG%
@cmake --build . --config RelWithDebInfo >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3

:DNRELDBG
@echo Doing: 'cmake --build . --config Release'
@echo Doing: 'cmake --build . --config Release' >> %TMPLOG%
@cmake --build . --config Release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR4

:DNRELBLD

@echo.
@echo Appears successful...
@echo.
@echo No install of this %TMPPRJ% at this time...
@echo.
@goto END

:ERR1
@echo FATAL ERROR: cmake configuration/generation FAILED
@echo FATAL ERROR: cmake configuration/generation FAILED >> %TMPLOG%
@goto ISERR

:ERR2
@echo ERROR: cmake build Debug
@echo ERROR: cmake build Debug >> %TMPLOG%
@if %CONTONERR% EQU 1 goto DNDEBBLD
@goto ISERR

:ERR3
@echo ERROR: cmake build RelWithDebInfo
@echo ERROR: cmake build RelWithDebInfo >> %TMPLOG%
@if %CONTONERR% EQU 1 goto DNRELBLD
@goto ISERR

:ERR4
@echo ERROR: cmake build Release
@echo ERROR: cmake build Release >> %TMPLOG%
@goto ISERR

:NOVS
@echo ERROR: Can NOT locate path %VS_PATH%! *** FIX ME *** to where MSVC is installed
@goto ISERR

:NOBAT
@echo ERROR: Can NOT locate batch %VC_BAT%! *** FIX ME *** to where MSVC is installed
@goto ISERR

:NOSETUP
@echo ERROR: Running the %VC_BAT% caused an ERROR! *** FIX ME ***
@goto ISERR

:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
