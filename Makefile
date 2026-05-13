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
		echo "$(BLUE)Running windeployqt...$(RESET)"; \
		windeployqt --release --compiler-runtime --dir $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)$(EXEEXT); \
		echo "$(BLUE)Copying MinGW/MSYS2 DLL dependencies...$(RESET)"; \
		for dll in $$(ntldd -R $(BUILD_DIR)/$(TARGET)$(EXEEXT) \
			| grep "mingw64/bin" \
			| sed -E 's/.*=> (.*) \(0x.*/\1/' \
			| sort -u); do \
			echo "$(GREEN)$$(basename "$$dll")$(RESET)"; \
			cp "$$dll" $(BUILD_DIR)/; \
		done; \
		echo "$(GREEN)Deployment complete!$(RESET)"; \
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
