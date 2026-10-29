# CCL Demo

Demo application showcasing UI controls, layouts, 2D/3D graphics, and more available in CCL.

## How to build

Building CCL Demo requires a copy of CCL.

You may install the CCL SDK with precompiled binaries or clone the CCL framework repository.

See [Getting Started with CCL - Installation](https://cclsoftware.github.io/ccl-documentation/cclstartguide/installation.html) for additional dependencies.

### Build from framework sources

Create a new directory and clone both, CCL and CCL Demo, into this directory:

```
mkdir ccl-demo-build && cd ccl-demo-build
git clone https://github.com/cclsoftware/ccl-demo.git
git clone --recursive https://github.com/cclsoftware/ccl-framework.git
```

Then continue with platform-specific build steps (see below), passing the CCL CMake module path as `CMAKE_MODULE_PATH`.

Example:

```
cd ccl-demo/cmake
cmake --preset win64 -DCMAKE_MODULE_PATH=../../ccl-framework/build/cmake/modules/shared/
```

See also [Getting Started with CCL - Building CCL](https://cclsoftware.github.io/ccl-documentation/cclframework/building.html).

### Build using the CCL SDK

Download and install the CCL SDK from the [Releases page](https://github.com/cclsoftware/ccl-framework/releases).

Clone this repository:

```
git clone https://github.com/cclsoftware/ccl-demo.git
```

Then continue with platform-specific build steps below.

### Windows (x86-64)

Generate a Visual Studio solution:

```
cd cmake
cmake --preset win64
```

Open the generated solution in Visual Studio, then build and run CCL Demo:

```
cmake --open build\x64
```

### Windows on Arm

Generate a Visual Studio solution:

```
cd cmake
cmake --preset win-arm64
```

Open the generated solution in Visual Studio, then build and run CCL Demo:

```
cmake --open build\windows-arm64
```

### macOS

Generate an Xcode project:

```
cd cmake
cmake --preset mac-default-sdk
```

Open the generated project in Xcode

```
cmake --open build/mac-default-sdk
```

Visit the Signing & Capabilities settings for the ccldemo target and change the team from "None" to your personal team, then build and run CCL Demo.

### iOS

Generate an Xcode project:

```
cd cmake
cmake --preset ios
```

Open the generated project in Xcode

```
cmake --open build
```

Visit the Signing & Capabilities settings for the ccldemo target, change the team from "None" to your personal team and change the bundle identifier to an available value. Select a hardware device as destination, then build and run CCL Demo.

### Linux (Ubuntu)

Generate a Ninja project:

```
cd cmake
cmake --preset linux-debug
```

Build CCL Demo:

```
cd build/Debug
cmake --build .
```

### Android

Install Ninja as well as Android Studio or Gradle (for command line builds).

Generate an Android Studio/Gradle project:

```
cd cmake
cmake --preset android
```

Open the generated project folder (`build/android` by default) in Android Studio, then build and run CCL Demo.

Alternatively, build CCL Demo using Gradle:

```
cd build/android
gradle assembleArm8Debug
```

The resulting APK will be found in the `ccldemo/build/outputs/apk/arm8/debug` subfolder.

### WebAssembly

Install the [Emscripten SDK](https://github.com/emscripten-core/emsdk), as well as `ninja`, `glslc`, `python` and `sed` (all must be in `PATH`):

```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
git checkout 4.0.22
./emsdk install 4.0.22
./emsdk activate 4.0.22
source ./emsdk_env.sh
```

The `EMSDK` environment variable set by `emsdk_env` is used by the CMake presets to locate the Emscripten toolchain file.

Generate a Ninja project and build CCL Demo:

```
cd cmake
cmake --preset wasm-release
cd build/wasm/Release
cmake --build .
```

Use `wasm-debug` for a debug build (output in `build/wasm/Debug`).

Build artifacts include `.js`, `.wasm` and `.html` files for the ccldemo executable. Serve the build output folder with a local web server and open the ccldemo `.html` page in a browser:

```
python -m http.server 8080
```
