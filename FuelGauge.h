//
// Created by Enrico on 6/15/2016.
//

#ifndef H_ART_QD_FUELGAUGE_H
#define H_ART_QD_FUELGAUGE_H

class Stream;

class fuelGauge;

class qduino;

/*
 * will use the global 'Wire', embed a 'fuelGauge' instance
 */
class FuelGauge {
public:
    static FuelGauge *instance();

    int measureChargeSinceLastReset();

    void showChargeOnConsole(Stream *);

    void showChargePulsedOnQduino(qduino *q, int pulseDuration);


private:
    FuelGauge();
    ~FuelGauge();

    void pulseLed(qduino *q, int n, int pulseLengthMs, int color);

    fuelGauge *m_battery;
};

#endif //H_ART_QD_FUELGAUGE_H
