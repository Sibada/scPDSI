# $previous = Get-ChildItem -Recurse -path C:\Users\kon055\Desktop\scPDSI\data
# $new = Get-ChildItem -Recurse -path F:\Github\R_packages\scPDSI\data
# Compare-Object -ReferenceObject $previous -DifferenceObject $new

## 1. New version, Dong 2018
# Release\scPDSI.exe -idata\example -odata potentials & diff -r data ..\scPDSI-org\data > diff.txt & subl diff.txt

