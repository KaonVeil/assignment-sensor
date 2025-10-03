sudo systemctl start assignment-sensor.service

Wait 10 seconds.

tail -n 5 /tmp/assignment_sensor.log

Expected lines similar to:

2025-09-28T14:12:35.123Z | 0xA1B2C3D4E5F60708


