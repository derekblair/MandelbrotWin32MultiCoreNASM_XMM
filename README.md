# MandelbrotWin32MultiCoreNASMXMM
A minimalistic Win32 implementation of a Mandelbrot Set renderer that utilizes multiple threads and SIMD xmm registers the the NASM assembler. 

## Build Instructions

1. Download and install [Dev C++ 5](https://sourceforge.net/projects/orwelldevcpp/)
2. Download and install [Nasm Win32](http://sourceforge.net/projects/nasm/files/Win32%20binaries/2.07/nasm-2.07-win32.zip/download)
3. After ensuring the nasm executable is in the same folder as the `mandelbrot.nasm` file (the root folder), run the following command from comand prompt in the root folder: `nasm -f win32 mandelbrot.nasm -o mandelbrot.obj`
4. Open the Dev C++ 5 project file, build and run.
