CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -g
CXXFLAGS += -Wno-deprecated-declarations
LDFLAGS = -lfcgi -lsqlite3 -ljsoncpp -lssl -lcrypto  # Link with jsoncpp, OpenSSL (ssl and crypto)

# Add the include path for jsoncpp and headers in 'inc'
INCLUDES = -I/usr/include/jsoncpp -I./inc  # Adjust the jsoncpp path as needed

# Define the source and object files paths
SRC_DIR = src
OBJ_DIR = build

# List of source files
SRC = $(SRC_DIR)/main.cpp $(SRC_DIR)/routes.cpp $(SRC_DIR)/db.cpp $(SRC_DIR)/auth.cpp $(SRC_DIR)/Logger.cpp
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
EXEC = user_service

# Default target to build the executable
all: $(EXEC)

# Ensure the build directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Rule to build the final executable
$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Rule to compile .cpp files to .o object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

# Rule for running the service (optional)
run: $(EXEC)
	./$(EXEC)

.PHONY: all clean run
