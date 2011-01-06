#include <jack/jack.h>
//#include <jack/midiport.h> // later we use the jack midi ports to, but not this time
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lo/lo.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
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

typedef struct _minicomputer {
// variables
float delayBuffer[_MULTITEMP][96000] __attribute__((aligned (16)));
float table [_WAVECOUNT][TableSize] __attribute__((aligned (16)));
float parameter[_MULTITEMP][_PARACOUNT] __attribute__((aligned (16)));
float modulator[_MULTITEMP][_MODCOUNT] __attribute__((aligned (16)));
float midi2freq [128],midif[_MULTITEMP] __attribute__((aligned (16)));
float EG[_MULTITEMP][8][8] __attribute__((aligned (16))); // 7 8
float EGFaktor[_MULTITEMP][8] __attribute__((aligned (16)));
float phase[_MULTITEMP][4] __attribute__((aligned (16)));//=0.f;
unsigned int choice[_MULTITEMP][_CHOICEMAX] __attribute__((aligned (16)));
int EGrepeat[_MULTITEMP][8] __attribute__((aligned (16)));
unsigned int EGtrigger[_MULTITEMP][8] __attribute__((aligned (16)));
unsigned int EGstate[_MULTITEMP][8] __attribute__((aligned (16)));
float high[_MULTITEMP][4],band[_MULTITEMP][4],low[_MULTITEMP][4],f[_MULTITEMP][4],q[_MULTITEMP][4],v[_MULTITEMP][4],faktor[_MULTITEMP][4];
jack_port_t   *port[_MULTITEMP + 4]; // _multitemp * ports + 2 mix and 2 aux
unsigned int lastnote[_MULTITEMP];
int delayI[_MULTITEMP],delayJ[_MULTITEMP];


float temp=0.f,lfo;
float sampleRate=48000.0f; // only default, going to be overriden by the actual, taken from jack
float tabX = 4096.f / 48000.0f;
float srate = 3.145f/ 48000.f;
float srDivisor = 1.f / 48000.f*100000.f;
int i,delayBufferSize=0,maxDelayBufferSize=0,maxDelayTime=0;
jack_nframes_t 	bufsize;
} minicomputer;
