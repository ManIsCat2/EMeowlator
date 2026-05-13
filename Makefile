DIRS := src src/nes src/nes/mappers src/nes/filters
TARGET := MeowNES
BUILD_DIR := build

ifeq ($(OS),Windows_NT)
    CXX := g++
    CXXFLAGS := -Wall -Wextra -Wno-parentheses -O3 -march=native -fstrict-aliasing -funroll-loops -Iinclude -static-libstdc++ -static-libgcc
    EXEEXT := .exe
	WINDEPLOYQT := $(shell command -v windeployqt6 2>/dev/null || command -v windeployqt 2>/dev/null)
else
    CXX := clang++
    CXXFLAGS := -Wall -Wextra -Wno-parentheses -O3 -march=native -fno-plt -fstrict-aliasing -funroll-loops -Iinclude
    EXEEXT :=
endif

LDFLAGS := $(shell pkg-config --libs Qt6Widgets sdl2)
CXXFLAGS += $(shell pkg-config --cflags Qt6Widgets sdl2)

ifeq ($(OS),Windows_NT)
	LDFLAGS += resource.res
endif

BLUE := $(shell printf "\033[34m")
GREEN := $(shell printf "\033[32m")
WHITE := $(shell printf "\033[97m")
RESET := $(shell printf "\033[0m")

SOURCES := $(wildcard $(addsuffix /*.cpp,$(DIRS)))
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)$(EXEEXT)

$(BUILD_DIR)/$(TARGET)$(EXEEXT): $(OBJECTS)
	@echo "$(BLUE)Linking object files...$(RESET)"
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

	@if [ "$(OS)" = "Windows_NT" ]; then \
		echo "$(BLUE)Running windeployqt to $(BUILD_DIR)/...$(RESET)"; \
		$(WINDEPLOYQT) --release --compiler-runtime --dir $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)$(EXEEXT); \
	fi
	@echo "$(GREEN)Build complete!$(RESET)"

$(BUILD_DIR)/%.o: %.cpp
	@echo "$(BLUE)Compiling: $(GREEN)$<$(WHITE) -> $(GREEN)$@$(RESET)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@echo "$(BLUE)Cleaning build directory...$(RESET)"
	@rm -rf $(BUILD_DIR)

copydll: # only for windows!
	@echo "$(BLUE)Copying MinGW runtime dependencies...$(RESET)"
	@ntldd -R "$(BUILD_DIR)/$(TARGET)$(EXEEXT)" | \
	grep -i "mingw64" | \
	sed -E 's/.*=> //g' | \
	sed -E 's/ \(0x.*\)//g' | \
	tr '\\\\' '/' | \
	sed -E 's|^[A-Za-z]:||' | \
	sed -E 's|^/.*?/mingw64|/mingw64|' | \
	tr -d '\r' | \
	while read dll; do \
		cp -u "$$dll" "$(BUILD_DIR)/"; \
	done

.PHONY: copydll all clean