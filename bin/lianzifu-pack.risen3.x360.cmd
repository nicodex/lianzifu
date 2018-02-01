@ECHO OFF
ECHO Packing Risen 3 (Xbox 360) string table...

CD /D "%~dp0"
lianzifu.exe ^
--read-ini "#G3:/ini/loc.ini" ^
--read-csv ^
--save-bin x360 5 "#X360:/data/compiled/localization/x_strings.bin" ^
--save-map "#G3:/%~n0.csv" ^
--exit 1> "%~dpn0.log" 2>&1

IF ERRORLEVEL 1 (
  TYPE "%~dpn0.log"
  PAUSE
)
