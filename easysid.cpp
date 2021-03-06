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
PcmOutput *pcm = NULL;
PcmOutput *wav = NULL;

extern "C" {

EXPORT void start_capture(const char *fn)
{
    if (wav != NULL) {
        // Deleting the existing object will flush, and close the current capture file.
        delete wav;
        wav = NULL;
    }
    wav = makePcmOutputWav(fn);
}

EXPORT void end_capture()
{
    if (wav != NULL) {
        // Deleting the existing object will flush, and close the current capture file.
        delete wav;
        wav = NULL;
    }
}

EXPORT void set_reg(uint8_t reg, uint8_t value)
{
    if (reg <= 24) {
        sid.write(reg, value);
    }
}

reg8 voice_reg_base(uint8_t voice)
{
    return 7 * (voice - 1);
}

EXPORT void set_freq(uint8_t voice, uint16_t freq)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    sid.write(voice_reg_base(voice) + 0, freq & 0xff);
    sid.write(voice_reg_base(voice) + 1, (freq >> 8) & 0xff);
}

EXPORT void set_pw(uint8_t voice, uint16_t pw)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    sid.write(voice_reg_base(voice) + 2, pw & 0xff);
    sid.write(voice_reg_base(voice) + 3, (pw >> 8) & 0x0f);
}

EXPORT void set_wave(uint8_t voice, uint8_t wave)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    if (wave > 0xF) {
        return;
    }
    char reg = sid.read_state().sid_register[voice_reg_base(voice) + 4];
    reg &= ~0xf0;
    reg |= (wave << 4);
    sid.write(voice_reg_base(voice) + 4, reg);
}

EXPORT void set_test(uint8_t voice, bool test)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[voice_reg_base(voice) + 4];
    reg &= ~0x08;
    if (test) {
        reg |= 0x08;
    }
    sid.write(voice_reg_base(voice) + 4, reg);
}

EXPORT void set_ring(uint8_t voice, bool ring)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[voice_reg_base(voice) + 4];
    reg &= ~0x04;
    if (ring) {
        reg |= 0x04;
    }
    sid.write(voice_reg_base(voice) + 4, reg);
}

EXPORT void set_sync(uint8_t voice, bool sync)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[voice_reg_base(voice) + 4];
    reg &= ~0x02;
    if (sync) {
        reg |= 0x02;
    }
    sid.write(voice_reg_base(voice) + 4, reg);
}

EXPORT void set_gate(uint8_t voice, bool gate)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[voice_reg_base(voice) + 4];
    reg &= ~0x01;
    if (gate) {
        reg |= 0x01;
    }
    sid.write(voice_reg_base(voice) + 4, reg);
}

EXPORT void set_adsr(uint8_t voice, uint8_t attack, uint8_t decay, uint8_t sustain, uint8_t release)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    sid.write(voice_reg_base(voice) + 5, ((attack & 0x0f) << 4) | (decay & 0x0f));
    sid.write(voice_reg_base(voice) + 6, ((sustain & 0x0f) << 4) | (release & 0x0f));
}

EXPORT void set_filter_freq(uint16_t freq)
{
    sid.write(21, freq & 0x07);
    sid.write(22, (freq >> 3) & 0xff);
}

EXPORT void set_filter_res(uint8_t res)
{
    char reg = sid.read_state().sid_register[23];
    reg &= ~0xf0;
    reg |= (res << 4) & 0x0f;
    sid.write(23, reg);
}

EXPORT void set_filter(uint8_t voice, bool enable)
{
    if (voice < 1 || voice > 3) {
        return;
    }
    char reg = sid.read_state().sid_register[23];
    reg8 mask = (1 << (voice - 1));
    reg &= ~mask;
    if (enable) {
        reg |= mask;
    }
    sid.write(23, reg);
}

EXPORT void set_filter_external(bool enable)
{
    char reg = sid.read_state().sid_register[23];
    reg &= ~0x08;
    if (enable) {
        reg |= 0x08;
    }
    sid.write(23, reg);
}

EXPORT void set_filter_lp(bool enable)
{
    char reg = sid.read_state().sid_register[24];
    reg &= ~0x10;
    if (enable) {
        reg |= 0x10;
    }
    sid.write(24, reg);
}

EXPORT void set_filter_bp(bool enable)
{
    char reg = sid.read_state().sid_register[24];
    reg &= ~0x20;
    if (enable) {
        reg |= 0x20;
    }
    sid.write(24, reg);
}

EXPORT void set_filter_hp(bool enable)
{
    char reg = sid.read_state().sid_register[24];
    reg &= ~0x40;
    if (enable) {
        reg |= 0x40;
    }
    sid.write(24, reg);
}

EXPORT void set_voice3_mute(bool mute)
{
    char reg = sid.read_state().sid_register[24];
    reg &= ~0x80;
    if (mute) {
        reg |= 0x80;
    }
    sid.write(24, reg);
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
        // If we are capturing to a file, output the data to the file, then the sound device.
        if (wav != NULL) {
            wav->output(buf, bufindex);
        }
        pcm->output(buf, bufindex);
        bufindex = 0;
    }
}

} // extern "C"

#if 0
int main()
{
    start_capture("EasySIDTest.wav");
    set_freq(1, 7382);
    set_pw(1, 2048);
    set_wave(1, 0);
    set_gate(1, true);
    set_adsr(1, 8, 0, 15, 0);
    set_volume(15);
    run_ms(1000);
    end_capture();
}
#endif
