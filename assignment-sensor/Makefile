CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra
SRC = src/assignment_sensor.c
BIN = build/assignment-sensor

all: $(BIN)

$(BIN): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $(SRC)

build:
	mkdir -p build

clean:
	rm -rf build

install: all
	install -d /usr/local/bin
	install -m 0755 $(BIN) /usr/local/bin/assignment-sensor
	install -d /etc/systemd/system
	install -m 0644 systemd/assignment-sensor.service /etc/systemd/system/assignment-sensor.service
	@echo "Instalado. Ahora ejecutar: sudo systemctl daemon-reload && sudo systemctl enable --now assignment-sensor.service"

uninstall:
	systemctl disable --now assignment-sensor.service || true
	rm -f /usr/local/bin/assignment-sensor
	rm -f /etc/systemd/system/assignment-sensor.service
	systemctl daemon-reload || true

