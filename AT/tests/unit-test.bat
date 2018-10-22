echo off
title Unit-test script
for %%a in (".\*.bin") do (
	echo %%a:
	call ..\x64\Debug\AT.exe %%a
	)
pause
