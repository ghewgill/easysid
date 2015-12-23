#include <iso646.h>
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

bool initAudioOutput(uint32_t sample_rate)
{
    if (pcm != NULL) {
        delete pcm;
    }
    pcm = makePcmOutputAudio(sample_rate);
    return pcm != NULL;
}

extern "C" {

// Clock speed defaults
EXPORT const uint32_t DEFAULT_SAMPLING_RATE    =   44100; // Hz
EXPORT const double   DEFAULT_NTSC_CLOCK_SPEED = 1022730; // Hz
EXPORT const double   DEFAULT_PAL_CLOCK_SPEED  =  985248; // Hz

EXPORT void start_capture(const char *fn)
{
    if (wav != NULL) {
        // Deleting the existing object will flush, and close the current capture file.
        delete wav;
        wav = NULL;
    }
    if (pcm == NULL) {
        if (not initAudioOutput(DEFAULT_SAMPLING_RATE)) {
            fprintf(stderr, "Failed to initialize audio output device.");
            // TODO: Error condition
            return;
        }
    }
    wav = makePcmOutputWav(fn, pcm->getSampleRate());
}

EXPORT void end_capture()
{
    if (wav != NULL) {
        // Deleting the existing object will flush, and close the current capture file.
        delete wav;
        wav = NULL;
    }
}

EXPORT void set_chip(uint32_t chip)
{
    if (chip < 0 || chip > 1) {
        fprintf(stderr, "Invalid SID Chip: %u\n", chip);
        // TODO: Implement error checking...
        return;
    }
    sid.set_chip_model(static_cast<chip_model>(chip));
}

EXPORT void set_sampling_parameters(double clock, uint32_t sample_mode, uint32_t sample_freq)
{
    if (sample_mode > 3) {
        fprintf(stderr, "Invalid Sampling Mode: %u\n", sample_mode);
        // TODO: Implement error checking...
        return;
    }
    if (sample_freq > 352800) { 
        // This is only an attempt to set some reasonable limit, as the audio hardware will
        // likely fail if value is out of its range.  352800 is defined as:  Digital eXtreme 
        // Definition, used for recording and editing Super Audio CDs.  Eight times the frequency of 44.1 kHz.
        fprintf(stderr, "Invalid Sampling frequency: %u\n", sample_freq);
        // TODO: Implement error checking...
        return;    
    }
    if (sid.set_sampling_parameters(clock, static_cast<sampling_method>(sample_mode), static_cast<double>(sample_freq))) {
        if (not initAudioOutput(sample_freq)) {
            fprintf(stderr, "initAudioOutput failed.\n");
            // TODO: Implement error checking...
            return;
        }
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
        if (not initAudioOutput(DEFAULT_SAMPLING_RATE)) {
            fprintf(stderr, "Cannot perform run_ms().  initAudioOutput failed.");
            // TODO: Implement error checking...
            return;
        }
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
// The following is a VERY simplistic example of how to use EasySID to interact with the reSID emulation library.
// It plays a single tone, on voice 1, for 1 second, leaving time for the RELEASE phase of the ADSR envelope.
int main()
{
    // Setup our Chip emulation, and sample parameters...
    set_chip(MOS6581);
    set_sample_parameters(DEFAULT_NTSC_CLOCK_SPEED, SAMPLE_RESAMPLE_INTERPOLATE, DEFAULT_SAMPLING_RATE);
    // Capture the audio output in a .wav file for diagnostic...
    start_capture("EasySIDTest.wav");
    // Set the SID volume to max..
    set_volume(15);
    // Set our Frequency, pulsewidth, and waveform for voice 1...
    set_freq(1, 7382);
    set_pw(1, 2048);
    set_wave(1, 0);
    // Set the ADSR envelope for the note we're about to play...
    set_adsr(1, 8, 0, 15, 0);
    // Play the note by setting the GATE on voice 1 to true.
    set_gate(1, true);
    // Generate 1000ms of SID sound...
    run_ms(1000);
    // Stop the note, by setting the GATE to false, which will begin the notes RELEASE portion of the ADSR envelope.
    set_gate(1, false);
    // Allow 1000ms for the SID to finish playing the note...
    run_ms(1000);
    // Close the capture file.
    end_capture();
}
#endif
