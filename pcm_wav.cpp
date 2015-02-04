#include "pcm.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class PcmOutputWav: public PcmOutput {
public:
    PcmOutputWav(const char *fn);
    virtual ~PcmOutputWav();
    virtual int getSampleRate() { return 44100; }
    virtual void output(const short *buf, int n);
    virtual void flush();
private:
    struct Header {
        char tagRIFF[4];
        unsigned long riffsize;
        char tagWAVE[4];
        char tagfmt[4];
        unsigned long fmtsize;
        unsigned short wFormatTag;
        unsigned short nChannels;
        unsigned long nSamplesPerSec;
        unsigned long nAvgBytesPerSec;
        unsigned short nBlockAlign;
        unsigned short nBitsPerSample;
        char tagdata[4];
        unsigned long datasize;
    };
    Header header;
    int data_size;
    FILE *f;
};

PcmOutputWav::PcmOutputWav(const char *fn)
{
    data_size = 0;

    strncpy(header.tagRIFF, "RIFF", 4);
    header.riffsize = 0;
    strncpy(header.tagWAVE, "WAVE", 4);
    strncpy(header.tagfmt, "fmt ", 4);
    header.fmtsize = 16;
    header.wFormatTag = 1;
    header.nChannels = 1;
    header.nSamplesPerSec = 44100;
    header.nAvgBytesPerSec = 44100*16/8*1;
    header.nBlockAlign = 16/8;
    header.nBitsPerSample = 16;
    strncpy(header.tagdata, "data", 4);
    header.datasize = 0;
    f = fopen(fn, "wb");
    if (f == NULL) {
        perror("failed to open output file");
        exit(1);
    }
    fwrite(&header, 1, sizeof(header), f);
}

PcmOutputWav::~PcmOutputWav()
{
    flush();
    fclose(f);
}

void PcmOutputWav::output(const short *buf, int n)
{
    fwrite(buf, sizeof(short), n, f);
    data_size += n*sizeof(short);
}

void PcmOutputWav::flush()
{
    header.riffsize = 36+data_size;
    header.datasize = data_size;
    fseek(f, 0, SEEK_SET);
    fwrite(&header, 1, sizeof(header), f);
    fseek(f, 0, SEEK_END);
}

PcmOutput *makePcmOutputWav(const char *fn)
{
    return new PcmOutputWav(fn);
}
