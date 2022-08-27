
REM set Ms=x15 55 xx4*8 112.6 12 800 88.1 sss
REM set Ss1=xx4*8 112.6 12 800 88.1

REM call set Replaced=%%Ms:%Ss1%=%%

REM echo Ms:       %Ms%
REM echo Ss1:      %Ss1%
REM echo Replaced: %Replaced%

REM If NOT "%Ms%"=="%Replaced%" (
     REM echo Ms contains Ss1
REM ) else (
     REM echo Ms does not contain Ss1
REM )

REM pause

rem set PATH="%PATH%C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin"

set CLANG_FORMAT="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-format.exe"

for /R %%S in (*.cpp *.c *.hpp *.h) do (
	 %CLANG_FORMAT% -i --style="{BasedOnStyle: WebKit,AllowShortLambdasOnASingleLine: All,BreakBeforeTernaryOperators: 'false',Standard: Cpp11,BreakBeforeBraces: Custom,FixNamespaceComments: 'true',AlignConsecutiveMacros: 'true',AlignTrailingComments: 'true',Cpp11BracedListStyle: 'true',AlignArrayOfStructures: Right}" %%S
)
pause