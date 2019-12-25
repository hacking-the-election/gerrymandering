.PHONY: all

CC = g++
BINDIR = bin

all: compile

compile:
	@${CC} -std=c++14 src/main.cpp src/util.cpp src/state.cpp -o ${BINDIR}/gerry

install:
	@install -m 0755 ${BINDIR}/gerry /usr/local/bin
