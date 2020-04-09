.PHONY: all
all: shape

CC = g++

shape:
	${CC} -std=c++11 -O3 shape_test.cpp ../src/canvas.cpp ../src/util.cpp ../lib/clipper/clipper.cpp ../src/geometry.cpp ../src/shape.cpp -w -lSDL2main -lSDL2 -lboost_serialization -lboost_filesystem -o test

