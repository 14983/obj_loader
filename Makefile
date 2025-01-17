CXX      := g++
CXXFLAGS := -Wall -I/usr/include/imgui -Iinclude/
LDFLAGS  := -lglfw -lGLEW -lGL -limgui -lstb

SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:.cpp=.o)
TARGET   := main

# Build rules
all: $(TARGET)

run: all
	./$(TARGET) || true

%.o: %.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJ)
	rm -f *.ini

.PHONY: all clean run
