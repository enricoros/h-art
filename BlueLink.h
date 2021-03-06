//
// Created by Enrico on 6/15/2016.
//

#ifndef H_ART_BLUELINK_H
#define H_ART_BLUELINK_H

#include <Arduino.h>

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
    BlueLink(HardwareSerial *btSerial);

    void init() const;

    /**
     * Decodes an incoming instruction packet transmission. Enrico's standard.
     * @param destBuffer a pointer to at least maxLength chars
     * @param maxLength of the destination command (the src packet has 1 sync byte too)
     * @return true if one or more packets have been decoded (just the last is kept though)
     */
    bool readCommandPacket(byte *destBuffer, int maxLength);

    /**
     * Sends a full raw packet to the connected device
     * NOTE: if a device is not connected, this has the potential of breaking the BT command/control
     *       pattern ($$$) and messing up with the comm.
     * No idea how to check if a BT device is connected...
     */
    bool sendSlavePacket(const byte *srcBuffer, size_t srcBufferLength);

private:
    void initNormal() const;

    /* Reconf Manually with:
     *  $$$ SF,1\n SN,OneLife-Beats\n SP,1337\n ST,0\n SU,57.6\n ---\n
     */
    bool initReconfigure() const;

    bool writeAndConfirm(const char *msg, const char *expected, int length, int timeoutMs) const;

    bool matchReply(const char *expected, int length, int timeoutMs) const;
};

#endif //H_ART_BLUELINK_H
