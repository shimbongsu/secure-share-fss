@echo off
:: $Id$
setlocal
:: Perform post-build steps
:: An example follows on the next two lines ...
:: xcopy /y ".\obj%BUILD_ALT_DIR%\i386\*.sys" "..\"
:: xcopy /y ".\obj%BUILD_ALT_DIR%\i386\*.pdb" "..\"

mkdir ..\build
xcopy /y ".\obj%BUILD_ALT_DIR%\i386\*.sys" "..\build\"
xcopy /y ".\obj%BUILD_ALT_DIR%\i386\*.pdb" "..\build\"
xcopy /y ".\gingko*.cmd" "..\build\"
xcopy /y ".\gingko*.inf" "..\build\"

endlocal