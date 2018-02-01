@ECHO OFF
ECHO Packing Risen 3 (Windows 32-bit) string table...

REM Risen 3 retail (without patch) has string table version 5 instead of 6

CD /D "%~dp0"
lianzifu.exe ^
--read-ini "#G3:/ini/loc.ini" ^
--read-csv ^
--save-bin pc 6 "#G3:/data/compiled/localization/w_strings.bin" ^
--save-map "#G3:/%~n0.csv" ^
--exit 1> "%~dpn0.log" 2>&1

IF ERRORLEVEL 1 (
  TYPE "%~dpn0.log"
  PAUSE
)
