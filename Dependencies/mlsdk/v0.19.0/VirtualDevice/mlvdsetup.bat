@echo off
set _ROOT=%~dp0
set ML_VDS_DATA=%_ROOT%data

@rem get directory of MLSDK based on previous envsetup.bat invocation
@rem need to expose MLSDK for ML Remote frontend
for /f "delims=" %%M in ('where mabu 2^>nul') do (
	set MLSDK=%%~dpM
	goto :found
)
if ERRORLEVEL 1 (
    call "%_ROOT%\..\envsetup.bat"
    if ERRORLEVEL 1 (
        echo Please run envsetup.bat from your MLSDK before running this script.
    )
)

:found
@rem locate CLI binaries, and, favor ML Remote DLL overrides over MLSDK stubs
PATH=%_ROOT%bin;%_ROOT%lib;%MLSDK%lib\win64;%PATH%
