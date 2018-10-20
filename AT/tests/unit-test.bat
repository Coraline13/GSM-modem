echo off
title Unit-test script
for %%a in (".\*.txt") do (
	echo %%a:
	call ..\Debug\at.exe %%a
	)
pause
