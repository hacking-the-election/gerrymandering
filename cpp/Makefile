.PHONY: all
.ONESHELL: all

#==================================
# Define variables
#==================================

# compiler options
CC := g++
CFLAGS := -O2 -Wall -pedantic -Wextra
CV := -std=c++14

# filepaths
BIN := bin
BUILD = build
BINARIES := $(BIN)/serialize_state $(BIN)/generate_communities
STATE_OBJECTS := $(BUILD)/parse.o $(BUILD)/canvas.o $(BUILD)/geometry.o $(BUILD)/util.o $(BUILD)/shape.o $(BUILD)/graph.o $(BUILD)/community.o

# libraries
BOOST := -lboost_serialization -lboost_filesystem
SDL := -lSDL2 -lSDL2main
LIBS := $(BOOST) $(SDL)


.SECONDARY: $(STATE_OBJECTS)
all: serialize_state

#====================================
# Binary targets for end compilations
#====================================

serialize_state: serialize_state.bin
generate_communities: generate_communities.bin
draw_communities: draw_community.bin

#====================================
# binary recipe for script dir files
#====================================

%.bin: setup dependencies $(STATE_OBJECTS) $(BUILD)/clipper.o script/%.cpp
	$(CC) $(CV) $(CFLAGS) script/$(shell echo $@ | cut -d '.' -f 1).cpp -c -o $(BUILD)/$(shell echo $@ | cut -d '.' -f 1).o
	$(CC) $(CV) $(CFLAGS) $(BUILD)/$(shell echo $@ | cut -d '.' -f 1).o $(STATE_OBJECTS) $(BUILD)/clipper.o $(LIBS) -o $(BIN)/$(shell echo $@ | cut -d '.' -f 1)


#=================================
# object file targets:
#=================================

*/%.o: src/%.cpp
	$(CC) $(CV) $< -c -o $@

$(BUILD)/clipper.o: lib/clipper/clipper.cpp
	$(CC) $< -c -o $(BUILD)/clipper.o


#==================================
# files that must exist to compile
#==================================

dependencies: lib/rapidjson/include/rapidjson/rapidjson.h.dependency

%.dependency: %
	@printf "checking for "
	@printf $< | rev | cut -d '/' -f 1 | rev | cut -d '.' -f 1 | tr -d '\n'
	@printf "... "

ifeq (,$(wildcard $<))
	@echo "yes"
else
	@echo "no"
endif


#===================================
# Directory system modification
#===================================

setup:
	mkdir -p $(BIN) $(BUILD)

clean:
	rm -rf $(BIN)/ $(BUILD)/
