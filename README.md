## mdrpc2

mdrpc is a (Work-In-Progress):tm: native Discord Rich Pressence MPV plugin.

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