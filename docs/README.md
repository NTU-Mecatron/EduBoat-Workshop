# Eduboat Software


## Building Code
after `building` once; error of `Servo.h` not found: <br>
in `.pio\libdeps\Server\RC_ESC\src\ESC.h` <br>
change `#include <Servo.h>` to `#include <ESP32Servo.h>`
