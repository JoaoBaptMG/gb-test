# Utility function
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# Tools
TOOL_SRCS := $(call rwildcard, tools/, *.cpp)
TOOL_BIN := tools.exe

# Resources
SPRITE_FILES := $(call rwildcard, data/sprites/, *.png)
SPRITE_OUT := $(SPRITE_FILES:data/sprites/%.png=odata/sprites/%.z80)
TILESET_FILES := $(call rwildcard, data/tilesets/, *.json)
TILESET_OUT := $(TILESET_FILES:data/tilesets/%.json=odata/tilesets/%.z80)
MAP_PREFILES := $(wildcard tiled-maps/*.tmx)
MAP_FILES := $(MAP_PREFILES:tiled-maps/%.tmx=data/maps/%.json)
MAP_OUT := $(MAP_FILES:data/maps/%.json=odata/maps/%.z80)
RESOURCE_FILES := $(SPRITE_OUT) $(TILESET_OUT) $(MAP_OUT)

# Compilation
SRC_FILES := $(call rwildcard, src/, *.z80)
OBJ_FILES := $(addprefix obj/, $(SRC_FILES:src/%.z80=%.o))
DEPEND_FILES := $(addsuffix .d, $(OBJ_FILES))
ROBJ_FILES = $(addprefix robj/, $(RESOURCE_FILES:odata/%.z80=%.o))
ASM_FLAGS := -i src/

# paths (remember to replace those paths)
GBEMU := C:/Users/jbapt/Desktop/bgb/bgb64.exe
TILED := C:/Users/jbapt/source/repos/build-tiled-Desktop_Qt_5_12_3_MSVC2017_64bit-Release/Release/install-root/tiled.exe

# Keep all intermediary files (maps, assembly and such) around
.SECONDARY:

# Actual rules
.PHONY: all clean run

all: bin/game.gb

bin/game.gb: $(OBJ_FILES) $(ROBJ_FILES)
	@mkdir -p bin
	rgblink -d -m bin/game.map -n bin/game.sym -o bin/game.gb -l link.lnk $(OBJ_FILES) $(ROBJ_FILES)
	rgbfix -p0 -v bin/game.gb

-include $(DEPEND_FILES)

obj/%.o: src/%.z80 $(OBJ_DIRS)
	@mkdir -p $(@D)
	rgbasm $(ASM_FLAGS) -M $@.d -o $@ $<

robj/%.o: odata/%.z80 $(ROBJ_DIRS)
	@mkdir -p $(@D)
	rgbasm $(ASM_FLAGS) -o $@ $<

odata/sprites/%.z80: data/sprites/%.png tools/tools.exe
	@mkdir -p $(@D)
	tools/tools.exe sprite-export $< $@

odata/tilesets/%.z80: data/tilesets/%.json tools/tools.exe
	@mkdir -p $(@D)
	tools/tools.exe tileset-export $< $@

data/maps/%.json: tiled-maps/%.tmx
	@mkdir -p $(@D)
	$(TILED) --export-map json $< $@

odata/maps/%.z80: data/maps/%.json tools/tools.exe
	@mkdir -p $(@D)
	tools/tools.exe map-export $< $@

tools/tools.exe: $(TOOL_SRCS)
	g++ -std=c++14 -Wall -O0 -g -o $@ $^

clean:
	rm -rf bin obj robj odata data/maps

run: bin/game.gb
	$(GBEMU) bin/game.gb &