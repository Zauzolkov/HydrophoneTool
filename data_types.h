#ifndef DATATYPES_H
#define DATATYPES_H

const uint32_t dataHeader = 'A';
const uint32_t dataFooter = '\n';

/* TODO: добавить bool abovePinger?
 * чтобы сохранить размер пакета в 48 байт,
 * сократить заголовок или конец пакета
 * uint32_t → uint16_t
 */

struct resultPacket {
    uint32_t header;
    uint32_t frequency;
    uint32_t index;
    uint32_t amplitude;
    uint8_t firstHydrophone;
    int8_t ping_quarter;
    double angle; // 8 bytes
    uint32_t timeStamp0;
    uint32_t timeStamp1;
    uint32_t timeStamp2;
    bool     overPinger;
    uint8_t footer;
};

//struct resultPacket {
//    uint32_t header = dataHeader;
//    uint32_t frequency;
//    uint32_t index;
//    uint32_t amplitude;
//    uint32_t firstHydrophone;
//    int32_t ping_quarter;
//    double angle; // 8 bytes
//    uint32_t timeStamp0;
//    uint32_t timeStamp1;
//    uint32_t timeStamp2;
//    uint32_t footer = dataFooter;
//};

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
