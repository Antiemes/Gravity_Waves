CC=avr-gcc
OBJCOPY=avr-objcopy

CFLAGS=-Os -DF_CPU=16000000UL -mmcu=atmega16u2 -lm -Wl,-Map=main.map,--cref -Wl,--gc-sections -Wl,--relax
PORT=/dev/ttyACM0

main.hex: main.elf
	${OBJCOPY} -O ihex -R .eeprom main.elf main.hex

main.elf: main.o
	${CC} -o main.elf main.o -lm -Wl,-Map=main.map,--cref -Wl,--gc-sections -Wl,--relax -mmcu=atmega16u2

clean:
	rm *.o main.elf main.hex

flash: main.hex
	dfu-programmer atmega16u2 erase || true
	dfu-programmer atmega16u2 flash main.hex
	dfu-programmer atmega16u2 reset
	
