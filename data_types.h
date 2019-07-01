#ifndef DATATYPES_H
#define DATATYPES_H

const uint32_t dataHeader = 'A';
const uint8_t dataFooter = '\n';

struct resultPacket {
    uint32_t header;
    uint32_t frequency;
    uint32_t index;
    uint32_t amplitude;
    uint8_t lastHydrophone;
    int8_t ping_quarter;
    double angle; // 8 bytes
    int32_t timeStamp0;
    int32_t timeStamp1;
    int32_t timeStamp2;
    bool     overPinger;
    uint8_t footer;
};

struct settingsPacket {
    uint32_t dataHeader = dataHeader;
    float sampleRate0;
    float sampleRate1;
    float sampleRate2;
    uint32_t threshold;
    int32_t max_a_b;
    int32_t max_a_c;
    int32_t min_a_b;
    int32_t min_a_c;
    int32_t min_b_c;
    int32_t base_a_b;
    uint32_t dataFooter = dataFooter;
};



#endif // DATATYPES_H
