#include "pcm.h"

#include <unistd.h>
#include <sys/ioctl.h>

#include <windows.h>

#define for if(0);else for

class PcmOutputWin32: public PcmOutput {
public:
    PcmOutputWin32();
    virtual ~PcmOutputWin32();
    virtual int getSampleRate() { return 44100; }
    virtual void output(const short *buf, int n);
    virtual void flush();
private:
    HWAVEOUT wo;
    struct Buffer {
        bool prepared;
        WAVEHDR hdr;
        enum {SIZE = 16384};
        short data[SIZE];
        int index;
    };
    enum {NBUFS = 4};
    Buffer buffers[NBUFS];
    int bufindex;
    int buftail;
    void wait();
};

PcmOutputWin32::PcmOutputWin32()
{
    WAVEFORMATEX wf;
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 1;
    wf.nSamplesPerSec = 44100;
    wf.nAvgBytesPerSec = 44100*16/8*1;
    wf.nBlockAlign = 16/8;
    wf.wBitsPerSample = 16;
    wf.cbSize = 0;
    MMRESULT r = waveOutOpen(&wo, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL);
    if (r != MMSYSERR_NOERROR) {
        fprintf(stderr, "could not open wave device\n");
        exit(1);
    }
    for (int i = 0; i < NBUFS; i++) {
        buffers[i].prepared = false;
        buffers[i].hdr.lpData = (char *)buffers[i].data;
        buffers[i].hdr.dwBufferLength = Buffer::SIZE*sizeof(short);
        buffers[i].hdr.dwFlags = 0;
        buffers[i].index = 0;
    }
    bufindex = 0;
    buftail = 0;
}

PcmOutputWin32::~PcmOutputWin32()
{
    flush();
    waveOutReset(wo);
    waveOutClose(wo);
}

void PcmOutputWin32::output(const short *buf, int n)
{
    while (n > 0) {
        if (!buffers[bufindex].prepared) {
            buffers[bufindex].hdr.dwBufferLength = Buffer::SIZE*sizeof(short);
            buffers[bufindex].hdr.dwFlags = 0;
            MMRESULT r = waveOutPrepareHeader(wo, &buffers[bufindex].hdr, sizeof(WAVEHDR));
            if (r != MMSYSERR_NOERROR) {
                fprintf(stderr, "error preparing header\n");
                return;
            }
            buffers[bufindex].index = 0;
            buffers[bufindex].prepared = true;
        }
        int c = Buffer::SIZE-buffers[bufindex].index;
        if (c > n) {
            c = n;
        }
        memcpy(buffers[bufindex].data+buffers[bufindex].index, buf, c*sizeof(short));
        buffers[bufindex].index += c;
        buf += c;
        n -= c;
        if (buffers[bufindex].index >= Buffer::SIZE) {
            MMRESULT r = waveOutWrite(wo, &buffers[bufindex].hdr, sizeof(WAVEHDR));
            if (r != MMSYSERR_NOERROR) {
                fprintf(stderr, "error on waveOutWrite: %d\n", r);
                return;
            }
            int newbufindex = (bufindex + 1) % NBUFS;
            if (newbufindex == buftail) {
                wait();
            }
            bufindex = newbufindex;
            assert(bufindex != buftail);
        }
    }
}

void PcmOutputWin32::flush()
{
    if (buffers[bufindex].prepared && buffers[bufindex].index > 0) {
        buffers[bufindex].hdr.dwBufferLength = buffers[bufindex].index*sizeof(short);
        MMRESULT r = waveOutWrite(wo, &buffers[bufindex].hdr, sizeof(WAVEHDR));
        if (r != MMSYSERR_NOERROR) {
            fprintf(stderr, "error on waveOutWrite: %d\n", r);
            return;
        }
        int newbufindex = (bufindex + 1) % NBUFS;
        if (newbufindex == buftail) {
            wait();
        }
        bufindex = newbufindex;
        assert(bufindex != buftail);
    }
    while (buftail != bufindex) {
        wait();
    }
}

void PcmOutputWin32::wait()
{
    if (buftail == bufindex) {
        return;
    }
    while ((buffers[buftail].hdr.dwFlags & WHDR_DONE) == 0) {
        Sleep(100);
    }
    MMRESULT r = waveOutUnprepareHeader(wo, &buffers[buftail].hdr, sizeof(WAVEHDR));
    if (r != MMSYSERR_NOERROR) {
        fprintf(stderr, "error on waveOutUnprepareHeader: %d\n", r);
        return;
    }
    buffers[buftail].prepared = false;
    buftail = (buftail + 1) % NBUFS;
}

PcmOutput *makePcmOutputWin32()
{
    return new PcmOutputWin32();
}
