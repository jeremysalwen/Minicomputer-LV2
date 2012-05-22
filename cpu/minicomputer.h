#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lo/lo.h>
#include <string.h>
#include <lv2.h>
#include "event-helpers.h"
#include "uri-map.h"

// some common definitions
#include "../common.h" 

// defines
#define _MODCOUNT 32
#define _WAVECOUNT 32
#define _CHOICEMAX 16
#define _MULTITEMP 8
#define TableSize 4096
#define tabM 4095
#define tabF 4096.f

#define NUM_MIDI 127

#define MIDI_COMMANDMASK 0xF0
#define MIDI_CHANNELMASK 0x0F

#define MIDI_NOTEON 0x90
#define MIDI_NOTEOFF 0x80
#define MIDI_CONTROL 0xB0
#define MIDI_PITCHBEND 0xE0
#define MIDI_CHANPRESS 0xD0

static const float anti_denormal = 1e-20;// magic number to get rid of denormalizing

typedef struct _modulator_selector {
	float type_p;
	float amount_p;
} mod_selector;

typedef struct _oscillator_params {
	float waveform_p;
	float volume_p;
	
	float fix_frequency_p; //This selects whether we use a fixed frequency or not
	float fixed_frequency_p; //This is the fixed frequency we might use
	float tune_frequency_p; //Or if we don't use fixed frequency, this is the offset from the MIDI NOTE
	
	float boost_modulation_p;
	float boost_factor;
	
	mod_selector freq_mod1;
	mod_selector freq_mod2;
	mod_selector amp_mod1;
	mod_selector amp_mod2;
	
	float fm_output_vol_p;
} oscillator_params;

typedef struct _envelope_settings {
	float attack_p __attribute__((aligned (16)));
	float decay_p __attribute__((aligned (16)));
	float sustain_p __attribute__((aligned (16)));
	float release_p __attribute__((aligned (16)));
	
	int EGrepeat_p __attribute__((aligned (16)));
} envelope_settings;

typedef struct _envelope_generator {
	float state __attribute__((aligned (16)));
	float Faktor __attribute__((aligned (16)));
	
	unsigned int EGtrigger __attribute__((aligned (16)));
	unsigned int EGstate __attribute__((aligned (16)));
} EG;

typedef struct _filter_settings {
	float f_p; //frequency
	float q_p; //resonance
	float v_p; //volume
} filter_settings;

typedef struct _filter {
	float high;
	float band
	float low;
} filter;

typedef struct _engine {
	float phase1;
	float phase2;
	
	EG envelope_generator[8];
	
	filter filt;
	
	float delayBuffer[96000] __attribute__((aligned (16)));

	float midif  __attribute__((aligned (16)));

	float  *port; // _multitemp * ports + 2 mix and 2 aux

	unsigned int lastnote;
	int delayI,delayJ;
} engine;

struct _engineblock;

typedef struct _listheader {
	struct _engineblock * next;
	struct _engineblock * previous;
} listheader;

typedef struct _engineblock {
	listheader h;
	engine e;
}engineblock;


static float table [_WAVECOUNT][TableSize] __attribute__((aligned (16)));
static float midi2freq [128]  __attribute__((aligned (16)));

typedef struct _minicomputer {
	engineblock * noteson[NUM_MIDI];
	listheader freeblocks;
	engineblock* inuse;
	
	filter filters[3];

	filter_settings filt_settings[3][2];
	mod_selector morph_mod1;
	mod_selector morph_mod2;
	envelope_settings ES;
	oscillator_params osc1;
	oscillator_params osc2;
	float osc2_sync_p;
	mod_selector amp_mod;
	
	
	mod_selector mod_osc;
	float mod_osc_phase;
	
	float delay_amount;
	float delay_time;
	float delay_feedback;
	float delay_volume;
	mod_selector delay_mod;
	
	
	float clear_filter;
	
	// variables
	float *MixLeft_p;
	float *MixRight_p;
	float *Aux1_p;
	float *Aux2_p;

	LV2_Event_Buffer *MidiIn;
	LV2_Event_Iterator in_iterator;

	LV2_Event_Feature* event_ref;
	int midi_event_id;

	float lfo;
	float tabX ;
	float srate;
	float srDivisor ;
	int delayBufferSize;
	int maxDelayTime;
	unsigned int bufsize;

	float modulator[_MODCOUNT] __attribute__((aligned (16)));

	lo_server_thread st;
} minicomputer;

#define MINICOMPUTER_URI "urn:malte.steiner:plugins:minicomputer"

static void connect_port_minicomputer(LV2_Handle instance, uint32_t port, void *data);

static LV2_Handle instantiateMinicomputer(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features);
static void run_minicomputer(LV2_Handle instance, uint32_t nframes);
static void cleanupMinicomputer(LV2_Handle instance);

const LV2_Descriptor miniDescriptor ={
	.URI=MINICOMPUTER_URI, 
	.activate=NULL,
	.cleanup=cleanupMinicomputer,
	.connect_port=connect_port_minicomputer,
	.deactivate=NULL,
	.activate=NULL,
	.instantiate=instantiateMinicomputer,
	.run=run_minicomputer,
	.extension_data=NULL
};
