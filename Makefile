DIRS := src src/nes src/nes/mappers src/nes/filters
TARGET := MeowNES
BUILD_DIR := build

ifeq ($(OS),Windows_NT)
    CXX := g++
    CXXFLAGS := -Wall -Wextra -Wno-parentheses -O3 -march=native -fstrict-aliasing -funroll-loops -Iinclude -static-libstdc++ -static-libgcc
    LDFLAGS := -lSDL2 -lSDL2main
    EXEEXT := .exe
else
    CXX := clang++
    CXXFLAGS := -Wall -Wextra -Wno-parentheses -O3 -march=native -fno-plt -fstrict-aliasing -funroll-loops -Iinclude
    LDFLAGS := -lSDL2
    EXEEXT :=
endif

CXXFLAGS += $(shell pkg-config --cflags Qt6Widgets 2>/dev/null || echo "")
LDFLAGS += $(shell pkg-config --libs Qt6Widgets 2>/dev/null || echo "")

BLUE := $(shell printf "\033[34m")
GREEN := $(shell printf "\033[32m")
WHITE := $(shell printf "\033[97m")
RESET := $(shell printf "\033[0m")

SOURCES := $(wildcard $(addsuffix /*.cpp,$(DIRS)))
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)$(EXEEXT)

$(BUILD_DIR)/$(TARGET)$(EXEEXT): $(OBJECTS)
	@echo "$(BLUE)Linking Object files...$(RESET)"
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "$(BLUE)Built $(TARGET)!$(RESET)"
	@if [ "$(OS)" = "Windows_NT" ]; then \
		echo "$(BLUE)Copying SDL2.dll to $(BUILD_DIR)/...$(RESET)"; \
		cp /mingw64/bin/SDL2.dll $(BUILD_DIR)/; \
		echo "$(BLUE)Running windeployqt to $(BUILD_DIR)/...$(RESET)"; \
		windeployqt --release --dir $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)$(EXEEXT); \
		echo "$(GREEN)Windows deployment to $(BUILD_DIR)/ complete!$(RESET)"; \
	fi

$(BUILD_DIR)/%.o: %.cpp
	@echo "$(BLUE)Compiling: $(GREEN)$<$(WHITE) -> $(GREEN)$@$(RESET)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@echo "$(BLUE)Cleaning build directory...$(RESET)"
	@rm -rf $(BUILD_DIR)

.PHONY: all clean
