NEO_SRC = $(wildcard src/*.cpp)

CC = g++

COMPILER_FLAGS = -Werror -Wfloat-conversion -ggdb -g -O3 -finline-functions

NEO_EXEC_NAME = Neo

all : neo

neo: $(NEO_SRC) 
	 $(CC) $(NEO_SRC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $(NEO_EXEC_NAME) 