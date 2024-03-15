set CLANG_FORMAT="C:\Qt\Tools\QtCreator\bin\clang\bin\clang-format.exe"
set CODE_STYLE=%CD%\code_style.txt

for %%P in (GGEasy GTE_Win plugins static_libs) do (
	cd %%P 
	for /R %%S in (*.cpp *.c *.hpp *.h) do (
	    %CLANG_FORMAT% -i --style="file:%CODE_STYLE%" %%S
	)
	cd ..
)
REM pause
