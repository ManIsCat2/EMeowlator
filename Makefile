DIRS := src src/nes src/nes/mappers src/nes/filters
TARGET := MeowNES
BUILD_DIR := build
CXX := clang++
CXXFLAGS := -Wall -Wextra -Wno-parentheses -Iinclude $(shell pkg-config --cflags Qt6Widgets) #-fsanitize=address -fsanitize=undefined
LDFLAGS := -lSDL2 $(shell pkg-config --libs Qt6Widgets) #-fsanitize=address -fsanitize=undefined
ifeq ($(OS),Windows_NT)
	LDFLAGS += -lmingw32 -lSDL2main -lSDL2
endif

#ifeq ($(OS),Windows_NT)
#   LDFLAGS += -static-libstdc++ -static-libgcc
#	CXX := g++
#endif

SOURCES := $(wildcard $(addsuffix /*.cpp,$(DIRS)))
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean