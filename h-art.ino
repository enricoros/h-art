//
// Created by Enrico on 6/15/2016.
//

#include <Arduino.h>

#include "BlueLink.h"
#include "Console.h"
#include "FuelGauge.h"
#include "Signals.h"
#include "TricolorLED.h"

// configuration
#define APP_BLUETOOTH_SERIAL    Serial1
#define APP_REPLAY_DEMO_DATA
#define MASTER_LOOP_DELAY       2  // 2ms = 500Hz


/**
 * @brief The DemoMode class runs an automated demo sequence, taking control of the main system loop.
 */
#define DEMOMODE_SAMPLES_COUNT      170
#define DEMOMODE_SAMPLES_FREQUENCY  200
class SignalRecorder {
private:
    // nice trace, 200Hz, 170 samples
    int m_demoTrace[DEMOMODE_SAMPLES_COUNT] = { 500, 500, 499, 500, 503, 503, 500, 500, 501, 498, 498, 501, 500, 499, 502, 502, 499, 502, 503, 501, 500, 502, 501, 500, 501, 502, 506, 508, 504, 503, 502, 500, 499, 500, 501, 498, 497, 498, 501, 501, 501, 502, 502, 503, 503, 517, 549, 578, 582, 535, 449, 375, 352, 373, 384, 400, 442, 478, 500, 507, 507, 510, 516, 520, 520, 520, 526, 527, 524, 525, 526, 527, 530, 531, 530, 531, 533, 532, 531, 535, 537, 534, 534, 536, 535, 537, 538, 540, 541, 543, 545, 547, 549, 550, 552, 554, 554, 553, 552, 549, 545, 542, 537, 531, 525, 520, 514, 510, 507, 503, 500, 499, 499, 497, 496, 496, 495, 495, 496, 493, 493, 498, 499, 500, 500, 501, 502, 505, 505, 504, 503, 503, 503, 503, 507, 508, 508, 507, 507, 506, 506, 508, 509, 511, 512, 509, 509, 511, 511, 509, 509, 509, 507, 506, 507, 508, 508, 510, 509, 508, 508, 506, 505, 506, 506, 503, 503, 504, 501, 501 };
    int m_readIdx;

public:
    SignalRecorder()
      : m_readIdx(0) {
    }

    int analogReadSample() {
        int val = m_demoTrace[m_readIdx];
        if (++m_readIdx >= DEMOMODE_SAMPLES_COUNT)
            m_readIdx = 0;
        delay(1000 / DEMOMODE_SAMPLES_FREQUENCY - MASTER_LOOP_DELAY);
        return val;
    }

    void captureTrace() {
        CONSOLE_ADD("Capturing... ");
        for (int i = 5; i > 0; i--) {
            delay(1000);
            CONSOLE_ADD(i);
            CONSOLE_ADD(".. ");
        }
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


BlueLink *sBlueLink = 0;
Averager *sAvg = 0;
SignalRecorder *sSignalRecorder = 0;


void setup() {
    // shorted to 3.3V or GND
    pinMode(6, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);
    pinMode(12, INPUT);

    // let the peripherals rest, use this time for re-flashing in case the cpu hangs after this
    delay(2000);

    // console
    Console::init();
    CONSOLE_LINE("HeartBeat Booting...");

    // init QDuino LED - Fuel Gauge is not inited here - the chip is dormant until the first read
    CONSOLE_LINE(" * init QDuino");
    TricolorLED::init();
    TricolorLED::setUserRGB(TricolorLED::RED);

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
    CONSOLE_LINE("HeartBeat ready.");
    TricolorLED::setUserRGB(TricolorLED::BLACK);

    // TEMP - FIXME - actually start the demo mode (replays recorded data)
#if defined(APP_REPLAY_DEMO_DATA)
    sSignalRecorder = new SignalRecorder();
#endif
}

// slave configuration
bool sCaptureEnabled;
int sSamplingFrequency;
int sReportingFrequency;
int sSamplesPerReport;
bool sFilterAverage;
int sFilterAverageLength;
bool tmp_checkFuel = false;
int tmp_fuelSkipper = 2000;
// runtime globals
byte sBTCommandBuffer[4];

bool readConsoleCommand(Stream *stream);
bool executeCommandPacket(const byte *command);

class SingnalProcessor {

};


int tmp_min = 999;
int tmp_max = 0;

void loop() {
    // uncomment to enable a direct talk from the USB Console to the Bluetooth Modem
    //Streams::cross(&APP_BLUETOOTH_SERIAL, &APP_CONSOLE);
    //return;

    // uncomment to enable signal recording with console output/plot
    /*if (sSignalRecorder) {
        sSignalRecorder->captureTrace();
        sSignalRecorder->printTraceAsArray();
        sSignalRecorder->printTraceAsPlot();
        delay(1000);
        return;
    }*/

    // no local 'pin' configuration

    // single mode of operation: configured via bluetooth

    // check the BT serial for new commands
    bool hasNewCommand = false;
    if (sBlueLink->readCommandPacket(sBTCommandBuffer, 4))
        hasNewCommand = true;

#if defined(APP_CONSOLE)
    // check the console for new commands
    if (!hasNewCommand)
        hasNewCommand = readConsoleCommand(&APP_CONSOLE, sBTCommandBuffer);
#endif

    // execute a command, when received (and blink the LED)
    if (hasNewCommand)
        if (executeCommandPacket(sBTCommandBuffer))
            TricolorLED::setUserRGB(TricolorLED::RED);

    // perform the current Mode/Task/Operation
    bool sLastDiscLeft;
    bool sLastDiscRight;
    int sLastPulseVal;
    if (sSignalRecorder) {
        sLastDiscLeft = false;
        sLastDiscRight = false;
        sLastPulseVal = 1023 - sSignalRecorder->analogReadSample();
    } else {
        sLastDiscLeft = digitalRead(A0);
        sLastDiscRight = digitalRead(A1);
        sLastPulseVal = 1023 - analogRead(A2);
    }

    if (sAvg) {
        sAvg->push(sLastPulseVal);
        sLastPulseVal = sAvg->computeAvg();
    }

    //CONSOLE_LINE(sLastPulseVal);

    //sBlueLink->rawPrintValue(pulseVal);

    int scval = (sLastPulseVal) >> 0;
    if (scval < tmp_min)
        tmp_min = scval;
    scval -= tmp_min;
    if (scval > 255)
        scval = 255;

    TricolorLED::setUserBlue(scval);

    // if enable, check continuously for fuel
    if (tmp_checkFuel && !--tmp_fuelSkipper) {
        QD::FuelGauge::showChargePulsedOnTricolor(400);
        //QD::FuelGauge::showChargeOnConsole(&APP_CONSOLE);
        tmp_fuelSkipper = 2000;
    }

    // upper limit to frequency
    delay(MASTER_LOOP_DELAY);

    // save energy, stop the LED now
    if (hasNewCommand)
        TricolorLED::setUserRGB(TricolorLED::BLACK);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "missing_default_case"
bool executeCommandPacket(const byte *msg) {
    const byte rCmd    = msg[0];
    const byte rTarget = msg[1];
    const byte rValue1 = msg[2];
    const byte rValue2 = msg[3];

    switch (rCmd) {
        case 0x01: { // C: 01 -> read variable
            int variableValue = 0;
            switch (rTarget) {
                case 1:
                    variableValue = QD::FuelGauge::measureChargeSinceLastReset();
                    break;
                case 2:
                    variableValue = tmp_checkFuel;
                    break;
            };
            const byte slavePacket[4] = {0x01, rTarget, (const byte) (variableValue & 0xff),
                                         (const byte) ((variableValue >> 8) & 0xff)};
            sBlueLink->sendSlavePacket(slavePacket, 4);
        }; return true;

        case 0x02: // C: 02 -> set variable
            switch (rTarget) {
                case 2:
                    tmp_checkFuel = rValue1;
                    if (tmp_checkFuel) {
                        QD::FuelGauge::showChargePulsedOnTricolor(400);
                        QD::FuelGauge::showChargeOnConsole(&APP_CONSOLE);
                    } else
                        CONSOLE_LINE("FG OFF");
                    return true;
                case 3:
                    TricolorLED::clearUserRGBOverride();
                    return true;
            };
            break;

        case 0x03: // C: 03 -> set led
            TricolorLED::overrideUserRGB(rTarget, rValue1, rValue2);
            return true;

        case 0x04: // C: 04 -> start/stop sampling
            CONSOLE_LINE("not implemented");
            return true;
    }

    // bad command
    CONSOLE_ADD("bt_bad_cmd: ");
    CONSOLE_LINE((int) rCmd);
    return false;
}
#pragma clang diagnostic pop

bool readConsoleCommand(Stream *stream, byte *cmdBuffer) {
    while (stream->available()) {
        char c = (char)stream->read();
        switch (c) {
            case 'l': case 'L': cmdBuffer[0] = 3; cmdBuffer[1] = 255; cmdBuffer[2] = 25; cmdBuffer[3] = 100; return true;
            case 'm': case 'M': cmdBuffer[0] = 2; cmdBuffer[1] = 3; return true;
            case 'f': case 'F': cmdBuffer[0] = 2; cmdBuffer[1] = 2; cmdBuffer[2] = tmp_checkFuel ? 0 : 1; return true;
        }
    }
    return false;
}
