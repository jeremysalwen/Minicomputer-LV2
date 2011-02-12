#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lo/lo.h>
#include <string.h>
#include <lv2.h>
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


static const float anti_denormal = 1e-20;// magic number to get rid of denormalizing
typedef struct _engine {
	float delayBuffer[96000] __attribute__((aligned (16)));
	float parameter[_PARACOUNT] __attribute__((aligned (16)));
	float modulator[_MODCOUNT] __attribute__((aligned (16)));
	float EG[8][8] __attribute__((aligned (16))); // 7 8
	float EGFaktor[8] __attribute__((aligned (16)));
	float phase[4] __attribute__((aligned (16)));//=0.f;
	unsigned int choice[_CHOICEMAX] __attribute__((aligned (16)));
	int EGrepeat[8] __attribute__((aligned (16)));
	unsigned int EGtrigger[8] __attribute__((aligned (16)));
	unsigned int EGstate[8] __attribute__((aligned (16)));	
	float midif[_MULTITEMP]  __attribute__((aligned (16)));
	float high[4],band[4],low[4],f[4],q[4],v[4],faktor[4];

	float  *port; // _multitemp * ports + 2 mix and 2 aux

	unsigned int lastnote;
	int delayI,delayJ;
} engine;

typedef struct _minicomputer {
	engine engines[_MULTITEMP];
	// variables
	float table [_WAVECOUNT][TableSize] __attribute__((aligned (16)));

	float midi2freq [128]  __attribute__((aligned (16)));
	float * MixLeft_p;
	float *MixRight_p;
	float *Aux1_p;
	float *Aux2_p;

	float temp=0.f,lfo;
	float sampleRate=48000.0f; // only default, going to be overriden by the actual, taken from jack
	float tabX = 4096.f / 48000.0f;
	float srate = 3.145f/ 48000.f;
	float srDivisor = 1.f / 48000.f*100000.f;
	int i,delayBufferSize=0,maxDelayBufferSize=0,maxDelayTime=0;
	jack_nframes_t 	bufsize;
} minicomputer;

#define MINICOMPUTER_URI "urn:malte.steiner:plugins:minicomputer"

const LV2_Descriptor * miniDescriptor ={.URI=MINCOMPUTER_URI, 
	.activate=NULL,
	.cleanup=NULL,
	.connect_port=connect_port_minicomputer, 
	.deactivate=NULL, .activate=NULL, 
	.instantiate=init,
	.run=run_minicomputer, 
	.extension_data=NULL};
