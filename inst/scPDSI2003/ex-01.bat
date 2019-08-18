@echo off
rem ## 2. Old version, 2003, compiled with rtools
rem ./scpdsi -idata/example -odata potentials 
rem scpdsi -idata/example -odata potentials 

rem make >> log 2>&1
rem Set cal_year_start and cal_year_end
scpdsi.exe -idata/example -odata potentials -cs 1961 -ce 1990
