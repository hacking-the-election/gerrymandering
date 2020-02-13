.PHONY: all

CC = g++
BINDIR = bin
STATE_FILES = src/parse.cpp src/shape.cpp src/geometry.cpp src/gui.cpp src/serialize.cpp src/util.cpp
BOOST = -lboost_serialization -lboost_filesystem
SDL = -lSDL2 -lSDL2main
V = 11

all: community

state_serializer:
	@echo "compiling state serializer..."
	@${CC} -std=c++${V} src/script/serialize_state.cpp ${STATE_FILES} ${BOOST} ${SDL} -o ${BINDIR}/serialize_state

community:
	@echo "compiling community algorithm executor"
	@${CC} -std=c++${V} ${STATE_FILES} lib/clipper/clipper.cpp src/algorithm/community/generate_communities.cpp src/algorithm/community/community.cpp ${BOOST} ${SDL} -o ${BINDIR}/generate_communities
