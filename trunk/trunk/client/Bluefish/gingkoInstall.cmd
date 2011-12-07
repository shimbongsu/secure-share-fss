@REM
@REM Runs the DefaultInstall section of gingkofilter.inf
@REM

@echo off

@REM rundll32.exe setupapi,InstallHinfSection DefaultInstall 132 .\gingkofilter.inf


bootstrap -Install

pause

bootstrap 
