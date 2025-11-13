# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -Iexternal/liquibook/src -pthread

# FastRTPS and FastCDR libraries
LIBS = -lfastrtps -lfastcdr

# Directories
BUILD_DIR = build
SRC_DIR   = src
EXTERNAL_DIR = external

# Target executable
TARGET = $(BUILD_DIR)/recon_orderbook

# Source files
SRC = $(SRC_DIR)/main.cpp \
      $(SRC_DIR)/MBOSubscriber.cpp \
      $(SRC_DIR)/OrderBookManager.cpp \
      $(SRC_DIR)/Order.cpp

# Default target
all: check_liquibook $(BUILD_DIR) $(TARGET)

# Check if Liquibook exists
check_liquibook:
	@if [ ! -d "$(EXTERNAL_DIR)/liquibook" ]; then \
		echo "ERROR: Liquibook not found in $(EXTERNAL_DIR)/liquibook"; \
		echo "Please run: git clone https://github.com/enewhuis/liquibook.git $(EXTERNAL_DIR)/liquibook"; \
		exit 1; \
	fi

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

# Setup Liquibook (helper target)
setup:
	@echo "Cloning Liquibook..."
	@mkdir -p $(EXTERNAL_DIR)
	@if [ ! -d "$(EXTERNAL_DIR)/liquibook" ]; then \
		git clone https://github.com/enewhuis/liquibook.git $(EXTERNAL_DIR)/liquibook; \
		echo "Liquibook cloned successfully"; \
	else \
		echo "Liquibook already exists"; \
	fi

.PHONY: all run clean check_liquibook setup