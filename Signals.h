//
// Created by Enrico on 6/15/2016.
//

#ifndef H_ART_SIGNALS_H
#define H_ART_SIGNALS_H

#include "Arduino.h"

// Note, this class assumes that the max int value < MAX_INT / bufferSize
class Averager {
public:
    Averager(int size = 10);

    void push(int value);

    int computeAvg();

private:
    int *m_buffer;
    int m_bufferSize;
    int m_bufferCount;
    int m_bufferPos;

    void resize(int size);
};

class Streams {
public:
    static void cross(Stream *s1, Stream *s2) {
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
};

#endif //H_ART_SIGNALS_H
