.PHONY: all

# compiler options
CC := g++
CFLAGS := -Wall -pedantic -Wextra
CV := -std=c++11

# filepaths
BIN := bin
BUILD = build
BINARIES := $(BIN)/serialize_state $(BIN)/generate_communities
STATE_OBJECTS := $(BUILD)/parse.o $(BUILD)/canvas.o $(BUILD)/geometry.o $(BUILD)/util.o $(BUILD)/shape.o

# libraries
BOOST := -lboost_serialization -lboost_filesystem
SDL := -lSDL2 -lSDL2main

all: serialize_state
#all: state_serializer

serialize_state: setup dependencies $(STATE_OBJECTS) $(BUILD)/clipper.o src/script/serialize_state.cpp
	$(CC) $(CV) src/script/serialize_state.cpp -c -o $(BUILD)/serialize_state.o
	$(CC) $(CV) $(BUILD)/serialize_state.o $(STATE_OBJECTS) $(BUILD)/clipper.o $(BOOST) $(SDL) -o $(BIN)/serialize_state


dependencies: lib/rapidjson/include/rapidjson/rapidjson.h.dependency

# object file targets:
*/%.o: src/%.cpp
	$(CC) $(CV) $< -c -o $@

$(BUILD)/clipper.o: lib/clipper/clipper.cpp
	$(CC) $< -c -o $(BUILD)/clipper.o

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