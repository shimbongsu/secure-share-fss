@REM
@REM Runs the DefaultUninstall section of gingkofilter.inf
@REM

@echo off

rundll32.exe setupapi,InstallHinfSection DefaultUninstall 132 .\gingkofilter.inf

