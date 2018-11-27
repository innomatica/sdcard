# gcc toolchain
export PATH:=/opt/gcc-arm-none-eabi/bin:$(PATH)
#.DEFAULT_GOAL := all
# terminal
TERM = mate-terminal

# additional source files
C_SOURCES += \
			 board.c \
			 SDCard.c

# additional header folder

# additional rules
clobber:
	-rm -fR .dep $(BUILD_DIR)
	-rm -f tags

ctags:
	-rm -f tags
	-ctags -R . 

astyle:
	-find ./Src -name *.c | xargs astyle --style=allman --indent=force-tab
	-find ./Src -name *.orig | xargs rm -f
	-find ./Inc -name *.h | xargs astyle --style=allman --indent=force-tab
	-find ./Inc -name *.orig | xargs rm -f

# flash with STLINK 
sflash:
	openocd \
		-f interface/stlink-v2.cfg \
		-f ./stm32l0.cfg \
		-c 'program $(BUILD_DIR)/$(TARGET).elf reset exit'

sflashv:
	openocd \
		-f interface/stlink-v2.cfg  \
		-f ./stm32l0.cfg \
		-c 'program $(BUILD_DIR)/$(TARGET).elf verify reset exit'

# flash with J-Link
jflash:
	JLinkExe -if swd -device stm32l072cb -speed 4000 -commandfile jlinkcmd > /dev/null

jflashv:
	JLinkExe -if swd -device stm32l072cb -speed 4000 -commandfile jlinkcmd

# flash with FTDI
lflash:
	openocd \
		-f interface/ftdi/luminary-icdi.cfg \
		-c "transport select swd" \
		-f ./stm32l0.cfg \
		-c 'program $(BUILD_DIR)/$(TARGET).elf reset exit'

lflashv:
	openocd \
		-f interface/ftdi/luminary-icdi.cfg \
		-c "transport select swd" \
		-f ./stm32l0.cfg \
		-c 'program $(BUILD_DIR)/$(TARGET).elf verify reset exit'

oocd:
	openocd \
		-f interface/jlink -f target/stm32l0.cfg

gdbs:
	$(TERM) -e '/opt/SEGGER/JLink/JLinkGDBServerCLExe -device STM32L072CB -if SWD -speed 4000' &
	#/opt/SEGGER/JLink/JLinkGDBServerExe -device STM32L072CB -if SWD -speed 4000 &

gdb:
	$(TERM) -e /opt/gcc-arm-none-eabi/bin/arm-none-eabi-gdb &

cgdb:
	$(TERM) -e 'cgdb -d /opt/gcc-arm-none-eabi/bin/arm-none-eabi-gdb' &

debug: gdbs gdb
