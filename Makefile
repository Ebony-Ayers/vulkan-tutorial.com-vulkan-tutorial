platform ?= -DPLATFORM=LINUX
platform_flags = $(platform)
debug_flags ?= -DDEBUG -D_GLIBCXX_DEBUG #-DTRACK_MEM_ALLOC -DPRINT_MEM_ALLOC

VULKAN_SDK_PATH = /vulkan/1.2.131.2/x86_64
vulkan_compile_flags = -I$(VULKAN_SDK_PATH)/include
vulkan_linker_flags = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

pch = pch.h.gch
object_files = main.o

output: $(object_files) $(pch) Makefile
	g++ -std=c++17 $(object_files) -o output $(vulkan_linker_flags)

main.o: main.cpp $(pch) Makefile
	g++ -std=c++17 -c main.cpp -o main.o $(platform_flags) $(debug_flags) $(vulkan_compile_flags)



pch.h.gch: pch.h
	g++ -std=c++17 pch.h

clear:
	rm *.o
	rm output
	rm *~

build:
	g++ -std=c++17 *.cpp *.h -o build -O3