DIRS := src src/nes src/nes/mappers src/nes/filters
TARGET := MeowNES
BUILD_DIR := build

ifeq ($(OS),Windows_NT)
    CXX := g++
    CXXFLAGS := -Wall -Wextra -Wno-parentheses -O3 -march=native -fstrict-aliasing -funroll-loops -Iinclude -static-libstdc++ -static-libgcc
    LDFLAGS :=
    EXEEXT := .exe
else
    CXX := clang++
    CXXFLAGS := -Wall -Wextra -Wno-parentheses -O3 -march=native -fno-plt -fstrict-aliasing -funroll-loops -Iinclude
    LDFLAGS :=
    EXEEXT :=
endif

CXXFLAGS += $(shell pkg-config --cflags Qt6Widgets sdl2)
LDFLAGS += $(shell pkg-config --libs Qt6Widgets sdl2)

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
		echo "$(BLUE)Running windeployqt to $(BUILD_DIR)/...$(RESET)"; \
		windeployqt --release --compiler-runtime --dir $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)$(EXEEXT); \
		echo "$(BLUE)Copying MINGW DLLs...$(RESET)"; \
		cp /mingw64/bin/libgcc_s_seh-1.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libwinpthread-1.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/Qt6Core.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libb2-1.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libdouble-conversion.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libicuin78.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libicuuc78.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libicudt78.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libpcre2-16-0.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/zlib1.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libzstd.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/Qt6Gui.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libfreetype-6.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libbrotlidec.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libbrotlicommon.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libbz2-1.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libharfbuzz-0.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libglib-2.0-0.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libintl-8.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libiconv-2.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libpcre2-8-0.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libgraphite2.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libmd4c.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libpng16-16.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/Qt6Widgets.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/SDL2.dll $(BUILD_DIR)/; \
		cp /mingw64/bin/libstdc++-6.dll $(BUILD_DIR)/; \
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
