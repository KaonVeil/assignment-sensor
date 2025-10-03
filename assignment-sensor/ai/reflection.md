# ai/reflection.md

Qué pedí y por qué
- Empecé pidiendo un diseño general y un esqueleto en C para asegurar que el proyecto cumpliera los requisitos: CLI, logging, manejo de señales y fallback. Necesitaba una base segura y portable.

Cómo iteré
- La primera versión propuesta era funcional pero algo verbosa; pedí una simplificación. En cada iteración pedí sólo un aspecto a la vez 
- Verifiqué cada sugerencia compilando y probando localmente: comprobé la creación del logfile, el formato de las líneas y la respuesta al `SIGTERM`. Si una sugerencia no funcionaba perfectamente la adaptaba manualmente.

Qué acepté y qué modifiqué
- Acepté la arquitectura general, el uso de `sigaction`, `clock_gettime` y la recomendación de `/dev/urandom` como mock sensor.
- Rechacé o modifiqué propuestas que introducían complejidad innecesaria (p. ej. usar hilos o dependencias externas).

Cómo validé
- Compilé el binario con `gcc` y ejecuté pruebas manuales: muestreo en tiempo real, fallback a `/var/tmp`.

Conclusión
- La IA se usó como asistente de arquitectura y como caja de ideas iterativas; la implementación final fue revisada, probada y adaptada por mí para asegurar robustez, claridad y cumplimiento de los requisitos de la asignatura.

