@echo off

rem 全局变量
rem dstProgramName 是替换的文件， srcProgramName 是被替换掉的文件
set dstProgramPath=%1
set dstProgramName=%2
set srcProgramPath=%3
set srcProgramName=%4

echo "%dstProgramPath%\%dstProgramName%"
echo "%srcProgramPath%\%srcProgramName%"

rem timeout /T 3 /NOBREAK
choice /t 1 /d y /n >nul


rem :killProc
rem taskkill /F /IM %srcProgramName%

del /q "%srcProgramName%"

move /y %dstProgramPath%\%dstProgramName% %srcProgramPath%\%srcProgramName%

rem 运行 queciot_uart_tool.exe
rem echo start %srcProgramName%
start %srcProgramPath%\%srcProgramName% update

rem 删除批处理自身
echo remove bat file
del /f /q %0