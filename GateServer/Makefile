# Makefile for GateServer (Flat Directory Structure)

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -g
LDFLAGS := -L/usr/local/lib
LIBS := -ljsoncpp -lgrpc++ -lgrpc `pkg-config --libs protobuf` -lpthread


# Source files and object files
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp, %.o, $(SRCS))

# Target executable
TARGET := GateServer

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "Build complete: $(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(TARGET)
	@echo "Build directory cleaned."