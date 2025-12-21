CXX := g++
PROJECT := slug
CFLAGS := -Wall -Wextra -Werror -Wpedantic
LDFLAGS ?=
INCLUDES := -Iinclude
SRC_DIR := src
SRC := $(wildcard $(SRC_DIR)/*.cpp)
ifeq ($(SRC),)
$(error No source files found in $(SRC_DIR))
endif

DEP := $(OBJ:.o=.d)
ifneq ($(MAKECMDGOALS),clean)
	-include $(wildcard $(DEP))
endif
BUILD_DIR := build
BUILD_ARGS ?= -DDEBUG
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

GREEN := $(shell printf '[0;32m')
CYAN := $(shell printf '[0;36m')
RESET := $(shell printf '[0m')
ECHO = @echo

.PHONY: all build release clean help

all: build

build: $(BUILD_DIR)/$(PROJECT)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(ECHO) "$(CYAN)[BUILD]$(RESET) Compiling $<..."
	@$(CXX) $(CFLAGS) -MMD -MP -c "$<" -o "$@" $(INCLUDES) $(BUILD_ARGS)

$(BUILD_DIR)/$(PROJECT): $(OBJ)
	$(ECHO) "$(CYAN)[LINK]$(RESET) Creating binary at $@"
	@$(CXX) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LDFLAGS) $(BUILD_ARGS)
	$(ECHO) "$(GREEN)[OK]$(RESET) Build complete: $@"

release:
	$(ECHO) "$(CYAN)[RELEASE]$(RESET) Building release version..."
	@$(MAKE) -B build BUILD_ARGS=-O3

clean:
	$(ECHO) "$(CYAN)[CLEAN]$(RESET) Removing build directory..."
	@$(RM) -r $(BUILD_DIR)
	$(ECHO) "$(GREEN)[OK]$(RESET) Clean complete."

help:
	$(ECHO) "$(CYAN)[HELP]$(RESET) Available targets:"
	$(ECHO) "$(CYAN)[HELP]$(RESET)   build     - Compile the project"
	$(ECHO) "$(CYAN)[HELP]$(RESET)   release   - Build with -O3 optimizations"
	$(ECHO) "$(CYAN)[HELP]$(RESET)   clean     - Remove build files"
