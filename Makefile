CC = g++ # specify the used compiler
CFLAGS = -Wall -pthread # specify the options added on compile.
# -Wall -> allow all warnings

all: clean main

# compile parent.cpp
main: ./src/main.cpp ./src/local.hpp
	$(CC) $(CFLAGS) ./src/main.cpp -o ./bin/main.out

#compile opgl.cpp
#opgl: opgl.cpp
#	$(CC) opgl.cpp -o opgl -lglut -lGLU -lGL

debug: ./src/main.cpp ./src/local.hpp
	$(CC) $(CFLAGS) -g ./src/main.cpp -o ./bin/main.out
	gdb ./bin/main.out

run:
	./bin/main.out

clean:
	mkdir -p ./bin
	rm -f ./bin/*
