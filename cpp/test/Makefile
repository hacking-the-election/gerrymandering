.PHONY: all
all: shape

CC = g++

shape:
	@${CC} -std=c++11 shape_test.cpp ../src/gui.cpp ../lib/clipper/clipper.cpp ../src/geometry.cpp ../src/shape.cpp ../src/serialize.cpp -Wall -lSDL2main -lSDL2 -lboost_serialization -o test
