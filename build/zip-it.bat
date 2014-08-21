@setlocal
@set TMPVER=04
@set TMPZIP=C:\GTools\ConApps\MMFIO_CP-%TMPVER%.zip
@set TMPA=-a
@if EXIST %TMPZIP% (
@echo This is an UPDATE of %TMPZIP%!
@set TMPA=-u
) else (
@echo This is a NEW zip %TMPZIP%
)
@echo *** CONTINUE with ZIP ***
@Pause

cd ..\..

call zip8 %TMPA% -r -P -o %TMPZIP% MMFIO_CP\*

@echo Maybe copy2tmp %TMPZIP%...

