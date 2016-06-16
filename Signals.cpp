//
// Created by Enrico on 6/15/2016.
//

#include "Signals.h"

Averager::Averager(int size)
        : m_buffer(0), m_bufferSize(0) {
    resize(size);
}

void Averager::push(int value) {
    if (++m_bufferPos >= m_bufferSize)
        m_bufferPos = 0;
    if (m_bufferPos >= 0 && m_bufferPos < m_bufferSize)
        m_buffer[m_bufferPos] = value;
    if (m_bufferCount < m_bufferSize)
        m_bufferCount++;
}

int Averager::computeAvg() {
    if (m_bufferCount < 1 || m_bufferCount > m_bufferSize)
        return -1;
    long sum = 0;
    for (int i = 0; i < m_bufferCount; ++i)
        sum += m_buffer[i];
    long avg = sum / (long) m_bufferCount;
    return (int) avg;
}

void Averager::resize(int size) {
    if (m_buffer)
        delete[] m_buffer;
    m_buffer = new int[size];
    for (int i = 0; i < size; ++i)
        m_buffer[i] = 0;
    m_bufferSize = size;
    m_bufferCount = 0;
    m_bufferPos = -1;
}
