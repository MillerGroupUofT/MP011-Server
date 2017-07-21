
# Define compiler
CC = g++

# Define compile-time options
CFLAGS = -Wall

# Define library paths to be included (-L\..)
LFLAGS = 

# Define libraries to link into executable
LIBS = -lwsock32 -lws2_32 -static-libgcc -static-libstdc++

# Define header, library, and object directories
IDIR = .\include
LDIR = .\lib
ODIR = .\obj
SDIR = .\src

# Define the executable file
MAIN = server.exe

# Define source files
SRCS = server_main.cpp server_functions.cpp server_commands.cpp
#SRCS = $(patsubst %,$(SDIR)\%,$(_SRCS))

# Define header files
DEPS = server_functions.h server_commands.h command_def.h 
#DEPS = $(patsubst %,$(IDIR)\%,$(_DEPS))

# Define object files
OBJS = $(SRCS:.cpp=.o)
#OBJS = $(patsubst %,$(ODIR)\%,$(_OBJ))


all: $(MAIN)

# Compile source codes into objects
%.o: %.cpp $(DEPS)
	$(CC) -c $(CFLAGS) $< -o $@ 

# Compile objects into executable
$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(MAIN) $(LFLAGS) $(LIBS)

# Clean up objects
.PHONY: clean

clean:
	@echo "Cleaning up..."
	rm -f *.o
	rm -f *~
