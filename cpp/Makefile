.PHONY: setup clean


# Setup global parameters for the compiler
CXXFLAGS := -std=c++17 -MMD -MP -O3 -w -Wall -pedantic -Wextra -ggdb -Wno-int-to-pointer-cast -Wno-return-type

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
SCRIPT_BUILD_DIR := build/script
TEST_SCRIPT_DIR := test/tests
TEST_SCRIPT_BUILD_DIR := build/test/tests

# Get a list of all the source files that need to be compiled to objects
SRC_FILES := $(shell find $(SRC_DIR) -name *.cpp)
TEST_SRC_FILES := $(shell find $(TEST_SRC_DIR) -maxdepth 1 -name *.cpp)

# Setup destination objects for each of the source files
OBJECT_DESTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
TEST_OBJECT_DESTS := $(patsubst $(TEST_SRC_DIR)/%.cpp, $(TEST_BUILD_DIR)/%.o, $(TEST_SRC_FILES))

# Special case object outputs (will need a special .o compilation rule)
DEPENDENCIES_OBJECT_DESTS := $(BUILD_DIR)/clipper.o
OBJECT_DESTS += $(DEPENDENCIES_OBJECT_DESTS)
ALL_OBJECTS := $(OBJECT_DESTS) $(TEST_OBJECT_DESTS)


# Get a list of all the script and test script files that need to be compiled to executables
SCRIPT_FILES := $(shell find $(SCRIPT_DIR) -name *.cpp)
BIN_SCRIPT_FILES := $(patsubst $(SCRIPT_DIR)/%.cpp, $(BIN_DIR)/%, $(SCRIPT_FILES))

TEST_SCRIPT_FILES := $(shell find $(TEST_SCRIPT_DIR) -name *.cpp)
BIN_TEST_SCRIPT_FILES := $(patsubst $(TEST_SCRIPT_DIR)/%.cpp, $(TEST_BIN_DIR)/%, $(TEST_SCRIPT_FILES))

ALL_OBJECTS += $(patsubst $(SCRIPT_DIR)/%.cpp, $(SCRIPT_BUILD_DIR)/%.o, $(SCRIPT_FILES))
ALL_OBJECTS += $(patsubst $(TEST_SCRIPT_DIR)/%.cpp, $(TEST_SCRIPT_BUILD_DIR)/%.o, $(TEST_SCRIPT_FILES))
.SECONDARY: $(ALL_OBJECTS)

all: $(BIN_SCRIPT_FILES) $(BIN_TEST_SCRIPT_FILES)

# Create dependency targets for each of the object files
DEPENDS_DESTS := $(patsubst %.o, %.d, $(OBJECT_DESTS))
TEST_DEPENDS_DESTS := $(patsubst %.o, %.d, $(TEST_OBJECT_DESTS))
ALL_DEPENDS := $(DEPENDS_DESTS) $(TEST_DEPENDS_DESTS)


# Compilation rules for the test and script binaries
$(BIN_DIR)/%: $(OBJECT_DESTS) $(SCRIPT_BUILD_DIR)/%.o
	$(CXX) $^ $(CXXFLAGS) -o $(BIN_DIR)/$* $(LDFLAGS)

$(TEST_BIN_DIR)/%: $(OBJECT_DESTS) $(TEST_OBJECT_DESTS) $(TEST_SCRIPT_BUILD_DIR)/%.o
	$(CXX) $^ $(CXXFLAGS) -o $(TEST_BIN_DIR)/$* $(LDFLAGS)


# Compilation rules for objects that contain entry points (src must be located in script dirs)
$(SCRIPT_BUILD_DIR)/%.o: $(SCRIPT_DIR)/%.cpp | setup
	$(CXX) $(CXXFLAGS) -MT $@ $< -c -o $@

$(TEST_SCRIPT_BUILD_DIR)/%.o: $(TEST_SCRIPT_DIR)/%.cpp | setup
	$(CXX) $(CXXFLAGS) -MT $@ $< -c -o $@


# Compilation rules for the two types of source object files (test and src)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | setup
	$(CXX) $(CXXFLAGS) -MT $@ $< -c -o $@

$(TEST_BUILD_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp | setup
	$(CXX) $(CXXFLAGS) -MT $@ $< -c -o $@


# Clipper is a special case object (since its src doesn't reside in $SRC_DIR or $TEST_SRC_DIR)
$(BUILD_DIR)/clipper.o: lib/Clipper/cpp/clipper.cpp | setup
	$(CXX) $(CXXFLAGS) -MT $@ $< -c -o $@


setup:
	mkdir -p $(BIN_DIR) $(TEST_BIN_DIR) $(BUILD_DIR) $(TEST_BUILD_DIR) $(SCRIPT_BUILD_DIR) $(TEST_SCRIPT_BUILD_DIR)

clean:
	rm -rf $(BIN_DIR) $(TEST_BIN_DIR) $(BUILD_DIR) $(TEST_BUILD_DIR) $(SCRIPT_BUILD_DIR) $(TEST_SCRIPT_BUILD_DIR)


-include $(ALL_DEPENDS)