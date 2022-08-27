set CLANG_FORMAT="C:\Qt\Tools\QtCreator\bin\clang\bin\clang-format.exe"

for %%P in (GGEasy GTE_Win plugins static_libs) do (
	cd %%P 
	for /R %%S in (*.cpp *.c *.hpp *.h) do (
	    %CLANG_FORMAT% -i --style="{BasedOnStyle: WebKit,AllowShortLambdasOnASingleLine: All,BreakBeforeTernaryOperators: 'false',Standard: Cpp11,BreakBeforeBraces: Custom,FixNamespaceComments: 'true',AlignConsecutiveMacros: 'true',AlignTrailingComments: 'true',Cpp11BracedListStyle: 'true',AlignArrayOfStructures: Right}" %%S
	)
	cd ..
)
pause
