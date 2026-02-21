COMPILE_FOLDERS := src src/mappers
TARGET := MeowNES
BUILD_DIR := build
CXX := clang++
CXXFLAGS := -Wall -Wextra -O3 -Iinclude $(shell pkg-config --cflags Qt6Widgets) -fsanitize=address -fsanitize=undefined
LDFLAGS := -fsanitize=address -fsanitize=undefined
ifeq ($(OS),Windows_NT)
	LDFLAGS += -Llib -lQt6CoreStatic -lQt6GuiStatic -lQt6WidgetsStatic -static
	CXX := g++
else
	LDFLAGS += $(shell pkg-config --libs Qt6Widgets)
endif

SOURCES := $(wildcard $(addsuffix /*.cpp,$(COMPILE_FOLDERS)))

OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	cp -r gui/ $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
