gcc PWDumpX.c -o PWDumpX.exe -lmpr -ladvapi32
gcc -c DumpSvc.c
gcc -c -DBUILD_DLL DumpExt.c
gcc -shared -o DumpExt.dll -Wl,--out-implib,libDumpExt.a DumpExt.o
gcc -o DumpSvc.exe DumpSvc.o -L./ -lDumpExt
