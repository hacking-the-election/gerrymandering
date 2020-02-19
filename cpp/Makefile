.PHONY: all

CC = g++
BINDIR = bin
STATE_FILES = src/parse.cpp src/shape.cpp src/geometry.cpp src/gui.cpp src/serialize.cpp src/util.cpp
BOOST = -lboost_serialization -lboost_filesystem
SDL = -lSDL2 -lSDL2main
V = 11
CLIPPER = `geos-config --ldflags` -lgeos
OPT = #-Wall -pedantic -Wextra

#all: community
all: state_serializer

state_serializer:
	@echo "compiling state serializer..."
	@${CC} -std=c++${V} src/script/serialize_state.cpp ${CLIPPER} ${STATE_FILES} ${BOOST} ${SDL} -o ${BINDIR}/serialize_state

community:
	@echo "compiling community algorithm executor"
	@${CC} -std=c++${V} ${STATE_FILES} src/algorithm/community/generate_communities.cpp src/algorithm/community/community.cpp ${BOOST} ${CLIPPER} ${SDL} -o ${BINDIR}/generate_communities
