CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra
INCLUDES  = -I$(SRC_DIR)/lexer

SRC_DIR   = src
BUILD_DIR = build

SRCS      = $(SRC_DIR)/main.cpp $(SRC_DIR)/lexer/lexer.cpp $(SRC_DIR)/lexer/token.cpp
OBJS      = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
TARGET    = lexer

ifeq ($(OS),Windows_NT)
TARGET_BIN = $(TARGET).exe
RUN_TARGET = .\\$(TARGET_BIN)
define MKDIR_P
	@if not exist "$(1)" mkdir "$(1)"
endef
RM_DIR = -if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
RM_BIN = -if exist "$(TARGET_BIN)" del /f /q "$(TARGET_BIN)"
else
TARGET_BIN = $(TARGET)
RUN_TARGET = ./$(TARGET_BIN)
define MKDIR_P
	@mkdir -p "$(1)"
endef
RM_DIR = -rm -rf "$(BUILD_DIR)"
RM_BIN = -rm -f "$(TARGET_BIN)" "$(TARGET)"
endif

# Target utama: kompilasi program lexer
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_BIN) $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(call MKDIR_P,$(dir $@))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# Membersihkan file hasil kompilasi
clean:
	$(RM_DIR)
	$(RM_BIN)

# Menjalankan program dengan file input
# Usage: make run INPUT=test/milestone-1/input-1.txt
run: $(TARGET)
	$(RUN_TARGET) $(INPUT)

# Menjalankan program dengan output ke file
# Usage: make run-output INPUT=test/milestone-1/input-1.txt OUTPUT=test/milestone-1/output-1.txt
run-output: $(TARGET)
	$(RUN_TARGET) $(INPUT) $(OUTPUT)

.PHONY: all clean run run-output
