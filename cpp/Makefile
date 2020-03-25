.PHONY: all

# compiler options
CC := g++
CFLAGS := -Wall -pedantic -Wextra
CV := -std=c++11

# filepaths
BIN := bin
BINARIES := $(BINDIR)/serialize_state $(BINDIR)/generate_communities
STATE_OBJECTS = parse.o canvas.o parse.o geometry.o util.o
BUILD = build

# libraries
BOOST = -lboost_serialization -lboost_filesystem
SDL = -lSDL2 -lSDL2main

all: compile
#all: state_serializer

compile: setup dependencies $(STATE_OBJECTS) clipper
	@echo dependencies done
	$(CC) -std=c++$(CV) $(STATE_OBJECTS) $(BOOST) $(SDL) -o $(BIN)/serialize_state

dependencies: lib/rapidjson/include/rapidjson/rapidjson.h.dependency

# object file targets:
%.o: src/%.cpp
	$(CC) $(CV) $< -c -o $@

clipper:
	$(CC) lib/clipper/clipper.cpp -c -o build/clipper.o

%.dependency: %
	@printf "checking for "
	@printf $< | rev | cut -d '/' -f 1 | rev | cut -d '.' -f 1 | tr -d '\n'
	@printf "... "

ifeq (,$(wildcard $<))
	@echo "yes"
else
	@echo "no"
endif


setup:
	mkdir -p $(BIN) $(BUILD)

clean:
	rm -f $(BIN)/ $(BUILD)/