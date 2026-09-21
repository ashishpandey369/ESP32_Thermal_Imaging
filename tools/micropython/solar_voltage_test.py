from machine import ADC, Pin
import time

# ==========================================
# PIN CONFIGURATION
# ==========================================
# Solar panel voltage divider -> GPIO32
solar_adc = ADC(Pin(32))
solar_adc.atten(ADC.ATTN_11DB)

# ==========================================
# CALIBRATION - SOLAR PANEL
# ==========================================
SOLAR_SLOPE = 0.000854
SOLAR_OFFSET = 0.079

NUM_SAMPLES = 10


def read_averaged(adc, samples=NUM_SAMPLES):
    """Read the ADC multiple times and return the average raw value."""
    total = 0

    for _ in range(samples):
        total += adc.read()
        time.sleep_ms(5)

    return total / samples


while True:
    solar_raw = read_averaged(solar_adc)
    solar_voltage = (solar_raw * SOLAR_SLOPE) + SOLAR_OFFSET

    print("====================================")
    print("SOLAR PANEL")
    print("Raw ADC (avg)     :", round(solar_raw, 1))
    print("Solar Voltage     :", round(solar_voltage, 2), "V")
    print("====================================")

    time.sleep(1)
