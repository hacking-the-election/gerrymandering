.PHONY: all

CC = g++
BINDIR = bin

all: serialize

serialize:
	@echo "compiling state serializer..."
	@${CC} -std=c++11 src/serialize_state.cpp src/util.cpp src/state.cpp src/shape.cpp src/json.cpp src/gui.cpp -lboost_serialization -lSDL2 -lSDL2main -o ${BINDIR}/serialize_state

install:
	@install -m 0755 ${BINDIR}/gerry /usr/local/bin
