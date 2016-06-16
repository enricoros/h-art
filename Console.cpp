//
// Created by Enrico on 6/15/2016.
//

#include "Console.h"

#if defined(APP_CONSOLE)
// global console var
Stream *_Console = 0;

void Console::init() {
    APP_CONSOLE.begin(APP_CONSOLE_SPEED);
    _Console = &APP_CONSOLE;
}

#else
void Console::init() {}
#endif
