//
// Created by Enrico on 6/15/2016.
//

#include "Console.h"

void Console::init() {
#if defined(APP_CONSOLE)
    APP_CONSOLE.begin(APP_CONSOLE_SPEED);
#endif
}
