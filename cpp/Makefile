.PHONY: setup clean dependencies

CXX := g++
OFLAGS := -O3 -MMD -MP -ggdb
CXXFLAGS := -O3 -Wall -pedantic -Wextra -ggdb
STDV := -std=c++17
SRC := src
BIN := bin
BUILD := build

OBJECTS := parse.o graphics.o geometry.o util.o shape.o graph.o community.o quantification.o
OBJECT_OUTPUTS = $(patsubst %, $(BUILD)/%, $(OBJECTS))
DEPENDS := $(patsubst %.o,%.d,$(OBJECTS))

.SECONDARY: $(OBJECT_OUTPUTS)

GLFW := -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lXi -ldl
BOOST := -lboost_serialization -lboost_filesystem -lboost_system
SDL := `sdl2-config --cflags` `sdl2-config --libs`
LIBS := -pthread $(GLFW) $(BOOST) $(SDL)


serialize_state: $(BIN)/serialize_state
sandbox: $(BIN)/sandbox

dependencies: $(BUILD)/clipper.o


$(BIN)/%: setup dependencies $(OBJECT_OUTPUTS) script/%.cpp
	$(CXX) $(STDV) $(OFLAGS) -MT $(BUILD)/$*.o script/$*.cpp -c -o $(BUILD)/$*.o
	$(CXX) $(STDV) $(CFLAGS) $(BUILD)/$*.o $(OBJECT_OUTPUTS) $(BUILD)/clipper.o $(LIBS) -o $(BIN)/$*

# Compile object files for project and deps
$(BUILD)/%.o: $(SRC)/%.cpp
	$(CXX) $(STDV) $(OFLAGS) -MT $@ $< -c -o $@

$(BUILD)/clipper.o: lib/Clipper/cpp/clipper.cpp
	$(CXX) $(STDV) $(OFLAGS) -MT $@ $< -c -o $@

setup:
	mkdir -p $(BIN) $(BUILD)

clean:
	rm $(BUILD)/*.o
	rm -rf $(BIN)