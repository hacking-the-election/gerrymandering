.PHONY: all

CC = g++
BINDIR = bin

all: compile

compile:
	@${CC} -std=c++11 src/main.cpp -o ${BINDIR}/gerry

install:
	@install -m 0755 ${BINDIR}/gerry /usr/local/bin
