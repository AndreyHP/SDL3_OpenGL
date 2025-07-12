#Variables
TARGET := main
OUTDIR := debug

IMGUI_DIR := ./include/imgui
SRC_DIR := ./src

DEP := -MP -MD

#Linux =========================================================================
INCLUDE := -I./include -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXX := g++
CXXFLAGS := -std=c++23 -O0 $(DEP)
LINKER := -lGL -lX11 -lpthread -lXrandr -lXi -l:libSDL3.a
#================================================================================

#Windows =========================================================================
#INCLUDE := -I./include -I/usr/local/include -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
#CXX = x86_64-w64-mingw32-g++
#CXXFLAGS = -std=c++23 -O3 -static-libgcc -static-libstdc++ -Wl,-subsystem,windows
#LINKER = -L./lib -lSDL3 -lopengl32
#================================================================================

#HTML5 ==========================================================================
#INCLUDE := -I./include -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
#CXX = em++
#CXXFLAGS := -std=c++23 -O0 -DGLSL_ES -DPLATFORM_WEB
#LINKER =  -sUSE_SDL=3 -s MAX_WEBGL_VERSION=2 --preload-file ./assets --preload-file ./glsl
#================================================================================

#-sASYNCIFY

# -g   Valgrind
# Add -ggdb to the command line for debug builds and -O2 -DNDEBUG
# -pedantic-errors disable compiler extensions
# -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion Extra warnings
# -Werror Treat warnings as errors


IMGUI_SRC := \
    $(IMGUI_DIR)/imgui.cpp \
    $(IMGUI_DIR)/imgui_demo.cpp \
    $(IMGUI_DIR)/imgui_draw.cpp \
    $(IMGUI_DIR)/imgui_tables.cpp \
    $(IMGUI_DIR)/imgui_widgets.cpp \
    $(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp \
    $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

SRC := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c) $(IMGUI_SRC)
OBJ := $(patsubst %.cpp,$(OUTDIR)/%.o,$(patsubst %.c,$(OUTDIR)/%.o,$(SRC)))
DEPFILES := $(patsubst %.cpp,$(OUTDIR)/%.d,$(patsubst %.c,$(OUTDIR)/%.d,$(SRC)))

# Default target
all: $(OUTDIR)/$(TARGET)

# Link
$(OUTDIR)/$(TARGET): $(OBJ) | $(OUTDIR)
	$(info Linking: $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^ $(LINKER)

# Compile source files to object files
$(OUTDIR)/%.o: %.cpp | $(OUTDIR)
	@mkdir -p $(@D)
	$(info Compiling: $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(OUTDIR)/%.o: %.c | $(OUTDIR)
	@mkdir -p $(@D)
	$(info Compiling: $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# Ensure the output directory exists
$(OUTDIR):
	@mkdir -p $(OUTDIR)

# Run program
run: all
	./$(OUTDIR)/$(TARGET)

# Clean target
clean:
	@rm -rf $(OUTDIR)


diff:
	$(info The status of the repository, and the volume of per-file changes:)
	@git status
	@git diff --stat

-include $(DEPFILES)

.PHONY: all clean run
