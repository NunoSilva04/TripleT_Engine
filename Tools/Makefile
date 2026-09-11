CC = gcc
COMPILE_FLAGS = -Wall -Wextra 

.DEFAULT_GOAL:= spv_to_header

spv_to_header: SPV_To_Header.c
	$(CC) $(COMPILE_FLAGS) -o $@ $<

clean:
	rm -f spv_to_header
