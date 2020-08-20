:: Windows Python Builder
@echo off
:: Get the python version (36, 37, etc.) and optional bits (64)
set python_version=%1
set python_bits=-%2

:: If the bits were the default value, assume using 32 bit python
IF %python_bits%==- (
    set python_dir=C:\\Python%python_version%
    set venv_name=venv_%python_version%

    set vc_toolchain=x86
) ELSE (
    set python_dir=C:\\Python%python_version%%python_bits%
    :: We trim the - for the venv name.
    set venv_name=venv_%python_version%.%python_bits:~1%

    set vc_toolchain=x86_x64
)

set vcvarsall="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
call %vcvarsall% %vc_toolchain%

set PATH=C:\tools\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin;%PATH%
set PATH=C:\gcc-arm-none-eabi-9-2019-q4-major-win32\bin;%PATH%
set PATH="C:\Program Files\Git\usr\bin";%PATH%

call scons || GOTO QUIT

C:\Python36\python.exe -m virtualenv -p %python_dir%\\python.exe %venv_name% || GOTO QUIT
call %venv_name%\Scripts\activate || GOTO QUIT

pip install . pytest || GOTO QUIT
pytest --verbose --color=yes test || GOTO QUIT
python setup.py bdist_wheel || GOTO QUIT

call deactivate

@echo on

:END
EXIT /B 0

:QUIT
EXIT /B 1
