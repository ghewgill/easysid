#include "pcm.h"

#include <sys/soundcard.h>

class PcmOutputUnix: public PcmOutput {
public:
    PcmOutputUnix(const char *dev);
    virtual ~PcmOutputUnix();
    virtual int getSampleRate();
    virtual void output(const short *buf, int n);
private:
    int fd;
    int sample_rate;
};

PcmOutputUnix::PcmOutputUnix(const char *dev)
{
    fd = open(dev, O_WRONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    int sndparam = AFMT_S16_LE;
    if (ioctl(fd, SNDCTL_DSP_SETFMT, &sndparam) == -1) { 
        perror("ioctl: SNDCTL_DSP_SETFMT");
        exit(1);
    }
    if (sndparam != AFMT_S16_LE) {
        perror("ioctl: SNDCTL_DSP_SETFMT");
        exit(1);
    }
    sndparam = 0;
    if (ioctl(fd, SNDCTL_DSP_STEREO, &sndparam) == -1) {
        perror("ioctl: SNDCTL_DSP_STEREO");
        exit(1);
    }
    if (sndparam != 0) {
        fprintf(stderr, "gen: Error, cannot set the channel number to 0\n");
        exit(1);
    }
    sample_rate = 44100;
    sndparam = sample_rate;
    if (ioctl(fd, SNDCTL_DSP_SPEED, &sndparam) == -1) {
        perror("ioctl: SNDCTL_DSP_SPEED");
        exit(1);
    }
    if ((10*abs(sndparam-sample_rate)) > sample_rate) {
        perror("ioctl: SNDCTL_DSP_SPEED");
        exit(1);
    }
    if (sndparam != sample_rate) {
        fprintf(stderr, "Warning: Sampling rate is %u, requested %u\n", sndparam, sample_rate);
    }
}

PcmOutputUnix::~PcmOutputUnix()
{
    close(fd);
}

int PcmOutputUnix::getSampleRate()
{
    return sample_rate;
}

void PcmOutputUnix::output(const short *buf, int n)
{
    write(fd, buf, n*sizeof(short));
}

PcmOutput *makePcmOutputUnix()
{
    return new PcmOutputUnix();
}
