@ECHO OFF

CD /D "%~dp0"
FOR %%I IN (data\compiled\localization\*strings*.bin) DO (
  ECHO %%~nxI
  lianzifu.exe --read-bin "data\compiled\localization\%%~nxI" --exit 1> "%~dpn0_%%~nI.log" 2>&1
)
