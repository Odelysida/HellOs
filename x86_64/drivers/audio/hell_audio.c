/*
 * HellOS 3-Channel Audio Driver
 * Square, Sine, and Sawtooth waveforms
 */

#include "../../kernel/kernel.h"
#include "../../kernel/audio.h"
#include "../../kernel/memory.h"
#include <stdint.h>

// Audio constants
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024
#define MAX_CHANNELS 3

// Audio state
static audio_state_t audio_state;
static audio_channel_t channels[MAX_CHANNELS];
static int16_t audio_buffer[BUFFER_SIZE];
static uint32_t buffer_position = 0;

// Waveform lookup tables
static int16_t sine_table[256];
static int16_t square_table[256];
static int16_t saw_table[256];

// PC Speaker and Audio hardware ports
#define PIT_CHANNEL_2   0x42
#define PIT_COMMAND     0x43
#define SPEAKER_PORT    0x61

/*
 * Initialize the audio system
 */
int init_hell_audio_driver(void) {
    // Initialize audio state
    audio_state.sample_rate = SAMPLE_RATE;
    audio_state.channels = MAX_CHANNELS;
    audio_state.buffer_size = BUFFER_SIZE;
    audio_state.initialized = false;
    
    // Initialize channels
    for (int i = 0; i < MAX_CHANNELS; i++) {
        channels[i].frequency = 0;
        channels[i].waveform = WAVE_SINE;
        channels[i].volume = 0;
        channels[i].phase = 0;
        channels[i].active = false;
    }
    
    // Generate waveform lookup tables
    generate_waveform_tables();
    
    // Initialize PC Speaker
    init_pc_speaker();
    
    // Clear audio buffer
    memset(audio_buffer, 0, sizeof(audio_buffer));
    
    audio_state.initialized = true;
    return HELL_SUCCESS;
}

/*
 * Generate waveform lookup tables
 */
void generate_waveform_tables(void) {
    for (int i = 0; i < 256; i++) {
        // Sine wave (using integer approximation)
        // Simple sine approximation using lookup table approach
        if (i < 64) {
            sine_table[i] = (int16_t)(i * 512);  // Rising
        } else if (i < 128) {
            sine_table[i] = (int16_t)(32767 - ((i - 64) * 512));  // Falling
        } else if (i < 192) {
            sine_table[i] = (int16_t)(-(i - 128) * 512);  // Negative rising
        } else {
            sine_table[i] = (int16_t)(-32767 + ((i - 192) * 512));  // Negative falling
        }
        
        // Square wave
        square_table[i] = (i < 128) ? 32767 : -32767;
        
        // Sawtooth wave
        saw_table[i] = (int16_t)((i * 256) - 32768);
    }
}

/*
 * Initialize PC Speaker for basic audio output
 */
void init_pc_speaker(void) {
    // Configure PIT channel 2 for audio
    outb(PIT_COMMAND, 0xB6); // Channel 2, square wave, binary mode
    
    // Enable speaker
    uint8_t speaker_control = inb(SPEAKER_PORT);
    speaker_control |= 0x03; // Enable speaker and PIT channel 2
    outb(SPEAKER_PORT, speaker_control);
}

/*
 * Play a note on a specific channel
 */
void play_note(int channel, uint16_t frequency, uint8_t waveform, uint8_t volume) {
    if (channel < 0 || channel >= MAX_CHANNELS) return;
    if (!audio_state.initialized) return;
    
    channels[channel].frequency = frequency;
    channels[channel].waveform = waveform;
    channels[channel].volume = volume;
    channels[channel].phase = 0;
    channels[channel].active = true;
    
    // For channel 0, also use PC Speaker for compatibility
    if (channel == 0 && frequency > 0) {
        set_pc_speaker_frequency(frequency);
    }
}

/*
 * Stop a note on a specific channel
 */
void stop_note(int channel) {
    if (channel < 0 || channel >= MAX_CHANNELS) return;
    if (!audio_state.initialized) return;
    
    channels[channel].active = false;
    channels[channel].volume = 0;
    
    // Turn off PC Speaker if channel 0
    if (channel == 0) {
        silence_pc_speaker();
    }
}

/*
 * Set PC Speaker frequency
 */
void set_pc_speaker_frequency(uint16_t frequency) {
    if (frequency == 0) {
        silence_pc_speaker();
        return;
    }
    
    // Calculate PIT divisor
    uint16_t divisor = 1193180 / frequency;
    
    // Send divisor to PIT
    outb(PIT_CHANNEL_2, divisor & 0xFF);
    outb(PIT_CHANNEL_2, (divisor >> 8) & 0xFF);
}

/*
 * Silence PC Speaker
 */
void silence_pc_speaker(void) {
    uint8_t speaker_control = inb(SPEAKER_PORT);
    speaker_control &= 0xFC; // Disable speaker
    outb(SPEAKER_PORT, speaker_control);
}

/*
 * Generate audio samples for mixing
 */
int16_t generate_sample(int channel) {
    if (!channels[channel].active) return 0;
    
    audio_channel_t* ch = &channels[channel];
    int16_t sample = 0;
    
    // Calculate phase increment
    uint32_t phase_increment = (ch->frequency * 256) / SAMPLE_RATE;
    
    // Get sample from waveform table
    uint8_t table_index = (ch->phase >> 8) & 0xFF;
    
    switch (ch->waveform) {
        case WAVE_SINE:
            sample = sine_table[table_index];
            break;
        case WAVE_SQUARE:
            sample = square_table[table_index];
            break;
        case WAVE_SAW:
            sample = saw_table[table_index];
            break;
        default:
            sample = 0;
            break;
    }
    
    // Apply volume
    sample = (sample * ch->volume) / 255;
    
    // Update phase
    ch->phase += phase_increment;
    
    return sample;
}

/*
 * Mix all channels and update audio buffer
 */
void update_audio_system(void) {
    if (!audio_state.initialized) return;
    
    // Mix all active channels
    for (int i = 0; i < BUFFER_SIZE; i++) {
        int32_t mixed_sample = 0;
        
        // Mix all channels
        for (int ch = 0; ch < MAX_CHANNELS; ch++) {
            mixed_sample += generate_sample(ch);
        }
        
        // Clamp to 16-bit range
        if (mixed_sample > 32767) mixed_sample = 32767;
        if (mixed_sample < -32768) mixed_sample = -32768;
        
        audio_buffer[i] = (int16_t)mixed_sample;
    }
    
    // Update buffer position
    buffer_position = (buffer_position + 1) % BUFFER_SIZE;
}

/*
 * Play an audio sequence
 */
void play_audio_sequence(audio_note_t* sequence, int max_channels) {
    if (!audio_state.initialized) return;
    if (max_channels > MAX_CHANNELS) max_channels = MAX_CHANNELS;
    
    int channel = 0;
    
    while (sequence->frequency != 0 && channel < max_channels) {
        play_note(channel, sequence->frequency, sequence->waveform, 128);
        
        // Simple delay (in a real OS, this would be timer-based)
        for (volatile int i = 0; i < sequence->duration_ms * 1000; i++) {
            // Busy wait - not ideal but simple
        }
        
        stop_note(channel);
        
        channel = (channel + 1) % max_channels;
        sequence++;
    }
}

/*
 * Create demonic sound effects
 */
void create_demonic_growl(int channel) {
    if (!audio_state.initialized) return;
    
    // Low frequency sawtooth for growl effect
    play_note(channel, 60, WAVE_SAW, 200);
    
    // Add some frequency modulation
    for (int i = 0; i < 100; i++) {
        uint16_t freq = 60 + (i % 20);
        channels[channel].frequency = freq;
        
        // Simple delay
        for (volatile int j = 0; j < 10000; j++);
    }
}

/*
 * Create fire crackling sound
 */
void create_fire_crackling(int channel) {
    if (!audio_state.initialized) return;
    
    // Random high frequency square waves
    for (int i = 0; i < 50; i++) {
        uint16_t freq = 1000 + (i * 17) % 500; // Pseudo-random
        play_note(channel, freq, WAVE_SQUARE, 100);
        
        // Short burst
        for (volatile int j = 0; j < 5000; j++);
        stop_note(channel);
        
        // Brief pause
        for (volatile int j = 0; j < 2000; j++);
    }
}

/*
 * Initialize audio system (called from kernel)
 */
void init_audio_system(void) {
    // This is called from kernel initialization
    // Actual driver initialization happens in init_hell_audio_driver
}

/*
 * Shutdown audio driver
 */
void shutdown_audio_driver(void) {
    if (audio_state.initialized) {
        // Stop all channels
        for (int i = 0; i < MAX_CHANNELS; i++) {
            stop_note(i);
        }
        
        // Silence PC Speaker
        silence_pc_speaker();
        
        audio_state.initialized = false;
    }
}

/*
 * Get current audio state
 */
audio_state_t* get_audio_state(void) {
    return &audio_state;
}

/*
 * Integer-based sine approximation (no longer needed)
 */ 