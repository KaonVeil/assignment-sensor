#define _POSIX_C_SOURCE 200809L
#include <stdio.h>      // fopen, fprintf, setvbuf, FILE
#include <stdlib.h>     // srand, rand, atoi, EXIT_* macros
#include <stdint.h>     // uint8_t
#include <unistd.h>     // read, close, sleep, getpid
#include <fcntl.h>      // open, O_RDONLY, O_CLOEXEC
#include <string.h>     // strerror, strcmp, strncpy
#include <time.h>       // clock_gettime, struct timespec, gmtime_r
#include <signal.h>     // sigaction, SIGTERM, SIGINT
#include <errno.h>      // errno
#include <limits.h>     // PATH_MAX

/* ---------------------------
   - usamos open/read para leer bytes del "dispositivo" (p. ej. /dev/urandom).
   - usamos fopen + setvbuf(line-buffered) para evitar líneas parciales en el log.
   - usamos sigaction en vez de signal() por seguridad reentrante y comportamiento POSIX.
   - usamos clock_gettime + gmtime_r para obtener tiempo UTC con ms.
   - fallback: si /tmp no escribible -> /var/tmp; si device por defecto falla -> PRNG.
   --------------------------- */

static volatile sig_atomic_t stop_requested = 0;

/* Manejador simple de señal: marca que se pidió detener */
static void handle_signal(int sig) {
    (void)sig;
    stop_requested = 1;
}

/* Formatea tiempo UTC en ISO8601 con milisegundos en buf (buf >= 32 recomendable) */
static void iso8601_utc(char *buf, size_t bufsz) {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        // en caso extraño, escribir algo simple
        snprintf(buf, bufsz, "1970-01-01T00:00:00.000Z");
        return;
    }
    struct tm tm;
    gmtime_r(&ts.tv_sec, &tm); // thread-safe
    int ms = (int)(ts.tv_nsec / 1000000);
    // YYYY-MM-DDTHH:MM:SS.mmmZ
    snprintf(buf, bufsz, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
}

/* Función principal: parsea args, abre logfile, abre device (o PRNG), loop de muestreo */
int main(int argc, char **argv) {
    const char *default_device = "/dev/urandom";
    const char *log_basename = "assignment_sensor.log";
    const char *tmp_dir = "/tmp";      // preferido
    const char *var_tmp_dir = "/var/tmp"; // fallback

    unsigned int interval = 5; // segundos por defecto
    const char *device = default_device;
    char logfile_path[PATH_MAX];
    logfile_path[0] = '\0';

    /* parseo simple de argumentos:
       -i <segundos>  : intervalo
       -l <ruta>      : logfile completo
       -d <dispositivo>: device path
    */
    int opt;
    while ((opt = getopt(argc, argv, "i:l:d:")) != -1) {
        switch (opt) {
            case 'i':
                interval = (unsigned int)atoi(optarg);
                if (interval == 0) interval = 1;
                break;
            case 'l':
                strncpy(logfile_path, optarg, sizeof(logfile_path)-1);
                logfile_path[sizeof(logfile_path)-1] = '\0';
                break;
            case 'd':
                device = optarg;
                break;
            default:
                fprintf(stderr, "Uso: %s [-i segundos] [-l /ruta/log] [-d /dev/xxx]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    /* Determinar ruta de logfile: si no se pasó, intentar /tmp, si falla /var/tmp */
    if (logfile_path[0] == '\0') {
        snprintf(logfile_path, sizeof(logfile_path), "%s/%s", tmp_dir, log_basename);
        FILE *t = fopen(logfile_path, "a");
        if (!t) {
            // intento fallback
            snprintf(logfile_path, sizeof(logfile_path), "%s/%s", var_tmp_dir, log_basename);
            t = fopen(logfile_path, "a");
            if (!t) {
                fprintf(stderr, "Error fatal: no se puede abrir log en %s ni en %s: %s\n",
                        tmp_dir, var_tmp_dir, strerror(errno));
                return 2;
            } else {
                fprintf(stderr, "Aviso: /tmp no escribible; usando %s\n", logfile_path);
            }
        }
        fclose(t);
    }

    /* Instalamos los manejadores de señal con sigaction (más seguro que signal()) */
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    /* Abrir fichero de log en modo append y asegurar line buffering */
    FILE *logf = fopen(logfile_path, "a");
    if (!logf) {
        fprintf(stderr, "Error fatal: no se puede abrir logfile %s: %s\n", logfile_path, strerror(errno));
        return 3;
    }
    /* setvbuf con _IOLBF hace que cada '\n' fuerce flush (evita líneas parciales). */
    setvbuf(logf, NULL, _IOLBF, 0);

    /* Intentar abrir el dispositivo; si falla y es el por defecto, usar PRNG */
    int devfd = -1;
    int use_prng = 0;
    devfd = open(device, O_RDONLY | O_CLOEXEC);
    if (devfd < 0) {
        if (strcmp(device, default_device) == 0) {
            /* No fatal: /dev/urandom puede faltar en entornos minimalistas; usamos PRNG */
            use_prng = 1;
            unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
            srand(seed);
            fprintf(stderr, "Aviso: %s no disponible; usando PRNG interno.\n", default_device);
        } else {
            /* Si el usuario pidió explícitamente un device inexistente, fallamos rápido */
            fprintf(stderr, "Error fatal: no se puede abrir el dispositivo '%s': %s\n",
                    device, strerror(errno));
            fclose(logf);
            return 4;
        }
    }

    /* Bucle principal */
    while (!stop_requested) {
        uint8_t sample[8];
        int ok = 0;
        if (!use_prng) {
            ssize_t r = read(devfd, sample, sizeof(sample));
            if (r == (ssize_t)sizeof(sample)) ok = 1;
            else {
                /* lectura fallida -> caemos a PRNG para no interrumpir servicio */
                use_prng = 1;
                fprintf(stderr, "Aviso: lectura del dispositivo falló; usando PRNG.\n");
            }
        }
        if (use_prng) {
            for (size_t i = 0; i < sizeof(sample); ++i) sample[i] = (uint8_t)(rand() & 0xFF);
            ok = 1;
        }

        if (ok) {
            char hexval[3 * sizeof(sample) + 3];
            char *p = hexval;
            p += sprintf(p, "0x");
            for (size_t i = 0; i < sizeof(sample); ++i) {
                p += sprintf(p, "%02X", sample[i]);
            }
            char ts[64];
            iso8601_utc(ts, sizeof(ts));
            fprintf(logf, "%s | %s\n", ts, hexval);
            fflush(logf); // seguro
        }

        /* Dormimos 'interval' segundos, pero con sleep(1) por iteración para reaccionar a señales */
        for (unsigned int s = 0; s < interval && !stop_requested; ++s) {
            sleep(1);
        }
    }

    /* Cierre limpio */
    if (devfd >= 0) close(devfd);
    fclose(logf);
    fprintf(stderr, "Info: servicio detenido limpiamente.\n");
    return 0;
}
