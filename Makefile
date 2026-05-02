CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra

SRC_DIR   = src
BUILD_DIR = build

TARGET    = arion_parser

# Cari semua file .cpp secara rekursif di dalam src/
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRCS      = $(call rwildcard,$(SRC_DIR)/,*.cpp)
OBJS      = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
INCLUDES  = $(addprefix -I,$(sort $(dir $(SRCS))))

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

# Target utama: kompilasi program syntax analyzer
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_BIN) $^

# Rule umum untuk semua file .cpp di dalam src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(call MKDIR_P,$(dir $@))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# Membersihkan file hasil kompilasi
clean:
	$(RM_DIR)
	$(RM_BIN)

# Menjalankan program dengan file input
# Usage: make run INPUT=test/milestone-2/input-1.txt
run: $(TARGET)
	$(RUN_TARGET) $(INPUT)

# Menjalankan program dengan output ke file
# Usage: make run-output INPUT=test/milestone-2/input-1.txt OUTPUT=test/milestone-2/output-1.txt
run-output: $(TARGET)
	$(RUN_TARGET) $(INPUT) $(OUTPUT)

.PHONY: all clean run run-output
