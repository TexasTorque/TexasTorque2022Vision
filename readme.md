# TexasTorque2022Vision

Compilation of computer vision code for external
coprocessors on the Texas Torque 2022 robot. This
does *not* include RoboRio interfacing code *or*
limelight, as it doesnt have offboard source code.

*Note* this will be merged as a subcomponent of
TexasTorque2022 monorepo, but is currently not as
it was made before the competition code was started.

## Configure Editor

This project is build with a Makefile utalizing the
[WPILib Raspbian Toolchain](https://github.com/wpilibsuite/raspbian-toolchain/releases)
added to your `$PATH`. The project utalizes the C17
and C++17 standards, and is built with the toolchain's
version of GCC 8 (`arm-raspbian10-linux-gnueabihf-gcc`
and `arm-raspbian10-linux-gnueabihf-g++`, which shoul
be in your `$PATH`). All of the headers to include
and static libraries to be linked are bundled in the
`./include` and `./lib` directories respectively.

To supress the include path errors, make sure your
`.vscode/c_cpp_properties.json` is structured for
your system and includes the `"includePath"` entry
like below.

```json
{
    "configurations": [
        {
            "name": "Mac",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/include"
            ],
            "defines": [],
            "macFrameworkPath": [],
            "compilerPath": "/usr/local/bin/gcc-11",
            "cStandard": "gnu17",
            "cppStandard": "gnu++17",
            "intelliSenseMode": "macos-gcc-x64"
        }
    ],
    "version": 4
}
```

I am assuming `./make.exe` is a portable GNU Make
binary for Windows, but I have yet to test it.

This project uses Clang-Format for, you guessed it,
code formatting. Clang-Format has a VSCode plugin
that I perfer to the command line utility, and it
should download all software automatically. Dont
try and format outside of `./src` because it will
run forever and waste your time.

## Cross-Compile

Run `make` to run the Makefile. The Makefile builds
all `.c`, `.cc`, (perfered C++ extension) and `.cpp`
files in `./src`.

The resulting binary at `./bin/camera-binary` is the
binary to upload to WPILibPI. *Do not try and run this
locally*, it is build for the ARM instruction set with
Raspberry-Pi system libraries, not x86 or Apple ARM.

To clean up the build generated `.o` files, run `make clean`.

## Subprojects

### Scripts

`./scripts` is a directory that containts Python scripts
used to test the functionality that will later be implemented
into the C++ pipeline.

`./project` is a cross-robot WPILib project that is used
to test the interface with the vision pipeline.

## Licensing

This project is licensed under the WPILib License, I
will manage licensing later. Currently this is Texas
Torque internal software.
