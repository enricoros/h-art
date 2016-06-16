//
// Created by Enrico on 6/15/2016.
//

#include "QD_FuelGauge.h"

#include "Wire.h"
#include "QDuino.h"

FuelGauge::FuelGauge() {
    Wire.begin();
    m_battery = new fuelGauge();
    // setup fuel gauge (includes reset)
    m_battery->setup();
}

FuelGauge::~FuelGauge() {
    delete m_battery;
}

int FuelGauge::showChargeOnConsole(Stream *console) {
    int charge = measureChargeSinceLastReset();
    console->print(charge);
    console->println("%");
}

int FuelGauge::showChargePulsedOnQduino(qduino *q, int pulseDuration) {
    int charge = measureChargeSinceLastReset();
    if (charge >= 50)
        pulseLed(q, (charge - 50) / 10 + 1, pulseDuration, GREEN);
    else
        pulseLed(q, (50 - charge) / 10 + 1, pulseDuration, RED);
}

int FuelGauge::measureChargeSinceLastReset() {
    int charge = m_battery->chargePercentage();  // get %
    //m_battery->reset();  // reset for next data request
    return charge;
}

void FuelGauge::pulseLed(qduino *q, int n, int pulseLengthMs, int color) {
    while (n--) {
        q->setRGB((COLORS)color);
        delay(pulseLengthMs / 2);
        q->ledOff();
        delay(pulseLengthMs / 2);
    }
}
