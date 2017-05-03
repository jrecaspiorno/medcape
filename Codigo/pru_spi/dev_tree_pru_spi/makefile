TARGET  = BB-PRU-SPI-E1

all: $(TARGET)

$(TARGET): $(OBJS)
	dtc -O dtb -I dts -o /lib/firmware/$(TARGET)-00A0.dtbo -b 0 -@ $(TARGET).dts 



