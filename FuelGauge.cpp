//
// Created by Enrico on 6/15/2016.
//

#include "FuelGauge.h"
#include "TricolorLED.h"
#include <Qduino.h>
#include <Wire.h>

static fuelGauge *sFuelGauge = 0;

int ::QD::FuelGauge::measureChargeSinceLastReset() {
    if (!sFuelGauge) {
        Wire.begin();
        sFuelGauge = new fuelGauge();
        // setup fuel gauge (includes reset)
        sFuelGauge->setup();
        // delay to let the Fuel Gauge perform the first reading
        delay(200);
    }
    int chargePct = sFuelGauge->chargePercentage();  // get %
    //m_battery->reset();  // reset for next data request
    return chargePct;
}

void ::QD::FuelGauge::showChargeOnConsole(Stream *console) {
    int charge = measureChargeSinceLastReset();
    console->print(charge);
    console->println("%");
}

void ::QD::FuelGauge::showChargePulsedOnTricolor(int pulseDuration) {
    int charge = measureChargeSinceLastReset();
    int n = (charge >= 50) ? ((charge - 50) / 10 + 1) : ((50 - charge) / 10 + 1);
    TricolorLED::Color color = (charge >= 50) ? TricolorLED::GREEN : TricolorLED::RED;
    while (n--) {
        TricolorLED::setUserRGB(color);
        delay((unsigned long) (pulseDuration / 2));
        TricolorLED::setUserRGB(TricolorLED::BLACK);
        delay((unsigned long) (pulseDuration / 2));
    }
}
