#ifndef global_def
#define global_def

typedef struct _versionControl
{
    uint8_t size;
    uint8_t MajorVersion;
    uint8_t MinorVersion;
    uint8_t HeaderLength;
    uint8_t TransmitInterval;
    uint8_t CRC;
} versionControl;

#endif