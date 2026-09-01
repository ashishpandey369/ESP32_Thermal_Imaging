#include <Arduino.h>

#include "camera.h"

void setup()
{
    if (!camera.begin()) {
        while (true) {
            delay(1000);
        }
    }
}

void loop()
{
    camera.update();
}
