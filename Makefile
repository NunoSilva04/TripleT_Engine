CC = gcc
LIB_CMD = ar

COMPILE_FLAGS = -Wall -Wextra -g -I$(INCLUDE_DIR)
LIB_OP_FLAG = rcs

# Library Name
LIB_NAME = TripleT_Engine

# Installation Directories
INSTALL_LIB_DIR = /usr/local/lib/
INSTALL_INC_DIR = /usr/local/include/$(LIB_NAME)

# Library and Objects Directories
LIB_DIR = Lib/
OBJ_DIR = Objects/

# Include Directory
INCLUDE_DIR = Include/

# Tools Directory
TOOLS_DIR:= Tools/
TOOLS_C_FILES:= $(wildcard $(TOOLS_DIR)*.c)
SPV_TO_HEADER_TOOL:= $(TOOLS_DIR)spv_to_header


# Shaders
GLSLC ?= $(shell command -v glslc)
# strip simply removes the leading and trailing white spaces from the string
# make sure that you alwasy add a space after ifeq or ifneq otherwise it will cause make to not work
ifeq ($(strip $(GLSLC)),)
	override GLSLC = /usr/bin/glslc # Override is needed here because make always gives higher priority to the path specified by the user, so we need to override it.
$(info entered)
endif
# This checks wether or not the path provided is an actual valid path
ifeq ($(wildcard $(GLSLC)),)	
$(error glslc command not found. Install glslc through your package manager or specify a location where glslc by doing "make GLSLC=path/to/glslc <configuration>", where configuration is either x11 or wayland)
endif

SHADERS_DIR = Shaders/
SHADERS_COMPILED_DIR = Shaders_Compiled/
SHADERS_GLSL_FILES:= $(wildcard $(SHADERS_DIR)*)
# Here we filter because patsubst keeps files in the list that do not match the pattern meaning any file in SHADERS_GLSL_FILES that does not match .vert will be added to the list
# Also '\' has to be the last thing on that line. Even if i add a comment it will fail and not even commence. Anything really, spaces comments whatever just make sure you instantly go to the next line
SHADERS_SPV_FILES = $(patsubst $(SHADERS_DIR)%.vert,$(SHADERS_COMPILED_DIR)%.spv, $(filter %.vert,$(SHADERS_GLSL_FILES)))
SHADERS_SPV_FILES += $(patsubst $(SHADERS_DIR)%.frag,$(SHADERS_COMPILED_DIR)%.spv,$(filter %.frag,$(SHADERS_GLSL_FILES)))
SHADERS_HEADER_DIR = Src/Graphics/Internals/
SHADERS_HEADER_FILES:= $(patsubst $(SHADERS_COMPILED_DIR)%.spv,$(SHADERS_HEADER_DIR)%.h,$(SHADERS_SPV_FILES))

# This is needed since it's going to be later used as a secondary step, so in order to not get deleted immmediatly we must mark it as .SECONDARY or .PRECIOUS
.SECONDARY: $(SHADERS_SPV_FILES)

# X11 Variables
# UI
SRC_UI_X11_DIR = Src/UI/X11/
UI_X11_C_FILES = $(wildcard $(SRC_UI_X11_DIR)*.c)
UI_X11_OBJ_FILES = $(patsubst $(SRC_UI_X11_DIR)%.c,$(OBJ_DIR)%.o,$(UI_X11_C_FILES))
# Graphics
SRC_GRAPHICS_X11_DIR = Src/Graphics/X11/
GRAPHICS_X11_C_FILES = $(wildcard $(SRC_GRAPHICS_X11_DIR)*.c)
GRAPHICS_X11_OBJ_FILES = $(patsubst $(SRC_GRAPHICS_X11_DIR)%.c,$(OBJ_DIR)%.o,$(GRAPHICS_X11_C_FILES))

# Wayland Variables
# UI
# Graphics

# Help
.DEFAULT_GOAL:= help

help:
	@echo "Choose what type of engine configuration you want:"
	@echo "'make x11' to build for X11, followed by sudo make install_x11"
	@echo "'make wayland' to build for Wayland, followed by sudo make install_wayland"

# PHONY
.PHONY: create_object_dir create_shaders_compiled_dir

create_object_dir: 
	mkdir -p $(OBJ_DIR)

create_shaders_compiled_dir:
	mkdir -p $(SHADERS_COMPILED_DIR)

# TripleT_Engine Library Creation for X11 
install_x11:
	mkdir -p $(INSTALL_INC_DIR)
	cp -r $(INCLUDE_DIR)*.h $(INCLUDE_DIR)*.mk $(INSTALL_INC_DIR)
	cp $(LIB_DIR)*.a $(INSTALL_LIB_DIR)

x11: $(UI_X11_OBJ_FILES) $(GRAPHICS_X11_OBJ_FILES)
	mkdir -p $(LIB_DIR)
	$(LIB_CMD) $(LIB_OP_FLAG) $(LIB_DIR)lib$(LIB_NAME)_X11.a $(UI_X11_OBJ_FILES) $(GRAPHICS_X11_OBJ_FILES)

$(OBJ_DIR)%.o: $(SRC_UI_X11_DIR)%.c | create_object_dir
	$(CC) $(COMPILE_FLAGS) -c -o $@ $< 

$(OBJ_DIR)%.o: $(SRC_GRAPHICS_X11_DIR)%.c $(SHADERS_HEADER_FILES) | create_object_dir
	$(CC) $(COMPILE_FLAGS) -c -o $@ $< 

$(SHADERS_HEADER_DIR)%.h: $(SHADERS_COMPILED_DIR)%.spv $(SPV_TO_HEADER_TOOL)
	$(SPV_TO_HEADER_TOOL) $<

$(SHADERS_COMPILED_DIR)%.spv: $(SHADERS_DIR)%.vert | create_shaders_compiled_dir
	$(GLSLC) $< -o $@

$(SHADERS_COMPILED_DIR)%.spv: $(SHADERS_DIR)%.frag | create_shaders_compiled_dir
	$(GLSLC) $< -o $@

$(SPV_TO_HEADER_TOOL): $(TOOLS_C_FILES)
	$(MAKE) -C $(TOOLS_DIR)


# TripleT_Engine Library Creation for Wayland
install_wayland:
	@echo "Not yet implemented"

wayland: 
	@echo "Not yet implemented"


# Cleanup
clean:
	rm -rf $(LIB_DIR) $(OBJ_DIR) $(SHADERS_COMPILED_DIR) $(SHADERS_HEADER_FILES)
	$(MAKE) -C $(TOOLS_DIR) clean

uninstall_x11:
	rm -rf $(INSTALL_INC_DIR) $(INSTALL_LIB_DIR)lib$(LIB_NAME)_X11.a
