rem set PATH="%PATH%C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin"
for /R %%S in (*.cpp *.c *.hpp *.h) do (
	rem echo %PATH%
	rem echo %%S
	rem -style="{key: value, ...}
	"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-format.exe" -i --style=WebKit %%S
)
pause