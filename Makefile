# CDC/ACM Memory Management Simulator Makefile (Modular)

CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -Iinclude 

# Directories
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = .

# Target Executable
TARGET = $(BIN_DIR)/mms

# Source Files
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/allocator.cpp \
       $(SRC_DIR)/cache.cpp \
       $(SRC_DIR)/virtual_memory.cpp

# Object Files (Auto-generated in build folder)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default Rule
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build Complete! Run ./mms to start."

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET) logs/*.log

.PHONY: all clean