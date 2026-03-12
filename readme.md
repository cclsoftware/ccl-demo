# CCL Demo Application

Application demoing features of CCL Cross-platform Framework.


## How to build

Install CMake from https://cmake.org/download/.

Clone this repository:

```
git clone --recursive https://github.com/cclsoftware/ccl-demo.git
```

### Windows (x86-64)

Install Microsoft Visual Studio.

Generate a Visual Studio solution:

```
cd cmake
cmake --preset win64
```

Open the generated solution in Visual Studio, then build and run the CCL Demo Application:

```
cmake --open build\x64
```

### Windows on Arm

Install Microsoft Visual Studio.

Generate a Visual Studio solution:

```
cd cmake
cmake --preset win-arm64
```

Open the generated solution in Visual Studio, then build and run the CCL Demo Application:

```
cmake --open build\windows-arm64
```

### MacOS

Install Xcode.

Generate an Xcode project:

```
cd cmake
cmake --preset mac-default-sdk
```

Open the generated project in Xcode, then build and run the CCL Demo Application:

```
cmake --open build/mac-default-sdk
```

### Linux (Ubuntu)

Install Ninja.
For more details on dependencies, see the "Getting Started with CCL" guide installed with CCL SDK.

Generate a Ninja project:

```
cd cmake
cmake --preset linux-debug
```

Build the CCL Demo Application:

```
cd build/Debug
cmake --build .
```
