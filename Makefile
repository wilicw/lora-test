PWD:=$(shell pwd)
DEVICE:=$(shell ls /dev/tty*usb* | head -n 1)
TARGET:=esp32c3
SERIAL_MONITOR:=pyserial-miniterm

all: ./main
	idf.py build

flash:
	idf.py flash

monitor:
	$(SERIAL_MONITOR) $(DEVICE) 115200

clean:
	rm -rf build/

.PHONY: flash monitor clean
