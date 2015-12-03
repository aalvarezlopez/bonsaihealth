TARGET = bonsai
VERSION_MINOR = 1
VERSION_MAJOR= 0
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR)
C_FILES := $(wildcard src/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
LD_FLAGS := -mcpu=24FJ64GB106 -omf=elf\
	-Wl,--defsym=__MPLAB_BUILD=1,,--script=p24FJ64GB106.gld\
	-Wl,--stack=16,--check-sections,--data-init,--pack-data\
	-Wl,--handles,--isr,--no-gc-sections,--fill-upper=0,\
	-Wl,--stackguard=16,--no-force-link,--smart-io,\
	-Wl,-Map="out/$(TARGET).map",--report-me
CC_FLAGS := -mcpu=24FJ64GB106 \
	-g -omf=elf -O0 -msmart-io=1 -Wall -msfr-warn=off -std=gnu99
INCLUDE := -I ./include -I ./lib/inc
CC = /opt/microchip/xc16/v1.11/bin/xc16-gcc
BIN2HEX = "/opt/microchip/xc16/v1.11/bin"/xc16-bin2hex 
GIT_HASH := $(shell git rev-parse --short HEAD)


LIB_FILES := $(wildcard lib/src/*.c)
LIB_OBJ := $(addprefix lib/obj/,$(notdir $(LIB_FILES:.c=.o)))
LIB_INCLUDES := -I ./lib/inc -I ./include

default: $(TARGET) $(LIB_OBJ)
.PHONY: program clean

$(TARGET): $(OBJ_FILES) $(LIB_OBJ)
	@mkdir -p out
	@$(CC)   -o out/$(TARGET).elf  $^ $(LD_FLAGS)
	@$(BIN2HEX) "out/$(TARGET).elf" -a  -omf=elf 

lib/obj/%.o: lib/src/%.c
	@mkdir -p lib/obj
	$(CC) -c -o $@ $< $(CC_FLAGS) $(LIB_INCLUDES)\
		-DHASH=$(GIT_HASH) -DVERSION=$(VERSION)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) -c -o $@ $< $(CC_FLAGS) $(INCLUDE)\
		-DHASH=$(GIT_HASH) -DVERSION=$(VERSION)

program: $(TARGET)
	@pk2cmd -P -M -F./out/$(TARGET).hex -T -R

clean:
	@rm ./obj/*.o -f
	@rm ./lib/obj/*.o -f
	@rm ./out/$(TARGET).elf -f
	@rm ./out/$(TARGET).hex -f
