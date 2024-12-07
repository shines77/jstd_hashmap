@rem Script to build jstd with MSVC.
@rem Copyright (C) 2020 XiongHui Guo. See Copyright Notice in /jstd/all.h
@rem
@rem Either open a "Visual Studio .NET Command Prompt"
@rem (Note that the Express Edition does not contain an x64 compiler)
@rem -or-
@rem Open a "Windows SDK Command Shell" and set the compiler environment:
@rem     setenv /release /x86
@rem   -or-
@rem     setenv /release /x64
@rem
@rem Then cd to this directory and run this script.

@if not defined INCLUDE goto :FAIL

@setlocal
@set MSVC_ARCH=%Platform% %CommandPromptType% %PreferredToolArchitecture%
@set CPUID_COMPILE=cl /nologo /c /O2 /W3 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_STDIO_INLINE=__declspec(dllexport)__inline
@set CPUID_LINK=link /nologo
@set CPUID_MT=mt /nologo
@set CPUID_LIB=lib /nologo /nodefaultlib
@set CPUID_DLLNAME=get_arch.dll
@set CPUID_LIBNAME=get_arch.lib

@set MYOBJS=
@set MYSRCS=

@set CPUIDLIB_A=get_arch.lib
@set CPUIDLIB_C=get_arch.c
@set CPUIDLIB_O=get_arch.obj

@set CORE_C=
@set CORE_O=

@set LIB_C=
@set LIB_O=

@set BASE_C=%CORE_C% %LIB_C% %MYSRCS%
@set BASE_O=%CORE_O% %LIB_O% %MYOBJS%

@set CPUID_T=get_arch.exe
@set CPUID_A=get_arch.dll
@set CPUID_C=get_arch.c
@set CPUID_O=get_arch.obj

@set ALL_T=%CPUID_A% %CPUID_T% %JSTDC_T%
@set ALL_A=%CPUID_A%
@set ALL_X=%BASE_C% %CPUID_C% %JSTDC_C%
@set ALL_O=%BASE_O% %CPUID_O% %JSTDC_O%

@if "%1" neq "debug" goto :NODEBUG
@shift
@set CPUID_COMPILE=%CPUID_COMPILE% /Zi
@set CPUID_LINK=%CPUID_LINK% /debug

:NODEBUG
%CPUID_COMPILE% get_arch.c
@if errorlevel 1 goto :BAD
%CPUID_LINK% /out:get_arch.exe get_arch.obj
@if errorlevel 1 goto :BAD
if exist get_arch.exe.manifest^
  %CPUID_MT% -manifest get_arch.exe.manifest -outputresource:get_arch.exe

@echo.
@echo === Successfully built JStd - get_arch.exe for Windows/%MSVC_ARCH% ===

@goto :END
:BAD
@echo.
@echo *******************************************************
@echo *** Build FAILED -- Please check the error messages ***
@echo *******************************************************
@goto :END
:FAIL
@echo You must open a "Visual Studio .NET Command Prompt" to run this script
:END
