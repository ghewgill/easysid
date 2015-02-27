#include <stdint.h>

class PcmOutput {
public:
    PcmOutput(uint32_t rate) : sample_rate(rate) {}
    virtual ~PcmOutput() {}
    virtual void output(const short *buf, int n) = 0;
    virtual void flush() {}

    uint32_t getSampleRate() const { return sample_rate; }

private:
    const uint32_t sample_rate;
};

PcmOutput *makePcmOutputWav(const char *fn, uint32_t rate);

#ifdef unix
PcmOutput *makePcmOutputUnix(uint32_t rate);
#define makePcmOutputAudio makePcmOutputUnix
#endif

#ifdef _WIN32
PcmOutput *makePcmOutputWin32(uint32_t rate);
#define makePcmOutputAudio makePcmOutputWin32
#endif

#ifdef __APPLE__
PcmOutput *makePcmOutputMacOSX(uint32_t rate);
#define makePcmOutputAudio makePcmOutputMacOSX
#endif
