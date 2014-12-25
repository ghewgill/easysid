class PcmOutput {
public:
    virtual ~PcmOutput() {}
    virtual int getSampleRate() = 0;
    virtual void output(const short *buf, int n) = 0;
    virtual void flush() {}
};

PcmOutput *makePcmOutputWav(const char *fn);

#ifdef unix
PcmOutput *makePcmOutputUnix();
#define makePcmOutputAudio makePcmOutputUnix
#endif

#ifdef _WIN32
PcmOutput *makePcmOutputWin32();
#define makePcmOutputAudio makePcmOutputWin32
#endif

#ifdef __APPLE__
PcmOutput *makePcmOutputMacOSX();
#define makePcmOutputAudio makePcmOutputMacOSX
#endif
