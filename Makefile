DIRS := src src/nes src/nes/mappers src/nes/filters
TARGET := MeowNES
BUILD_DIR := build
CXX := clang++
CXXFLAGS := -Wall -Wextra -Wno-parentheses -O3 -march=native -fno-plt -fstrict-aliasing -funroll-loops -Iinclude $(shell pkg-config --cflags Qt6Widgets) #-fsanitize=address -fsanitize=undefined
LDFLAGS := -lSDL2 $(shell pkg-config --libs Qt6Widgets) #-fsanitize=address -fsanitize=undefined
ifeq ($(OS),Windows_NT)
	LDFLAGS += -lSDL2main -lSDL2 -static
endif

BLUE := $(shell printf "\033[34m")
GREEN := $(shell printf "\033[32m")
WHITE := $(shell printf "\033[97m")
RESET := $(shell printf "\033[0m")

#ifeq ($(OS),Windows_NT)
#   LDFLAGS += -static-libstdc++ -static-libgcc
#	CXX := g++
#endif

SOURCES := $(wildcard $(addsuffix /*.cpp,$(DIRS)))
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@echo "$(BLUE)Linking Object files...$(RESET)"
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "$(BLUE)Built $(TARGET)!$(RESET)"

$(BUILD_DIR)/%.o: %.cpp
	@echo "$(BLUE)Compiling: $(GREEN)$<$(WHITE) -> $(GREEN)$@$(RESET)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all clean