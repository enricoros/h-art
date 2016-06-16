/* Enrico's Code */

#include "Arduino.h"
#include "Wire.h"
#include "Qduino.h"

// misc constants
#define APP_CONSOLE           Serial
#define APP_CONSOLE_SPEED     9600

#define APP_BLUETOOTH_SERIAL  Serial1

// console types implementation
#if defined(APP_CONSOLE)
Stream *_Console = 0;
#define CONSOLE_ADD(...)        _Console->print(__VA_ARGS__)
#define CONSOLE_LINE(...)       _Console->println(__VA_ARGS__)

void initConsole() {
    APP_CONSOLE.begin(APP_CONSOLE_SPEED);
    _Console = &APP_CONSOLE;
}

#else
#undef CONSOLE_SOFTWARE
#define CONSOLE_ADD(...)
#define CONSOLE_LINE(...)
void initConsole() {}
#endif


/*
 * will use the global 'Wire', embed a 'fuelGauge' instance
 */
class EGauge {
public:
    void setup() {
        Wire.begin();
        m_battery.setup();  // setup fuel gauge (includes reset)
    }

    int showChargeOnConsole() {
        int charge = measureChargeSinceLastReset();
        CONSOLE_ADD(charge);
        CONSOLE_LINE("%");
    }

    int showChargePulsedOnLed(qduino *q, int pulseDuration) {
        int charge = measureChargeSinceLastReset();
        if (charge >= 50)
            pulseLed(q, (charge - 50) / 10 + 1, pulseDuration, GREEN);
        else
            pulseLed(q, (50 - charge) / 10 + 1, pulseDuration, RED);
    }

private:
    fuelGauge m_battery;

    int measureChargeSinceLastReset() {
        int charge = m_battery.chargePercentage();  // get %
        //m_battery.reset();  // reset for next data request
        return charge;
    }

    void pulseLed(qduino *q, int n, int pulseLengthMs, COLORS color = GREEN) {
        while (n--) {
            q->setRGB(color);
            delay(pulseLengthMs / 2);
            q->ledOff();
            delay(pulseLengthMs / 2);
        }
    }
};

/**
 * @brief The BlueLink class communicates via a Blueetooth dongle
 *
 * The serial packet is comprised by: { 0xA5, B1, B2, B3, B4 },
 *   where the first is the sync byte (in case we lost it), and the other
 *   come from the packet encoding standard.
 */
class BlueLink {
    HardwareSerial *m_btSerial;
public:
    BlueLink(HardwareSerial *btSerial)
            : m_btSerial(btSerial) {
    }

    void init() const {
        // check if the user requested to reconfigure
//        if (digitalRead(LILY_PIN_BT_RECONFIG) == LOW)
        //          initReconfigure();
        //      else
        initNormal();
    }

    void rawPrintValue(int value) const {
        m_btSerial->print(value);
        m_btSerial->print(" ");
    }

    /**
     * @brief readPacket Decodes an incoming instruction packet transmission. Enrico's standard.
     * @param destBuffer a pointer to at least maxLength chars
     * @param maxLength of the destination command (the src packet has 1 sync byte too)
     * @return true if one or more packets have been decoded (just the last is kept though)
     */
    bool readPacket(byte *destBuffer, int maxLength) {
        // we need maxLength + 1 sync byte
        while (m_btSerial->available() >= (maxLength + 1)) {
            // validate the sync byte
            byte syncByte = (byte) m_btSerial->read();
            if (syncByte != 0xA5) {
                // ERROR: we were unsynced
                CONSOLE_ADD("e21: ");
                CONSOLE_LINE((int) syncByte);
                // try the next
                continue;
            }

            // read the next 4 bytes
            destBuffer[0] = (byte) m_btSerial->read();
            destBuffer[1] = (byte) m_btSerial->read();
            destBuffer[2] = (byte) m_btSerial->read();
            destBuffer[3] = (byte) m_btSerial->read();
            // we're ok for further processing
            return true;
        }

        // we didn't get any packet
        return false;
    }

private:
    void initNormal() const {
        CONSOLE_ADD("   * open BT... ");
        m_btSerial->begin(57600, SERIAL_8N1);
        delay(200);
        CONSOLE_LINE("done");
    }

    /* Reconf Manually with:
     *  $$$ SF,1\n SN,OneLife-Beats\n SP,1337\n ST,0\n SU,57.6\n ---\n
     */
    bool initReconfigure() const {
        CONSOLE_LINE("   * reconfiguraton requested ");

        // jump up to the modem speed, then move it to the target
        if (true /*lowerBtSpeed*/) {
            // start with the modem speed
            CONSOLE_ADD("     * changing the BT modem speed to 57600.. ");
            delay(1000);
            m_btSerial->begin(115200); // the default speed for the Bluetooth controller
            delay(1000);
            m_btSerial->print("$");
            m_btSerial->print("$");
            m_btSerial->print("$");
            delay(1000);
            m_btSerial->println("U,57.6,N");
            delay(1000);
            CONSOLE_LINE("done");
        }

        // now open at the right speed
        initNormal();

        CONSOLE_ADD("   * reconfiguring BT... ");

        // enter command mode
        if (!writeAndConfirm("$$$", "CMD\r\n", 5, 1000))
            return false;

        // factory config
        if (!writeAndConfirm("SF,1\n", "AOK\r\n", 5, 2000))
            return false;

        // rename device
        if (!writeAndConfirm("SN,OneLife-Beats\n", "AOK\r\n", 5, 1000))
            return false;

        // set pin to 1337
        if (!writeAndConfirm("SP,1337\n", "AOK\r\n", 5, 1000))
            return false;

        // turn config timer off
        if (!writeAndConfirm("ST,0\n", "AOK\r\n", 5, 1000))
            return false;

        // set the default speed
        if (!writeAndConfirm("SU,57.6\n", "AOK\r\n", 5, 1000))
            return false;

        // set the GPIO2 to Input, to not have the leds, which save power (restore with S%,0404)
        if (!writeAndConfirm("S%,0400\n", "AOK\r\n", 5, 1000))
            return false;

        // go back to data mode
        if (!writeAndConfirm("---\n", "END\r\n", 5, 1000))
            return false;

        CONSOLE_LINE("done.");
        return true;
    }

    bool writeAndConfirm(const char *msg, const char *expected, int length, int timeoutMs) const {
        m_btSerial->print(msg);
        bool ok = (expected == 0) || matchReply(expected, length, timeoutMs);
        if (!ok) {
            CONSOLE_ADD(ok ? "   * BlueLink: wrote: '" : "   * BlueLink: exception writing: '");
            CONSOLE_ADD(msg);
            CONSOLE_LINE("'");
        }
        return ok;
    }

    bool matchReply(const char *expected, int length, int timeoutMs) const {
        int sIdx = 0;
        while (timeoutMs > 0) {
            while (m_btSerial->available()) {
                char c = (char) m_btSerial->read();
                if (expected[sIdx] != c) {
                    CONSOLE_ADD("   * BlueLink: expected ");
                    CONSOLE_ADD((int) expected[sIdx]);
                    CONSOLE_ADD(" gotten ");
                    CONSOLE_ADD((int) c);
                    CONSOLE_ADD(" at index ");
                    CONSOLE_LINE((int) sIdx);
                    return false;
                }
                sIdx++;
                if (sIdx >= length)
                    return true;
            }
            delay(1);
            timeoutMs--;
        }
        return false;
    }
};

void crossStreams(Stream *s1, Stream *s2) {
    while (s1 && s1->available()) {
        const char c1 = (char) s1->read();
        if (s2)
            s2->print(c1);
    }
    while (s2 && s2->available()) {
        const char c2 = (char) s2->read();
        if (s1)
            s1->print(c2);
    }
}

// Note, this class assumes that the max int value < MAX_INT / bufferSize
class Averager {
public:
    Averager()
            : m_buffer(0), m_bufferSize(0) {
        resize(10);
    }

    void push(int value) {
        if (++m_bufferPos >= m_bufferSize)
            m_bufferPos = 0;
        if (m_bufferPos < m_bufferSize && m_bufferPos)
            m_buffer[m_bufferPos] = value;
        if (m_bufferCount < m_bufferSize)
            m_bufferCount++;
    }

    int computeAvg() {
        if (m_bufferCount < 1 || m_bufferCount > m_bufferSize)
            return -1;
        long sum = 0;
        for (int i = 0; i < m_bufferCount; ++i)
            sum += m_buffer[i];
        long avg = sum / (long) m_bufferCount;
        return (int) avg;
    }

    void resize(int size) {
        if (m_buffer)
            delete[] m_buffer;
        m_buffer = new int[size];
        for (int i = 0; i < size; ++i)
            m_buffer[i] = 0;
        m_bufferSize = size;
        m_bufferCount = 0;
        m_bufferPos = -1;
    }


private:
    int *m_buffer;
    int m_bufferSize;
    int m_bufferCount;
    int m_bufferPos;

};

// runtime globals
qduino sQduino;
BlueLink *sBlueLink = 0;
Averager *sAvg = 0;
//EGauge sFuel;



void setup() {
    // shorted to 3.3V or GND
    pinMode(6, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);
    pinMode(12, INPUT);

    // let the peripherials rest
    delay(1000);

    // console
    initConsole();
    CONSOLE_LINE("HeartBeat Booting...");

    // init QDuino Fuel gauge and LED
    CONSOLE_LINE(" * init QDuino");
    sQduino.setRGB(RED);
    //sFuel.setup();

    // init i/os (and let them settle)
    CONSOLE_LINE(" * init I/O");
    //setupAndExplainPullup(LILY_PIN_MODE_ANALOG_A,   "Analog A mode ");
    delay(100);

    // init Bluetooth
    CONSOLE_LINE(" * init BT");
    sBlueLink = new BlueLink(&APP_BLUETOOTH_SERIAL);
    sBlueLink->init();

    // init other
    sAvg = new Averager();
    sAvg->resize(100);

    // complete Boot
    CONSOLE_LINE("APP Ready!");
    sQduino.setRGB(GREEN);
}

void loop() {
    // upper limit to 500Hz
    delay(2);
//  sFuel.showChargePulsedOnLed(&sQduino, 400);
//  sFuel.showChargeOnConsole();

    // uncomment this
    //crossStreams(&APP_BLUETOOTH_SERIAL, &APP_CONSOLE);

    bool isDiscLeft = digitalRead(A0);
    bool isDiscRight = digitalRead(A1);

    int pulseVal = analogRead(A2);
    if (sAvg) {
        sAvg->push(pulseVal);
        pulseVal = sAvg->computeAvg();
    }

    sBlueLink->rawPrintValue(pulseVal);
    CONSOLE_LINE(pulseVal);



    //CONSOLE_LINE(isDiscLeft);
    //CONSOLE_LINE(isDiscRight);

/*  int val1 = analogRead(A2);
      Serial.print(val1);

      Serial.print(" ");
      Serial.print();

      Serial.print(" ");
      Serial.print(digitalRead(A1));
    Serial.println(".");*/
}

