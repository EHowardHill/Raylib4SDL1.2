# =============================================================================
#  raylib-sdl12 — Makefile
#
#  Adjust SDL_CFLAGS / SDL_LIBS for your retro toolchain.
#  On a modern Linux with SDL 1.2 dev packages installed:
#      sudo apt install libsdl1.2-dev libsdl-ttf2.0-dev libsdl-mixer1.2-dev
#
# =============================================================================

CXX       ?= g++
AR        ?= ar
CXXFLAGS  := -Wall -Wextra -O2 -Iheaders

# SDL 1.2 flags (use sdl-config if available)
SDL_CFLAGS  := $(shell sdl-config --cflags 2>/dev/null || echo "-I/usr/include/SDL")
SDL_LIBS    := $(shell sdl-config --libs   2>/dev/null || echo "-lSDL") -lSDL_ttf -lSDL_mixer -lSDL_image

CXXFLAGS += $(SDL_CFLAGS)

SRCDIR    := src
OBJDIR    := obj

SOURCES   := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS   := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

LIBNAME   := libraylib-sdl12.a

# --- Targets ---

.PHONY: all clean example

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $@ $^
	@echo "==> Built $(LIBNAME)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(LIBNAME) example_app

# --- Example build (assumes example.cpp in project root) ---

example: $(LIBNAME)
	$(CXX) $(CXXFLAGS) example.cpp -o example_app -L. -lraylib-sdl12 $(SDL_LIBS) -lm
	@echo "==> Built example_app"
