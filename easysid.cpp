#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sid.h"

#include "pcm.h"

#ifdef _MSC_VER
#define EXPORT _declspec(dllexport)
#else
#define EXPORT
#endif

SID sid;
PcmOutput *pcm;

extern "C" {

EXPORT void set_freq(uint8_t voice, uint16_t freq)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    sid.write(7*voice + 0, freq & 0xff);
    sid.write(7*voice + 1, (freq >> 8) & 0xff);
}

EXPORT void set_pw(uint8_t voice, uint16_t pw)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    sid.write(7*voice + 2, pw & 0xff);
    sid.write(7*voice + 3, (pw >> 8) & 0x0f);
}

EXPORT void set_wave(uint8_t voice, uint8_t wave)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    if (wave > 4) {
        return;
    }
    char reg = sid.read_state().sid_register[7*voice + 4];
    reg &= ~0xf0;
    reg |= 0x10 << wave;
    sid.write(7*voice + 4, reg);
}

EXPORT void set_test(uint8_t voice, bool test)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[7*voice + 4];
    reg &= ~0x08;
    if (test) {
        reg |= 0x08;
    }
    sid.write(7*voice + 4, reg);
}

EXPORT void set_ring(uint8_t voice, bool ring)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[7*voice + 4];
    reg &= ~0x04;
    if (ring) {
        reg |= 0x04;
    }
    sid.write(7*voice + 4, reg);
}

EXPORT void set_sync(uint8_t voice, bool sync)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[7*voice + 4];
    reg &= ~0x02;
    if (sync) {
        reg |= 0x02;
    }
    sid.write(7*voice + 4, reg);
}

EXPORT void set_gate(uint8_t voice, bool gate)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[7*voice + 4];
    reg &= ~0x01;
    if (gate) {
        reg |= 0x01;
    }
    sid.write(7*voice + 4, reg);
}

EXPORT void set_adsr(uint8_t voice, uint8_t attack, uint8_t decay, uint8_t sustain, uint8_t release)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    sid.write(7*voice + 5, ((attack & 0x0f) << 4) | (decay & 0x0f));
    sid.write(7*voice + 6, ((sustain & 0x0f) << 4) | (release & 0x0f));
}

EXPORT void set_volume(uint8_t volume)
{
    char reg = sid.read_state().sid_register[24];
    reg &= ~0x0f;
    reg |= volume & 0x0f;
    sid.write(24, reg);
}

EXPORT void run_ms(uint32_t ms)
{
    if (pcm == NULL) {
        pcm = makePcmOutputAudio();
    }
    const size_t buflength = 16384;
    short buf[buflength];
    size_t bufindex = 0;
    cycle_count delta_t = ms * 1000;
    while (delta_t > 0) {
        bufindex += sid.clock(delta_t, buf + bufindex, buflength - bufindex);
        pcm->output(buf, bufindex);
        bufindex = 0;
    }
}

} // extern "C"

#if 0
int main()
{
    set_freq(1, 7382);
    set_pw(1, 2048);
    set_wave(1, 0);
    set_gate(1, true);
    set_adsr(1, 8, 0, 15, 0);
    set_volume(15);
    run_ms(1000);
}
#endif
