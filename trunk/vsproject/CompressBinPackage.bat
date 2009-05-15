echo Please put dlls in bin\Release\iH3C\ first. Press any key when ready.
pause

mkdir bin\Release\iH3C
copy bin\Release\h3c_svr.exe bin\Release\iH3C\h3c_svr.exe
copy bin\Release\H3CWatcher.exe bin\Release\iH3C\H3CWatcher.exe
copy bin\Release\h3c_svr.exe bin\Release\iH3C\h3c_svr.exe
copy ..\h3c_svr\pw.data bin\Release\iH3C\pw.data
copy ..\README bin\Release\iH3C\readme.txt
copy ..\changes bin\Release\iH3C\changes.txt

del bin\Release\iH3C_bin.rar
cd bin\Release\
rar a iH3C_bin.rar iH3C\*.*

pause