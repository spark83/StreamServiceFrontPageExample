# StreamServiceFrontPageExample
Streaming service main page display written in C and OpenGL

![image](https://user-images.githubusercontent.com/9837282/139517846-3f11ae7e-7b55-4ef0-b61b-138f039bfc44.png)

How to build project
VS 2019 required.
Simply open .sln file and build.

Exxecutable included inside of Executable folder.

Known Issues:
1. Font s,t coordinates are not correct.
2. One of the image file doesn't seem to exist in one of the given URL included in json file.

TODO:
1. Custom allocators: Stack allocator, pool allocator, free allocator.
2. Compress texture format into DXT1 for less memory usage in gpu.
3. GUI scene into rendering commands.
