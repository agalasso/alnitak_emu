LDFLAGS =
INCLUDES := -I$(HOME)/Incoming/hidapi-0.7.0/hidapi/
LIBS := $(HOME)/Incoming/hidapi-0.7.0/windows/Debug/hidapi.lib

all: emu

clean:
	rm -f *.o emu

emu: emu.o serialport.o usbdimmer.o
	g++ $(LDFLAGS) -o $@ $^ $(LIBS)

emu.o: emu.cpp
	g++ -c -o $@ $<

serialport.o: serialport.cpp
	g++ -c -o $@ $<

usbdimmer.o: usbdimmer.cpp
	g++ $(INCLUDES) -c -o $@ $<

