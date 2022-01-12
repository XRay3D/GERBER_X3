For /R %%S IN (*.cpp *.h) do (
  rem Echo %%S
  rem -style="{key: value, ...}
  "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-format.exe" -i --style=WebKit %%S
)
pause