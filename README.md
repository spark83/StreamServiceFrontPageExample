# StreamServiceFrontPageExample
Streaming service main page display written in C and OpenGL

![image](https://user-images.githubusercontent.com/9837282/139688160-16610f10-1bf4-4723-adee-f18fc7f0f5ba.png)

How to build project
VS 2019 required.
Simply open .sln file and build.

Exxecutable included inside of Executable folder.

Known Issues:
1. Not all font scaling is correct.
2. Font advance offset is bit off.

TODO:
1. Custom allocators: Stack allocator, pool allocator, free allocator.
2. Compress texture format into DXT1 for less memory usage in gpu.
3. GUI scene into rendering commands that gets sorted.
4. Use optimized font generation.
