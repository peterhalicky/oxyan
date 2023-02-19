#include <Arduino.h>
#include <SPI.h>
#include <LiquidCrystal_PCF8574.h>
#include <Adafruit_ADS1X15.h>

#define CAL_COUNT 100
#define AIR_PCT 20.9

LiquidCrystal_PCF8574 lcd(0x27);
Adafruit_ADS1115 ads;
bool isError = false;
double airMv = 10;

int cmpint16(const void *a, const void *b) {
    return (*(int16_t *) a - *(int16_t *) b);
}

void calibrate() {
    int16_t measurements[CAL_COUNT];
    for (int &measurement: measurements) {
        measurement = ads.readADC_Differential_0_1();
    }

    qsort(measurements, CAL_COUNT, sizeof(int16_t), cmpint16);
    int32_t sum = 0;
    int16_t count = 0;
    for (int i = CAL_COUNT / 10; i < CAL_COUNT - (CAL_COUNT / 10); i++) {
        sum += measurements[i];
        count++;
    }

    auto airAdc = (int16_t)((double)sum / count);
    airMv = ads.computeVolts(airAdc) * 1000;
}

void setup() {
    lcd.begin(16, 2);
    lcd.setBacklight(255);
    lcd.home();
    lcd.clear();

    if (!ads.begin()) {
        lcd.print("Error initializing ADS1115");
        isError = true;
    }

    ads.setGain(GAIN_SIXTEEN);
    calibrate();
}

void loop() {
    if (isError) {
        delay(100);
        return;
    }

    int16_t adcSensor = ads.readADC_Differential_0_1();

    double nowMv = ads.computeVolts(adcSensor) * 1000;

    lcd.clear();
    lcd.home();
    lcd.print(nowMv);
    lcd.print("mV");

    double nowPct = (AIR_PCT / airMv) * nowMv;

    lcd.setCursor(0, 1);

    lcd.print(nowPct);
    lcd.print("%");

    delay(200);
}