CC = g++ # specify the used compiler
CFLAGS = -Wall -pthread # specify the options added on compile.
# -Wall -> allow all warnings

all: main

# compile parent.cpp
main: ./src/main.cpp ./src/local.hpp
	$(CC) $(CFLAGS) ./src/main.cpp -o ./bin/main

#compile opgl.cpp
#opgl: opgl.cpp
#	$(CC) opgl.cpp -o opgl -lglut -lGLU -lGL

debug: ./src/main.cpp ./src/local.hpp
	$(CC) $(CFLAGS) -g ./src/main.cpp -o ./bin/main
	gdb ./bin/main

run:
	./bin/main

clean:
	rm -f ./bin/*
