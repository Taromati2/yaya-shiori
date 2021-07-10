del *.bak
mkdir tmp\

del /F /S /Q tmp\aya\*

mkdir tmp\aya\
mkdir tmp\aya\messagetxt\
copy /B /Y .\Release\aya.dll tmp\aya\aya.dll
copy /B /Y .\readme.txt tmp\aya\readme.txt
copy /B /Y .\readme-original.txt tmp\aya\readme-original.txt
copy /B /Y .\messagetxt\*.* tmp\aya\messagetxt\

del /F /S /Q tmp\aya.zip

chdir tmp\aya\
zip -r -9 -q ..\aya.zip *
chdir ..\..

