@ECHO OFF
ECHO Packing Risen 3 (Windows 64-bit) string table...

CD /D "%~dp0"
lianzifu.exe ^
--read-ini "#G3:/ini/loc.ini" ^
--read-csv ^
--save-bin x64 6 "#X64:/data/compiled/localization/w_strings.bin" ^
--save-map "#G3:/%~n0.csv" ^
--exit 1> "%~dpn0.log" 2>&1

IF ERRORLEVEL 1 (
  TYPE "%~dpn0.log"
  PAUSE
)
