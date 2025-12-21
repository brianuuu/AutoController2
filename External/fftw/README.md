#FFTW installation
Download FFTW: https://www.fftw.org/install/windows.html
Make sure both version matches.

Extract fftw3.h and put it inside "include" folder
Extract "libfftw3f-3.dll" and "libfftw3f-3.def" to this folder

Generate .lib files following the instructions here: https://wiki.videolan.org/GenerateLibFromDll

The result should look something like this:

fftw/
├── include/
│   ├── fftw3.h
├── libfftw3f-3.dll
├── libfftw3f-3.lib
├── README.md
