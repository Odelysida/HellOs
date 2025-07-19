/*
 * HellOS Audio System Header
 * Audio subsystem definitions and structures
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>

// Audio state structure
typedef struct {
    uint32_t sample_rate;
    uint8_t channels;
    uint16_t buffer_size;
    bool initialized;
} audio_state_t;

// Audio channel structure
typedef struct {
    uint16_t frequency;
    uint8_t waveform;
    uint8_t volume;
    uint32_t phase;
    bool active;
} audio_channel_t;

// Function prototypes
void generate_waveform_tables(void);
void init_pc_speaker(void);
void play_note(int channel, uint16_t frequency, uint8_t waveform, uint8_t volume);
void stop_note(int channel);
void set_pc_speaker_frequency(uint16_t frequency);
void silence_pc_speaker(void);
int16_t generate_sample(int channel);
void create_demonic_growl(int channel);
void create_fire_crackling(int channel);
void shutdown_audio_driver(void);
audio_state_t* get_audio_state(void);

#endif // AUDIO_H 