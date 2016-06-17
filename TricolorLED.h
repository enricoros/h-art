//
// Created by Enrico on 6/17/2016.
//

#ifndef H_ART_TRICOLORLED_H
#define H_ART_TRICOLORLED_H

#include <Arduino.h>

/**
 * We reimplemt this because <QDuino.h> has wrong mappings for the RED channel
 */
namespace TricolorLED {

    enum Color { BLACK, WHITE, RED, GREEN, BLUE };

    void init();

    void setUserRGB(Color color);
    void setUserRGB(uint8_t r, uint8_t g, uint8_t b);
    void setUserRed(uint8_t r);
    void setUserGreen(uint8_t g);
    void setUserBlue(uint8_t b);

    void overrideUserRGB(uint8_t r, uint8_t g, uint8_t b);
    void clearUserRGBOverride();

};


#endif //H_ART_TRICOLORLED_H
