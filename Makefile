platform ?= -DPLATFORM=LINUX
platform_flags = $(platform)
cpp_version ?= -std=c++17
debug_flags ?= -DDEBUG -D_GLIBCXX_DEBUG -DDEBUG_VK #-DTRACK_MEM_ALLOC -DPRINT_MEM_ALLOC

VULKAN_SDK_PATH = /code-libraries/cpp/vulkan/latest/x86_64
vulkan_compile_flags = -I$(VULKAN_SDK_PATH)/include
vulkan_linker_flags = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

STB_INCLUDE_PATH = /code-libraries/cpp/stb
stb_compile_flags = -I$(STB_INCLUDE_PATH)

CLFAGS = $(cpp_version) $(vulkan_compile_flags) $(stb_compile_flags)
LDFLAGS = $(vulkan_linker_flags)

pch = pch.h.gch
object_files = main.o

output: $(object_files) $(pch) Makefile
	g++ $(CLFAGS) $(object_files) -o output $(LDFLAGS)

main.o: main.cpp $(pch) Makefile
	g++ $(CLFAGS) -c main.cpp -o main.o $(LDFLAGS) $(platform_flags) $(debug_flags) 



pch.h.gch: pch.h
	g++ $(CLFAGS) pch.h

clear:
	rm *.o
	rm output
	rm *~

build:
	g++ -std=c++17 *.cpp *.h -o build -O3