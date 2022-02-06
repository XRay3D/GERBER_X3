rem set PATH="%PATH%C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin"
for /R %%S in (*.cpp *.c *.hpp *.h) do (
	"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-format.exe" -i --style="{BasedOnStyle: WebKit, AlignConsecutiveMacros: 'true', AlignTrailingComments: 'true', BreakBeforeBraces: Custom, FixNamespaceComments: 'true'}" %%S
)
pause