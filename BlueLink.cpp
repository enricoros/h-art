//
// Created by Enrico on 6/15/2016.
//

#include "BlueLink.h"
#include "Console.h"

BlueLink::BlueLink(HardwareSerial *btSerial)
        : m_btSerial(btSerial) {
}

void BlueLink::init() const {
    // check if the user requested to reconfigure
    //if (digitalRead(LILY_PIN_BT_RECONFIG) == LOW)
    //    initReconfigure();
    //else
    initNormal();
}

void BlueLink::rawPrintValue(int value) const {
    m_btSerial->print(value);
    m_btSerial->print(" ");
}

bool BlueLink::readPacket(byte *destBuffer, int maxLength) {
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

void BlueLink::initNormal() const {
    CONSOLE_ADD("   * open BT... ");
    m_btSerial->begin(57600, SERIAL_8N1);
    delay(200);
    CONSOLE_LINE("done");
}

bool BlueLink::initReconfigure() const {
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

bool BlueLink::writeAndConfirm(const char *msg, const char *expected, int length, int timeoutMs) const {
    m_btSerial->print(msg);
    bool ok = (expected == 0) || matchReply(expected, length, timeoutMs);
    if (!ok) {
        CONSOLE_ADD(ok ? "   * BlueLink: wrote: '" : "   * BlueLink: exception writing: '");
        CONSOLE_ADD(msg);
        CONSOLE_LINE("'");
    }
    return ok;
}

bool BlueLink::matchReply(const char *expected, int length, int timeoutMs) const {
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

