# assignment-sensor

**Descripción**
Programa en C que muestrea un "mock sensor" (por defecto `/dev/urandom`) a intervalos configurables y escribe líneas `ISO8601_TIMESTAMP | VALUE` en un fichero de log bajo `/tmp` (con fallback a `/var/tmp` si es necesario). Empaquetado como un servicio `systemd`.

---

## Estructura del repositorio
├─ src/ # código fuente (assignment_sensor.c)
├─ systemd/
│ └─ assignment-sensor.service # archivo unit de systemd
├─ tests/ # scripts y manual de pruebas
├─ ai/ # evidencia de uso de IA (prompt-log, reflection, provenance)
├─ build/ # artefacto generado por make
├─ Makefile
└─ README.md

---

## Requisitos previos
- Sistema Linux con `systemd`.
- `gcc` (o toolchain C equivalente) y `make`.
- Permisos `sudo` para instalar el servicio a nivel sistema.
- (Opcional) `gh` (GitHub CLI) para crear repositorios desde la terminal.

---

## Clonar (si el repo ya existe remotamente)
```bash
# público
git clone https://github.com/KaonVeil/assignment-sensor.git

# o SSH
git clone git@github.com:KaonVeil/assignment-sensor.git

cd assignment-sensor

## Construir (one-command)

make
# Artefacto resultante: build/assignment-sensor

---

## Instalar (instalación system-wide)

sudo make install
### Opcional: recargar systemd, habilitar y arrancar
sudo systemctl daemon-reload
sudo systemctl enable --now assignment-sensor.service


Nota: make install copia el binario a /usr/local/bin/assignment-sensor y la unidad a /etc/systemd/system/assignment-sensor.service.


Configuración y Flags

El binario acepta:

--interval <seconds> o -i <seconds>: intervalo de muestreo (por defecto: 5).

--logfile <path> o -l <path>: ruta completa del archivo de log.

--device <path> o -d <path>: dispositivo a muestrear (por defecto: /dev/urandom).

Ejemplo de ejecución manual:

/usr/local/bin/assignment-sensor --interval 2 --logfile /tmp/assignment_sensor.log

---

## Check status

sudo systemctl status assignment-sensor.service
tail -n 10 /tmp/assignment_sensor.log
# If not writable, check /var/tmp/assignment_sensor.log

---

## Uninstall

sudo systemctl disable --now assignment-sensor.service
sudo rm /etc/systemd/system/assignment-sensor.service
sudo rm /usr/local/bin/assignment-sensor
sudo systemctl daemon-reload
