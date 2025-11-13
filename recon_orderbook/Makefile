# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -pthread

# FastRTPS and FastCDR libraries
LIBS = -lfastrtps -lfastcdr

# Directories
BUILD_DIR = build
SRC_DIR   = src

# Target executable
TARGET = $(BUILD_DIR)/recon_orderbook

# Source files
SRC = $(SRC_DIR)/main.cpp $(SRC_DIR)/MBOSubscriber.cpp

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the executable
$(TARGET): $(BUILD_DIR) $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean build
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean