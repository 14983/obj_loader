CXX      := g++
CXXFLAGS := -Wall
LDFLAGS  := -lglfw -lGLEW -lGL

SRC      := $(wildcard *.cpp)
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

.PHONY: all clean run
