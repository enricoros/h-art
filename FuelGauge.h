//
// Created by Enrico on 6/15/2016.
//

#ifndef H_ART_FUELGAUGE_H
#define H_ART_FUELGAUGE_H

class Stream;

class fuelGauge;

/*
 * will use the global 'Wire', embed a 'fuelGauge' instance
 */
namespace QD {
    namespace FuelGauge {
        int measureChargeSinceLastReset();

        void showChargeOnConsole(Stream *);

        void showChargePulsedOnTricolor(int pulseDuration);
    };
};

#endif //H_ART_FUELGAUGE_H
