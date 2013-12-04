if not exist E:\DEV\WinDDK\Debuggers\windbg.exe goto next
start E:\DEV\WinDDK\Debuggers\windbg -b -k com:pipe,port=\\.\pipe\com_1,resets=0
exit
:next
start "" "C:\Program Files\Debugging Tools for Windows (x86)\windbg" -b -k com:pipe,port=\\.\pipe\com_1,resets=0
