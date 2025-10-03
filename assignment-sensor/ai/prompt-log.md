# ai/prompt-log.md
Registro cronológico resumido de prompts usados con el asistente AI durante el desarrollo.
Se documentan las preguntas (prompts) que hice y un resumen breve y claro de la respuesta que recibí y cómo la usé.

---

2025-09-26 — Prompt 01
**Prompt:** "Dame un diseño simple para un programa en C que lea un 'mock sensor' y escriba timestamp + valor cada N segundos. Requisitos: manejo SIGTERM, fallback logfile, CLI."
**Resumen de la respuesta:** Esquema de alto nivel: parseo de flags, abrir logfile, abrir dispositivo (ej. /dev/urandom), loop de muestreo, manejo SIGTERM, fallback a /var/tmp y PRNG. Acepté la arquitectura general.

2025-09-26 — Prompt 02
**Prompt:** "Sugiéreme la forma más segura de manejar señales (SIGTERM) en C."
**Resumen:** Recomendó `sigaction` y uso de variable `sig_atomic_t` para la bandera de terminación. Implementé `sigaction` tal cual.

2025-09-26 — Prompt 03
**Prompt:** "Cómo obtener timestamp ISO8601 en UTC con ms en C?"
**Resumen:** Propuso `clock_gettime` + `gmtime_r` + `snprintf` para formateo. Implementado sin cambios.

2025-09-27 — Prompt 04
**Prompt:** "¿Mejor /dev/random o /dev/urandom para mock sensor? Justifica."
**Resumen:** Explicó diferencias; aconsejó `/dev/urandom` por no bloquear y por disponibilidad. Elegí `/dev/urandom` y documenté la elección.

2025-09-27 — Prompt 05
**Prompt:** "Cómo asegurar que no se escriban líneas parciales al loguear?"
**Resumen:** Sugerencia: `setvbuf(logf, NULL, _IOLBF, 0)` (line buffering) o `fflush` tras cada fprintf. Implementé `setvbuf` y llamadas puntuales a `fflush`.

2025-09-27 — Prompt 06
**Prompt:** "Dame un ejemplo simple de parseo de argumentos (interval, logfile, device) en C."
**Resumen:** Ejemplo con `getopt`/`getopt_long`. Usé `getopt` para mantenerlo corto y portable.

2025-09-28 — Prompt 07
**Prompt:** "Cómo debo comportarme si el usuario pasa --device /dev/fake0?"
**Resumen:** Recomendó fallo inmediato con código no-cero y mensaje en stderr. Implementé esa lógica (fallo rápido) y la documenté.

2025-09-28 — Prompt 08
**Prompt:** "Si /tmp no es escribible, ¿qué fallback usar y cómo detectarlo?"
**Resumen:** Propuesta: intentar `fopen("/tmp/...","a")`, si falla, usar `/var/tmp/...` y avisar por stderr. Implementado.

2025-09-28 — Prompt 09
**Prompt:** "¿Cómo evitar busy-waiting durante el sleep y a la vez reaccionar rápido a SIGTERM?"
**Resumen:** Sugerió dormir por segundos (sleep(1) en bucle) o `nanosleep` con chequeo de bandera. Elegí `sleep(1)` en un bucle para simplicidad.

2025-09-28 — Prompt 10
**Prompt:** "Dame un Makefile simple para compilar e instalar el binario y unit systemd."
**Resumen:** Plantilla de Makefile con targets `all`, `clean`, `install`, `uninstall`. Integré install directo a `/usr/local/bin` y copia del unit a `/etc/systemd/system`.

2025-09-29 — Prompt 11
**Prompt:** "Cómo escribir un unit de systemd que arranque en multi-user.target y reinicie en fallo?"
**Resumen:** Unit propuesto con `After=multi-user.target`, `Restart=on-failure`, `RestartSec=2`. Lo usé con poco cambio.

2025-09-29 — Prompt 12
**Prompt:** "Qué permisos y User= debo usar si el servicio corre como no-root?"
**Resumen:** Recomienda crear usuario system (`useradd --system`) y ajustar permisos del logfile; documenté ambos pasos en README.

2025-09-29 — Prompt 13
**Prompt:** "Sugerencias para pruebas manuales (happy path, fallback, SIGTERM, failure path)."
**Resumen:** Lista de pruebas paso a paso, comandos `systemctl`, `tail`, `journalctl` y expectativas. Añadí estas pruebas en `tests/test_manual.md`.

2025-09-30 — Prompt 14
**Prompt:** "Qué exit codes usar para diferenciar fallos (logfile, dispositivo, etc.)?"
**Resumen:** Propuso códigos distintos para facilitar testing (p.ej. 2 = fallo logdir, 3 = fopen log, 4 = device inválido). Aceptado tal cual.

2025-09-30 — Prompt 15
**Prompt:** "Cómo documentar el uso de IA en ai/prompt-log, reflection y provenance para la entrega?"
**Resumen:** Plantilla de estructura (registro, reflexión ≤500 palabras, json de procedencia). Usé esa plantilla y la adapté.

2025-10-01 — Prompt 16
**Prompt:** "Convénceme con 3 líneas por qué usar PRNG fallback cuando /dev/urandom falla."
**Resumen:** Respuesta corta y práctica: disponibilidad, continuidad de servicio y testabilidad. Incluí la justificación en README.

2025-10-01 — Prompt 17
**Prompt:** "¿Cuál es la forma más clara de explicar en README el comportamiento cuando /tmp no es escribible?"
**Resumen:** Texto breve con comandos para reproducir fallback y ejemplos de salida. Añadido al README.

