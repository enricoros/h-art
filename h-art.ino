/* Enrico's Code */

#include "Arduino.h"

// Console - configured in Console.h
#include "Console.h"

// Bluetooth operation
#include "BlueLink.h"

#define APP_BLUETOOTH_SERIAL  Serial1

// QDuino specific
#include "Qduino.h"
#include "QD_FuelGauge.h"

// signal processing
#include "Signals.h"


qduino *sQduino = 0;
BlueLink *sBlueLink = 0;
Averager *sAvg = 0;
FuelGauge *sFuelGauge = 0;

void setup() {
    // shorted to 3.3V or GND
    pinMode(6, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);
    pinMode(12, INPUT);

    // let the peripherals rest
    delay(2000);

    // console
    Console::init();
    CONSOLE_LINE("HeartBeat Booting...");

    // init QDuino Fuel gauge and LED
    CONSOLE_LINE(" * init QDuino");
    sQduino = new qduino();
    sQduino->setRGB(RED);
    sFuelGauge = new FuelGauge();

    // init i/os (and let them settle)
    CONSOLE_LINE(" * init I/O");
    //setupAndExplainPullup(LILY_PIN_MODE_ANALOG_A,   "Analog A mode ");
    delay(100);

    // init Bluetooth
    CONSOLE_LINE(" * init BT");
    sBlueLink = new BlueLink(&APP_BLUETOOTH_SERIAL);
    sBlueLink->init();

    // init other
    sAvg = new Averager(10);

    // complete Boot
    CONSOLE_LINE("APP Ready!");
    sQduino->setRGB(GREEN);
}


/**
 * @brief The DemoMode class runs an automated demo sequence, taking control of the main system loop.
 */
#define DEMOMODE_SAMPLES_COUNT      170
#define DEMOMODE_SAMPLES_FREQUENCY  200

class DemoMode {
private:
    // nice trace, 200Hz, 170 samples
    int m_demoTrace[DEMOMODE_SAMPLES_COUNT] = { 500, 500, 499, 500, 503, 503, 500, 500, 501, 498, 498, 501, 500, 499, 502, 502, 499, 502, 503, 501, 500, 502, 501, 500, 501, 502, 506, 508, 504, 503, 502, 500, 499, 500, 501, 498, 497, 498, 501, 501, 501, 502, 502, 503, 503, 517, 549, 578, 582, 535, 449, 375, 352, 373, 384, 400, 442, 478, 500, 507, 507, 510, 516, 520, 520, 520, 526, 527, 524, 525, 526, 527, 530, 531, 530, 531, 533, 532, 531, 535, 537, 534, 534, 536, 535, 537, 538, 540, 541, 543, 545, 547, 549, 550, 552, 554, 554, 553, 552, 549, 545, 542, 537, 531, 525, 520, 514, 510, 507, 503, 500, 499, 499, 497, 496, 496, 495, 495, 496, 493, 493, 498, 499, 500, 500, 501, 502, 505, 505, 504, 503, 503, 503, 503, 507, 508, 508, 507, 507, 506, 506, 508, 509, 511, 512, 509, 509, 511, 511, 509, 509, 509, 507, 506, 507, 508, 508, 510, 509, 508, 508, 506, 505, 506, 506, 503, 503, 504, 501, 501 };
    int m_readIdx;

public:
    DemoMode()
      : m_readIdx(0) {
    }

    int analogReadSample() {
        int val = m_demoTrace[m_readIdx];
        if (++m_readIdx >= DEMOMODE_SAMPLES_COUNT)
            m_readIdx = 0;
        return val;
    }

    void captureTrace() {
        // 5 seconds delay
        CONSOLE_ADD("Capturing... ");
        for (int i = 5; i > 0; i--) {
            delay(1000);
            CONSOLE_ADD(i);
            CONSOLE_ADD(".. ");
        }
        // capture
        for (int i = 0; i < DEMOMODE_SAMPLES_COUNT; i++) {
            m_demoTrace[i] = analogRead(A2);
            delay(1000 / DEMOMODE_SAMPLES_FREQUENCY);
        }
        CONSOLE_LINE("done");
    }

    void printTraceAsPlot() const {
        for (int i = 0; i < DEMOMODE_SAMPLES_COUNT; i++) {
            int pulseVal = 1023 - m_demoTrace[i];
            CONSOLE_LINE(pulseVal);
            delay(1000 / DEMOMODE_SAMPLES_FREQUENCY);
        }
    }

    void printTraceAsArray() const {
        for (int i = 0; i < DEMOMODE_SAMPLES_COUNT; i++) {
            CONSOLE_ADD(m_demoTrace[i]);
            CONSOLE_ADD(", ");
        }
    }
};


DemoMode *demo = new DemoMode();
bool sLastDiscLeft;
bool sLastDiscRight;
int sLastPulseVal;

void loop() {
    /* DEMO recording
    if (demo) {
        demo->captureTrace();
        demo->printTraceAsArray();
        demo->printTraceAsPlot();
        delay(1000);
        return;
    }*/

    // upper limit to 500Hz
    delay(2);

    //sFuelGauge->showChargePulsedOnQduino(sQduino, 400);
    //sFuelGauge->showChargeOnConsole(&APP_CONSOLE);

    // uncomment this to talk from the USB Console to the Bluetooth Modem
    //Streams::cross(&APP_BLUETOOTH_SERIAL, &APP_CONSOLE);

    if (demo) {
        sLastDiscLeft = false;
        sLastDiscRight = false;
        sLastPulseVal = 1023 - demo->analogReadSample();
    } else {
        sLastDiscLeft = digitalRead(A0);
        sLastDiscRight = digitalRead(A1);
        sLastPulseVal = 1023 - analogRead(A2);
    }

    CONSOLE_LINE(sLastPulseVal);
    
    if (sAvg) {
        sAvg->push(sLastPulseVal);
        sLastPulseVal = sAvg->computeAvg();
    }

    //sBlueLink->rawPrintValue(pulseVal);
}

