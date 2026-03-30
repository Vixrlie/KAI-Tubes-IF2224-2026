CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra

SRC_DIR   = src
BUILD_DIR = build

SRCS      = $(SRC_DIR)/main.cpp $(SRC_DIR)/lexer/lexer.cpp $(SRC_DIR)/lexer/token.cpp
OBJS      = $(patsubst $(SRC_DIR)/lexer/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
TARGET    = lexer

# Target utama: kompilasi program lexer
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/lexer/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Membersihkan file hasil kompilasi
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Menjalankan program dengan file input
# Usage: make run INPUT=test/milestone-1/input-1.txt
run: $(TARGET)
	./$(TARGET) $(INPUT)

# Menjalankan program dengan output ke file
# Usage: make run-output INPUT=test/milestone-1/input-1.txt OUTPUT=test/milestone-1/output-1.txt
run-output: $(TARGET)
	./$(TARGET) $(INPUT) $(OUTPUT)

.PHONY: all clean run run-output
