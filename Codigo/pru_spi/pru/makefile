SRC    := pru_ads.p
TARGET := $(SRC:.p=.bin)

all: $(TARGET)

$(TARGET): $(SRC)
	pasm -b $(SRC)

clean:
	rm $(TARGET)