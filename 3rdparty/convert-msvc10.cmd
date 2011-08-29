@echo off
REM ---------------------------------------------------------------
REM -- Visual Studio 2010 conversion script
REM ---------------------------------------------------------------
REM
REM This script converts the existing 3rd party libs projects to the
REM msbuild format for use with Visual Studio 2010
REM
REM Requirements: Visual Studio 2010 installed and in the path
REM
REM ---------------------------------------------------------------

REM First, copy and rename the project files to be used for conversion
copy /Y ".\WinSparkleDeps.sln"                     ".\WinSparkleDeps-2010.sln"
copy /Y ".\expat\lib\expat_static.vcproj"          ".\expat\lib\expat_static-2010.vcproj"
copy /Y ".\wxWidgets\build\msw\wx_vc9_base.vcproj" ".\wxWidgets\build\msw\wx_vc10_base.vcproj"
copy /Y ".\wxWidgets\build\msw\wx_vc9_core.vcproj" ".\wxWidgets\build\msw\wx_vc10_core.vcproj"

REM Run the script to replace references to projects in solution
@call cscript "convert-msvc10.vbs" "reference"
if not %errorlevel% == 0 goto error_script

REM Call devenv with /upgrade option to upgrade project to latest version
devenv /Upgrade ".\WinSparkleDeps-2010.sln"

REM Run the script to fix errors in wxWidgets projects
@call cscript "convert-msvc10.vbs" "fix"
if not %errorlevel% == 0 goto error_script

REM Remove unused files and conversion logs
del ".\UpgradeLog.XML"
del ".\expat\lib\expat_static-2010.vcproj"
del ".\wxWidgets\build\msw\wx_vc10_base.vcproj"
del ".\wxWidgets\build\msw\wx_vc10_core.vcproj"
rmdir /S /Q Backup
rmdir /S /Q _UpgradeReport_Files

goto done

:error_script:
echo An error occured while running the conversion script!

:done
exit /B0
