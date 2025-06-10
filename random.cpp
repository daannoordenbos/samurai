#include <iostream>
#include <chrono>
#include "random.h"

/** Random integer generator **/
int m_w = 521288629;
int m_z = 362436069;

unsigned int randomInteger()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;
}

uint64_t randomLong()
{
    uint64_t a = randomInteger();
    uint64_t b = randomInteger();
    return (a << 32) | b;
}

int getTime ()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
