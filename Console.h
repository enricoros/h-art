//
// Created by Enrico on 6/15/2016.
//

#ifndef H_ART_CONSOLE_H
#define H_ART_CONSOLE_H

#include "Arduino.h"

// App configuration (comment to disable console altogether)
// the console is implemented with the preprocessor for efficiency
#define APP_CONSOLE           Serial
#define APP_CONSOLE_SPEED     9600

#if defined(APP_CONSOLE)
extern Stream *_Console;
#define CONSOLE_ADD(...)        _Console->print(__VA_ARGS__)
#define CONSOLE_LINE(...)       _Console->println(__VA_ARGS__)
#else
#define CONSOLE_ADD(...)
#define CONSOLE_LINE(...)
#endif
namespace Console {
    void init();
};

#endif //H_ART_CONSOLE_H
