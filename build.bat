@ECHO OFF
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\VsDevCmd.bat"
cd bin

set vst=../../../src/vstsdk2.4/
set vst2x=%vst%public.sdk/source/vst2.x/
set vstdef=%vst%public.sdk/samples/vst2.x/win/vstplug.def
set vst_c=/I"%vst%"
set vst_ld=/DEF:"%vstdef%"
set vstsrc=%vst2x%audioeffect.cpp %vst2x%audioeffectx.cpp %vst2x%vstplugmain.cpp

set wall=/Wall /wd4100 /wd4668 /wd4820 /wd4514 /wd4365 /wd4711 /wd4996 /wd4127
REM the warning disabled below is function-not-inlined
set common_c=/LD /I"../" /I"../include/" %wall% /wd4710
set release_c=%common_c% /O2 /Oy- /GL /EHsc /fp:fast /analyze- /nologo
set release_ld=

call:compile delay_test
call:compile eq
call:compile eq_const
call:compile eq_const_T420
REM call:compile eq_const_T420_svf
call:compile level
call:compile mugi4
call:compile noise
call:compile tube
goto:eof

:compile
set crap=%~1
cscript ..\util\generate.vbs %crap% vst
cscript ..\util\generate.vbs %crap% ladspa
cl %release_c% %vst_c% ../crap/vst/%crap%.cpp %vstsrc% /link %release_ld% %vst_ld% /OUT:"vst/crap_%crap%.dll"
cl %release_c% ../crap/ladspa/%crap%.cpp /link %release_ld% /OUT:"ladspa/crap_%crap%.dll"
goto:eof
