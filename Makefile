# Variables
IMGUI_DIR = ./include/imgui
MAIN += $(wildcard ./src/*cpp) $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp ./src/*.c
MAIN += $(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
TARGET = main
OUTDIR = release
#INCLUDE = -I:./include -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
INCLUDE = -I:./include -I/usr/local/include -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
#CXX = g++
CXX = x86_64-w64-mingw32-g++
#CXXFLAGS = -std=c++23  -O0
CXXFLAGS = -std=c++23 -O3 -static-libgcc -static-libstdc++ -Wl,-subsystem,windows
#LINKER = -lGL -lX11 -lpthread -lXrandr -lXi -l:libSDL3.a
LINKER = -L./lib -lSDL3 -lopengl32

# -g   Valgrind
# Add -ggdb to the command line for debug builds and -O2 -DNDEBUG
# -pedantic-errors disable compiler extensions 
# -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion Extra warnings
# -Werror Treat warnings as errors

# Default target
all: $(OUTDIR)/$(TARGET)

# Rule to compile and link the executable
$(OUTDIR)/$(TARGET): $(MAIN) | $(OUTDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(MAIN) -o $(OUTDIR)/$(TARGET) $(LINKER)

# Ensure the output directory exists
$(OUTDIR):
	mkdir -p $(OUTDIR)

# Run program
run:
	./$(OUTDIR)/$(TARGET)

# Clean target
clean:
	rm -rf $(OUTDIR)

.PHONY: all clean
