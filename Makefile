CXX      = g++
CXXFLAGS = -Wall -std=c++17 -Isrc
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)

ifeq ($(strip $(RAYLIB_LIBS)),)
	RAYLIB_LIBS = -lraylib
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	RAYLIB_LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif

SRC = $(wildcard src/*.cpp src/data_structures/*.cpp)
OUT = build/program

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(RAYLIB_CFLAGS) -o $(OUT) $(SRC) $(RAYLIB_LIBS)

clean:
	rm -f $(OUT)

run: all
	./$(OUT)
