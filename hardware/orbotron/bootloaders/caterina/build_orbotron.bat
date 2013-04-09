REM from the freetronics post at http://forum.freetronics.com/viewtopic.php?t=663
set BASEDIR=c:\bin\arduino-1.0.3\hardware
set DIRAVRUTIL=%BASEDIR%\tools\avr\utils\bin
set DIRAVRBIN=%BASEDIR%\tools\avr\bin
set DIRAVRAVR=%BASEDIR%\tools\avr\avr\bin
set DIRLIBEXEC=%BASEDIR%\tools\avr\libexec\gcc\avr\4.3.2
set OLDPATH=%PATH%
@path %DIRAVRUTIL%;%DIRAVRBIN%;%DIRAVRAVR%;%DIRLIBEXEC%;%PATH%
cd %BASEDIR%\orbotron\bootloaders\caterina
%DIRAVRUTIL%\make.exe clean
%DIRAVRUTIL%\make.exe all VID=0x1d50 PID=0x6067
copy Caterina.hex Caterina-Orbotron.hex
%DIRAVRUTIL%\make.exe clean
@path %OLDPATH%