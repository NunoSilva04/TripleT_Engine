CC = gcc
LIB_CMD = ar

COMPILE_FLAGS = -Wall -Wextra -g 
LIB_OP_FLAG = rcs

# Installation Directories
INSTALL_LIB_DIR = /usr/local/lib/
INSTALL_INC_DIR = /usr/local/include/$(LIB_NAME)

# Library and Objects Directories
LIB_NAME = TripleT_UI
LIB_DIR = Lib/
OBJ_DIR = Objects/
INCLUDE_DIR = Include/

# X11 
X11 = X11
SRC_X11_DIR = Src/X11/
X11_C_FILES = $(wildcard $(SRC_X11_DIR)*.c)
X11_OBJ_FILES = $(patsubst $(SRC_X11_DIR)%.c, $(OBJ_DIR)%.o, $(X11_C_FILES))

# Help
.DEFAULT_GOAL:= help
help:
	@echo "Choose what type of configuration you want:"
	@echo "'make x11' to build for X11, followed by sudo make install_x11"
	@echo "'make wayland' to build for Wayland, followed by sudo make install_wayland"

create_object_dir: 
	mkdir -p $(OBJ_DIR)


# X11 Library Creation
install_x11:
	mkdir -p $(INSTALL_INC_DIR)
	cp -r $(INCLUDE_DIR)* $(INSTALL_INC_DIR)
	cp $(LIB_DIR)*.a $(INSTALL_LIB_DIR)

x11: $(X11_OBJ_FILES) 
	mkdir -p $(LIB_DIR)
	$(LIB_CMD) $(LIB_OP_FLAG) $(LIB_DIR)lib$(LIB_NAME)_$(X11).a $(X11_OBJ_FILES)

$(OBJ_DIR)%.o: $(SRC_X11_DIR)%.c | create_object_dir
	$(CC) $(COMPILE_FLAGS) -c -o $@ $^ 

# Wayland Library Creation
install_wayland:
	@echo "Not yet implemented"

wayland: 
	@echo "Not yet implemented"

# Cleanup
clean:
	rm -rf $(LIB_DIR) $(OBJ_DIR)

uninstall_x11:
	rm -rf $(INSTALL_INC_DIR) $(INSTALL_LIB_DIR)lib$(LIB_NAME)_$(X11).a
