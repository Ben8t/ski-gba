TARGET  := ski
BUILDDIR := build

CC      := arm-none-eabi-gcc
AS      := arm-none-eabi-as
LD      := arm-none-eabi-ld
OBJCOPY := arm-none-eabi-objcopy

CFLAGS  := -mcpu=arm7tdmi -mthumb-interwork -mthumb \
           -O2 -Wall -Wextra \
           -fno-strict-aliasing \
           -nostdlib -nostartfiles \
           -Isrc

ASFLAGS := -mcpu=arm7tdmi

SRCS    := $(wildcard src/*.c)
OBJS    := $(BUILDDIR)/crt0.o $(patsubst src/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(BUILDDIR)/$(TARGET).gba

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/crt0.o: crt0.s | $(BUILDDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILDDIR)/%.o: src/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

LIBGCC := $(shell arm-none-eabi-gcc -mcpu=arm7tdmi -mthumb -print-libgcc-file-name)

$(BUILDDIR)/$(TARGET).elf: $(OBJS)
	$(LD) -T gba.ld $^ $(LIBGCC) -o $@

$(BUILDDIR)/$(TARGET).gba: $(BUILDDIR)/$(TARGET).elf
	$(OBJCOPY) -O binary -R .bss $< $@
	@echo "Built: $@"
	@ls -lh $@

clean:
	rm -rf $(BUILDDIR)

run: all
	mgba-qt $(BUILDDIR)/$(TARGET).gba &
