#include <Arduino.h>

#include "camera.h"
#include "system_config.h"
#include "usb_transport.h"
#include "wifi_transport.h"

namespace {

UsbTransport usbTransport;
WifiTransport wifiTransport;

} // namespace

void setup()
{
    usbTransport.begin();
    delay(500);

    if (!camera.begin()) {
        Serial.println("[SYSTEM][FATAL] MLX90640 initialization failed.");
        while (true) {
            delay(1000);
        }
    }

    if (wifiTransport.begin()) {
        Serial.print("[WIFI] SSID: ");
        Serial.println(ThermalConfig::WIFI_AP_SSID);
        Serial.print("[WIFI] IP: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("[WIFI] TCP port: ");
        Serial.println(ThermalConfig::WIFI_PORT);
    } else {
        Serial.println("[WIFI][WARN] Access point startup failed.");
    }
}

void loop()
{
    const uint32_t captureStart = micros();

    if (camera.update()) {
        const ThermalFrame &frame = camera.getFrame();

        // Keep both transports on the same raw binary frame. No image
        // interpolation or color processing is performed on the ESP32.
        usbTransport.sendFrame(frame);
        wifiTransport.sendFrame(frame);
    }

    // The sensor controls the frame cadence. Avoid adding an arbitrary
    // delay here because it only increases end-to-end latency.
    (void)captureStart;
}
