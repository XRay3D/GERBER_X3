
set Ms=x15 55 xx4*8 112.6 12 800 88.1 sss
set Ss1=xx4*8 112.6 12 800 88.1

call set Replaced=%%Ms:%Ss1%=%%

echo Ms:       %Ms%
echo Ss1:      %Ss1%
echo Replaced: %Replaced%

If NOT "%Ms%"=="%Replaced%" (
     echo Ms contains Ss1
) else (
     echo Ms does not contain Ss1
)

pause

rem set PATH="%PATH%C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin"
for /R %%S in (*.cpp *.c *.hpp *.h) do (
	rem if %%S goto :continue
	"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-format.exe" -i --style="{BasedOnStyle: WebKit, AlignConsecutiveMacros: 'true', AlignTrailingComments: 'true', BreakBeforeBraces: Custom, FixNamespaceComments: 'true'}" %%S
rem :continue
)
pause