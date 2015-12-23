#include "pcm.h"

#include <semaphore.h>
#include <AudioToolbox/AudioQueue.h>

class PcmOutputMacOSX: public PcmOutput {
public:
    PcmOutputMacOSX(uint32_t rate);
    virtual ~PcmOutputMacOSX();
    virtual void output(const short *buf, int n);
    virtual void flush();
private:
    enum {
        NBUFFERS = 3,
        BUFFER_SIZE = 4096
    };
    short buffer[BUFFER_SIZE];
    int index;
    AudioQueueRef aq;
    AudioQueueBufferRef buffers[NBUFFERS];
    sem_t *sem_notfull;
    sem_t *sem_full;
    static void callback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);
};

PcmOutputMacOSX::PcmOutputMacOSX(uint32_t rate)
 : PcmOutput(rate), index(0), aq(NULL)
{
    sem_unlink("pcm.notfull");
    sem_unlink("pcm.full");
    sem_notfull = sem_open("pcm.notfull", O_CREAT|O_EXCL, 0700, 1);
    if (sem_notfull == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_full = sem_open("pcm.full", O_CREAT|O_EXCL, 0700, 0);
    if (sem_full == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    AudioStreamBasicDescription format;
    format.mSampleRate = getSampleRate();
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    format.mBytesPerPacket = 2;
    format.mFramesPerPacket = 1;
    format.mBytesPerFrame = 2;
    format.mChannelsPerFrame = 1;
    format.mBitsPerChannel = 16;
    format.mReserved = 0;
    OSStatus status = AudioQueueNewOutput(&format, callback, this, NULL, NULL, 0, &aq);
    if (status != 0) {
        fprintf(stderr, "could not open output audio queue: %d\n", status);
        exit(1);
    }
    for (int i = 0; i < NBUFFERS; i++) {
        status = AudioQueueAllocateBuffer(aq, BUFFER_SIZE*2, &buffers[i]);
        if (status != 0) {
            fprintf(stderr, "could not allocate audio buffer: %d\n", status);
            exit(1);
        }
        buffers[i]->mAudioDataByteSize = BUFFER_SIZE * sizeof(short);
        memset(buffers[i]->mAudioData, 0, buffers[i]->mAudioDataByteSize);
        AudioQueueEnqueueBuffer(aq, buffers[i], 0, NULL);
    }
    status = AudioQueueStart(aq, NULL);
    if (status != 0) {
        fprintf(stderr, "could not start output audio queue: %d\n", status);
        exit(1);
    }
}

PcmOutputMacOSX::~PcmOutputMacOSX()
{
    //printf("destructor index=%d\n", index);
    sem_wait(sem_notfull);
    sem_post(sem_full);
    sem_wait(sem_notfull);
    if (aq != NULL) {
        AudioQueueDispose(aq, false);
    }
    sem_close(sem_notfull);
    sem_close(sem_full);
}

void PcmOutputMacOSX::output(const short *buf, int n)
{
    while (n > 0) {
        //printf("output %d index=%d\n", n, index);
        sem_wait(sem_notfull);
        int take = BUFFER_SIZE - index;
        if (n < take) {
            take = n;
        }
        memcpy(buffer + index, buf, take * sizeof(short));
        index += take;
        buf += take;
        n -= take;
        if (index < BUFFER_SIZE) {
            sem_post(sem_notfull);
        } else {
            sem_post(sem_full);
        }
    }
}

void PcmOutputMacOSX::flush()
{
}

void PcmOutputMacOSX::callback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
    //printf("callback %p\n", inBuffer);
    PcmOutputMacOSX *This = reinterpret_cast<PcmOutputMacOSX *>(inUserData);
    sem_wait(This->sem_full);
    //printf("  index=%d\n", This->index);
    inBuffer->mAudioDataByteSize = This->index * sizeof(short);
    memcpy(inBuffer->mAudioData, This->buffer, inBuffer->mAudioDataByteSize);
    AudioQueueEnqueueBuffer(This->aq, inBuffer, 0, NULL);
    if (This->index < BUFFER_SIZE) {
        //printf("stop\n");
        AudioQueueStop(This->aq, false);
    }
    This->index = 0;
    sem_post(This->sem_notfull);
}

PcmOutput *makePcmOutputMacOSX(uint32_t rate)
{
    return new PcmOutputMacOSX(rate);
}
