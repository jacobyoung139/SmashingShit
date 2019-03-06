@echo off
@rem need to create/use a workspace in the same place each time
cd /d "%~dp0\bin\UIFrontend"
"%~dp0\bin\UIFrontend\MLRemote" %*
