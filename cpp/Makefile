.PHONY: setup clean

#hmmm do we need this?
# CXX := g++

# Setup global parameters for the compiler
CXXFLAGS := -std=c++17 -MMD -MP -O3 -w # -Wall -pedantic -Wextra -ggdb -Wno-int-to-pointer-cast -Wno-return-type

# Define library/dependency flags for CXXFLAGS
GLFW := -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lXi -ldl -pthread
LDFLAGS := $(GLFW)

# Define directories for source files and build output
SRC_DIR := src
TEST_SRC_DIR := test

BIN_DIR := bin
BUILD_DIR := build
TEST_BIN_DIR := bin/test
TEST_BUILD_DIR := build/test

SCRIPT_DIR := script
TEST_SCRIPT_DIR := test/tests

# Search for source files within the source directories
SRC_FILES := $(shell find $(SRC_DIR) -name *.cpp)
TEST_SRC_FILES := $(shell find $(TEST_SRC_DIR) -maxdepth 1 -name *.cpp)

# Setup destination objects for each of the matched source files
OBJECT_DESTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
TEST_OBJECT_DESTS := $(patsubst $(TEST_SRC_DIR)/%.cpp, $(TEST_BUILD_DIR)/%.o, $(TEST_SRC_FILES))
DEPENDENCIES_OBJECT_DESTS := $(BUILD_DIR)/clipper.o
ALL_OBJECTS := $(OBJECT_DESTS) $(TEST_OBJECT_DESTS) $(DEPENDENCIES_OBJECT_DESTS)

all: $(SCRIPT_DIR)/sandbox.cpp

# Create dependency targets for each of the object files
DEPENDS_DESTS := $(patsubst %.o, %.d, $(OBJECT_DESTS))
TEST_DEPENDS_DESTS := $(patsubst %.o, %.d, $(TEST_OBJECT_DESTS))
ALL_DEPENDS := $(DEPENDS_DESTS) $(TEST_DEPENDS_DESTS)


# Compilation rules for the test and script binaries
$(SCRIPT_DIR)/%.cpp: $(OBJECT_DESTS) $(DEPENDENCIES_OBJECT_DESTS)
	$(CXX) $(CXXFLAGS) $@ -c -o $(BUILD_DIR)/$*.o
	$(CXX) $^ $(BUILD_DIR)/$*.o $(CXXFLAGS) -o $(BIN_DIR)/$* $(LDFLAGS)

$(TEST_SCRIPT_DIR)/%.cpp: $(TEST_OBJECT_DESTS) $(OBJECT_DESTS) $(DEPENDENCIES_OBJECT_DESTS)
	$(CXX) $(CXXFLAGS) $@ -c -o $(TEST_BUILD_DIR)/$*.o
	$(CXX) $^ $(TEST_BUILD_DIR)/$*.o $(CXXFLAGS) -o $(TEST_BIN_DIR)/$* $(LDFLAGS)


# Compilation rules for the two types of project object files 
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | setup
	$(CXX) $(CXXFLAGS) -MT $@ $< -c -o $@

$(TEST_BUILD_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp | setup
	$(CXX) $(CXXFLAGS) -MT $@ $< -c -o $@


# Clipper is a special case object (since its src doesn't reside in $SRC_DIR or $TEST_SRC_DIR)
$(BUILD_DIR)/clipper.o: lib/Clipper/cpp/clipper.cpp | setup
	$(CXX) $(CXXFLAGS) -MT $@ $< -c -o $@


setup:
	mkdir -p $(BIN_DIR) $(TEST_BIN_DIR) $(BUILD_DIR) $(TEST_BUILD_DIR)

clean:
	rm -rf $(BIN_DIR) $(TEST_BIN_DIR) $(BUILD_DIR) $(TEST_BUILD_DIR)


-include $(ALL_DEPENDS)