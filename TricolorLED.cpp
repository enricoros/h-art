//
// Created by Enrico on 6/17/2016.
//

#include "TricolorLED.h"

#define QDUINO_USER_LED_R_PIN 10
#define QDUINO_USER_LED_G_PIN 11
#define QDUINO_USER_LED_B_PIN 13

static void applyUserRGB(uint8_t r, uint8_t g, uint8_t b);

// private vars (to this compilation unit, or 'file')
static uint8_t s_userR, s_userG, s_userB;
static bool s_userOverride;

void ::TricolorLED::init() {
    pinMode(QDUINO_USER_LED_R_PIN, OUTPUT);
    digitalWrite(QDUINO_USER_LED_R_PIN, HIGH);
    pinMode(QDUINO_USER_LED_G_PIN, OUTPUT);
    digitalWrite(QDUINO_USER_LED_G_PIN, HIGH);
    pinMode(QDUINO_USER_LED_B_PIN, OUTPUT);
    digitalWrite(QDUINO_USER_LED_B_PIN, HIGH);
    s_userR = 0;
    s_userG = 0;
    s_userB = 0;
    s_userOverride = false;
}

void ::TricolorLED::setUserRGB(TricolorLED::Color color) {
    switch (color) {
        case BLACK:
            setUserRGB(0, 0, 0);
            break;
        case WHITE:
            setUserRGB(255, 255, 255);
            break;
        case RED:
            setUserRGB(255, 0, 0);
            break;
        case GREEN:
            setUserRGB(0, 255, 0);
            break;
        case BLUE:
            setUserRGB(0, 0, 255);
            break;
    }
}

void ::TricolorLED::setUserRGB(uint8_t r, uint8_t g, uint8_t b) {
    s_userR = r;
    s_userG = g;
    s_userB = b;
    if (!s_userOverride)
        applyUserRGB(r, g, b);
}

void ::TricolorLED::overrideUserRGB(uint8_t r, uint8_t g, uint8_t b) {
    s_userOverride = true;
    applyUserRGB(r, g, b);
}

void ::TricolorLED::clearUserRGBOverride() {
    s_userOverride = false;
    setUserRGB(s_userR, s_userG, s_userB);
}

void ::TricolorLED::setUserRed(uint8_t r) {
    s_userR = r;
    if (!s_userOverride)
        analogWrite(QDUINO_USER_LED_R_PIN, 255 - r);
}

void ::TricolorLED::setUserGreen(uint8_t g) {
    s_userG = g;
    if (!s_userOverride)
        analogWrite(QDUINO_USER_LED_G_PIN, 255 - g);
}

void ::TricolorLED::setUserBlue(uint8_t b) {
    s_userB = b;
    if (!s_userOverride)
        analogWrite(QDUINO_USER_LED_B_PIN, 255 - b);
}

static void applyUserRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(QDUINO_USER_LED_R_PIN, 255 - r);
    analogWrite(QDUINO_USER_LED_G_PIN, 255 - g);
    analogWrite(QDUINO_USER_LED_B_PIN, 255 - b);
}
