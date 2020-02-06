.PHONY: all

CC = g++
BINDIR = bin
FILES = src/util.cpp src/state.cpp src/shape.cpp src/serialize_methods.cpp src/gui.cpp
BOOST = -lboost_serialization
SDL = -lSDL2 -lSDL2main
V = 11

all: serialize #community

serialize:
	@echo "compiling state serializer..."
	@${CC} -std=c++${V} src/serialize_state.cpp ${FILES} ${BOOST} ${SDL} -o ${BINDIR}/serialize_state

community:
	@echo "compiling community algorithm executor"
	@${CC} -std=c++${V} ${FILES} src/generate_communities.cpp src/algorithm/community.cpp ${BOOST} ${SDL} -o ${BINDIR}/generate_communities

install:
	@install -m 0755 ${BINDIR}/gerry /usr/local/bin
