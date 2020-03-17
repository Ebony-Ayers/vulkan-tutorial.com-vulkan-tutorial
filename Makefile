platform ?= -DPLATFORM=LINUX
platform_flags = $(platform)
debug_flags ?= -DDEBUG -D_GLIBCXX_DEBUG -DTRACK_MEM_ALLOC -DPRINT_MEM_ALLOC

pch = pch.h.gch
object_files = main.o

output: $(object_files) $(pch) Makefile
	g++ -std=c++17 $(object_files) -o output

main.o: main.cpp $(pch) Makefile
	g++ -std=c++17 -c main.cpp -o main.o $(platform_flags) $(debug_flags)



pch.h.gch: pch.h
	g++ -std=c++17 pch.h

clear:
	rm *.o
	rm output
	rm *~

build:
	g++ -std=c++17 *.cpp *.h -o build -O3