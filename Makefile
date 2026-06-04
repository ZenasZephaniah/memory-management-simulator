CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Target executable name
TARGET = memsim

# Source files (Added Simulator.cpp)
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/Allocator.cpp \
       $(SRC_DIR)/BuddyAllocator.cpp \
       $(SRC_DIR)/Cache.cpp \
       $(SRC_DIR)/VirtualMemory.cpp \
       $(SRC_DIR)/Simulator.cpp

# Object files matching the sources
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
