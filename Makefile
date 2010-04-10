TARGET=main
SOURCE=main.c     \
       rcc.c      \
       $(STM_LIB_SRC)

#srcdir=.
#VPATH=$(srcdir)/lib/fwlib/src $(srcdir)

HEADER=$(wildcard *.h)
OBJ=$(SOURCE:=.o)

GCC_PATH=
GCC_PREFIX=$(GCC_PATH)arm-none-eabi-

FIND=find
CC=$(GCC_PREFIX)gcc
AS=$(GCC_PREFIX)gcc
LD=$(GCC_PREFIX)gcc
OBJCOPY=$(GCC_PREFIX)objcopy

.SUFFIXES:


INCLUDES=-I$(srcdir) -I$(srcdir)/lib/fwlib/inc -I$(srcdir)/lib
LD_INC=-L$(srcdir)/lib -L$(srcdir)/ld

# When changing boards, modify STMPROC, HSE_VALUE, and the
#  startup asm code.

STMPROC=STM32F10X_HD
HSE_VALUE=8000000
STM_LIB_SRC= $(srcdir)/lib/startup/gcc_ride7/startup_stm32f10x_hd.s \
             $(srcdir)/lib/fwlib/src/stm32f10x_gpio.c               \
             $(srcdir)/lib/fwlib/src/stm32f10x_usart.c

ALL_CFLAGS=-MD -D$(STMPROC) -DHSE_VALUE=$(HSE_VALUE) \
           -mthumb -mcpu=cortex-m3 -Wall -g\
	   -Wno-main \
            $(INCLUDES) $(CFLAGS)
ALL_LDFLAGS=$(ALL_CFLAGS)\
            -Wl,--gc-sections,-Map=$@.map,-cref,-u,Reset_Handler \
            $(LD_INC) -T STM32F103VD.ld

ALL_ASFLAGS=$(ALL_CFLAGS)


.SECONDARY:

all: $(TARGET).hex $(TARGET).bin

%.s.o: %.s $(HEADER)
	$(AS) $(ALL_ASFLAGS) -c -o $@ $<

%.c.o: %.c $(HEADER)
	$(CC) $(ALL_CFLAGS) -c -o $@ $<

%.elf: $(OBJ)
	$(LD) $(ALL_LDFLAGS) -o $@ $^

%.hex: %.elf
	$(OBJCOPY) -S -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -S -O bin $< $@
	
clean:
	@$(FIND) . -regex '.*\.\([od]\|elf\|hex\|bin\|map\)'\
		-printf 'RM %P\n' -delete


