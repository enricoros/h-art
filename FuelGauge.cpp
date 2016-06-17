//
// Created by Enrico on 6/15/2016.
//

#include "FuelGauge.h"
#include <QDuino.h>
#include <Wire.h>

static FuelGauge *sFuelGaugeInstance;
FuelGauge *FuelGauge::instance() {
    if (!sFuelGaugeInstance)
        sFuelGaugeInstance = new FuelGauge();
    return sFuelGaugeInstance;
}


FuelGauge::FuelGauge() {
    Wire.begin();
    m_battery = new fuelGauge();
    // setup fuel gauge (includes reset)
    m_battery->setup();
    delay(200);
}

FuelGauge::~FuelGauge() {
    delete m_battery;
}


int FuelGauge::measureChargeSinceLastReset() {
    int charge = m_battery->chargePercentage();  // get %
    //m_battery->reset();  // reset for next data request
    return charge;
}

void FuelGauge::showChargeOnConsole(Stream *console) {
    int charge = measureChargeSinceLastReset();
    console->print(charge);
    console->println("%");
}

void FuelGauge::showChargePulsedOnQduino(qduino *q, int pulseDuration) {
    int charge = measureChargeSinceLastReset();
    if (charge >= 50)
        pulseLed(q, (charge - 50) / 10 + 1, pulseDuration, GREEN);
    else
        pulseLed(q, (50 - charge) / 10 + 1, pulseDuration, RED);
}

void FuelGauge::pulseLed(qduino *q, int n, int pulseLengthMs, int color) {
    while (n--) {
        q->setRGB((COLORS) color);
        delay((unsigned long) (pulseLengthMs / 2));
        q->ledOff();
        delay((unsigned long) (pulseLengthMs / 2));
    }
}
