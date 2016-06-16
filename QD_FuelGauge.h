//
// Created by Enrico on 6/15/2016.
//

#ifndef H_ART_QD_FUELGAUGE_H
#define H_ART_QD_FUELGAUGE_H

class qduino;
class fuelGauge;
class Stream;

/*
 * will use the global 'Wire', embed a 'fuelGauge' instance
 */
class FuelGauge {
public:
    FuelGauge();
    ~FuelGauge();

    int showChargeOnConsole(Stream *);

    int showChargePulsedOnQduino(qduino *q, int pulseDuration);

private:
    fuelGauge *m_battery;

    int measureChargeSinceLastReset();

    void pulseLed(qduino *q, int n, int pulseLengthMs, int color);
};

#endif //H_ART_QD_FUELGAUGE_H
