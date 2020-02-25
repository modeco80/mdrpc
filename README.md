## mdrpc

mdrpc is a native Discord Rich Pressence MPV plugin written in C++, licensed under the MIT License.

## Building

Building on either Windows or Linux should be as easy as

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build .
```

### Installation

Copy the DLL or SO to your configured mpv scripts directory or call MPV with `--script=<path to SO/DLL>`.
