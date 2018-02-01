@ECHO OFF
ECHO.
ECHO This will override the CSVs in ./data/raw/strings/.
ECHO Press [Ctrl+C] to cancel this script.
ECHO.
PAUSE
ECHO.
ECHO Unpacking Risen 3 string table...

CD /D "%~dp0"
lianzifu.exe ^
--read-map "#G3:/%~n0.csv" ^
--read-ini "#G3:/ini/loc.ini" ^
--read-bin "#G3:/data/compiled/localization/w_strings.bin" ^
--save-csv ^
--exit 1> "%~dpn0.log" 2>&1

IF ERRORLEVEL 1 (
  TYPE "%~dpn0.log"
  PAUSE
)
