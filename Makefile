CC = g++ # specify the used compiler
CFLAGS = -Wall -pthread # specify the options added on compile.
# -Wall -> allow all warnings

all: clean main

# compile parent.cpp
main: ./src/main.cpp ./src/local.hpp ./src/opgl.hpp
	$(CC) $(CFLAGS) ./src/main.cpp -o ./bin/main.out -lglut -lGLU -lGL -lfreetype -I./res/freetype2 -I./res/libpng16

debug: ./src/main.cpp ./src/local.hpp
	$(CC) $(CFLAGS) -g ./src/main.cpp -o ./bin/main.out -lglut -lGLU -lGL -lfreetype -I./res/freetype2 -I./res/libpng16
	gdb ./bin/main.out

run:
	./bin/main.out

clean:
	mkdir -p ./bin
	rm -f ./bin/*
