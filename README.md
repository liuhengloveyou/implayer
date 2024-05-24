# implayer
ImPlayer: Video player write with sdl2/ffmpeg/imgui

## Features
- run in web browser by wasm.

## compile on windows

1. generate makefile
```
cmake -S . -B build
```

2. make
```
cmake --build build
```

本地编译，用事件可以吗？
不用线程显示视频？
多线程听声音？推声音？