@echo off

call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set PATH=d:\programming\github\compiler\cmds;%PATH%

cd code
call 4coder
call vs
cls