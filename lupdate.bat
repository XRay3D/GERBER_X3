Setlocal EnableDelayedExpansion

set LUPDATE="C:\Qt\5.15.2\msvc2019_64\bin\lupdate.exe"

set FILE=
set TS=
set CURRENT=%CD%\

cd GGEasy 
for /R %%S in (*.ts) do set TS=!TS! %%S
cd ..

call set TS=%%TS:!CURRENT!=%%
echo %TS% 

for %%P in (GGEasy GTE_Win plugins static_libs) do (
	cd %%P 
	for /R %%S in (*.cpp *.h *.ui) do set FILE=!FILE! %%S
	cd ..
)

call set FILE=%%FILE:!CURRENT!=%%
echo %FILE% 

%LUPDATE% %FILE% -ts %TS% 

REM pause lupdate qml.qrc filevalidator.cpp -ts myapp_en.ts myapp_fr.ts

pause

:FUNCTIONS
@REM FUNCTIONS AREA
GOTO:EOF
EXIT /B

:ReplaceText
::Replace Text In String
::USE:
:: CALL:ReplaceText "!OrginalText!" OldWordToReplace NewWordToUse  Result
::Example
::SET "MYTEXT=jump over the chair"
::  echo !MYTEXT!
::  call:ReplaceText "!MYTEXT!" chair table RESULT
::  echo !RESULT!
::
:: Remember to use the "! on the input text, but NOT on the Output text.
:: The Following is Wrong: "!MYTEXT!" !chair! !table! !RESULT!
:: ^^Because it has a ! around the chair table and RESULT
:: Remember to add quotes "" around the MYTEXT Variable when calling.
:: If you don't add quotes, it won't treat it as a single string
::
set "OrginalText=%~1"
set "OldWord=%~2"
set "NewWord=%~3"
call set OrginalText=%%OrginalText:!OldWord!=!NewWord!%%
SET %4=!OrginalText!
GOTO:EOF