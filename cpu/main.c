/** Minicomputer
 * industrial grade digital synthesizer
 *
 * Copyright 2007 Malte Steiner
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

#define _MODCOUNT 32
#define _WAVECOUNT 32
#define _CHOICEMAX 16
#define _MULTITEMP 8
#define TableSize 4096
#define tabM 4095
#define tabF 4096.f
float delayBuffer[_MULTITEMP][96000] __attribute__((aligned (16)));
float table [_WAVECOUNT][TableSize] __attribute__((aligned (16)));
float parameter[_MULTITEMP][_PARACOUNT] __attribute__((aligned (16)));
float modulator[_MULTITEMP][_MODCOUNT] __attribute__((aligned (16)));
float midi2freq [128],midif[_MULTITEMP] __attribute__((aligned (16)));
float EG[_MULTITEMP][8][8] __attribute__((aligned (16))); // 7 8
float phase[_MULTITEMP][4] __attribute__((aligned (16)));//=0.f;
unsigned int choice[_MULTITEMP][_CHOICEMAX] __attribute__((aligned (16)));
int EGrepeat[_MULTITEMP][8] __attribute__((aligned (16)));
unsigned int EGtrigger[_MULTITEMP][8] __attribute__((aligned (16)));
unsigned int EGstate[_MULTITEMP][8] __attribute__((aligned (16)));
float high[_MULTITEMP][4],band[_MULTITEMP][4],low[_MULTITEMP][4],f[_MULTITEMP][4],q[_MULTITEMP][4],v[_MULTITEMP][4],faktor[_MULTITEMP][4];
jack_port_t   *port[_MULTITEMP + 4]; // _multitemp * ports + 2 mix and 2 aux
unsigned int lastnote[_MULTITEMP];
int delayI[_MULTITEMP],delayJ[_MULTITEMP];

snd_seq_t *open_seq();
 snd_seq_t *seq_handle;
  int npfd;
  struct pollfd *pfd;
#ifdef _VECTOR  
  typedef float v4sf __attribute__ ((vector_size(16),aligned(16)));//((mode(V4SF))); // vector of four single floats

union f4vector 
{
  v4sf v;// __attribute__((aligned (16)));
  float f[4];// __attribute__((aligned (16)));
};
#endif
//void midi_action(snd_seq_t *seq_handle);


snd_seq_t *open_seq() {

  snd_seq_t *seq_handle;
  int portid;

  if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "Minicomputer");
  if ((portid = snd_seq_create_simple_port(seq_handle, "Minicomputer",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return(seq_handle);
}


//  gcc -o synthesizer synth2.c -ljack -ffast-math -O3 -march=k8 -mtune=k8 -funit-at-a-time -fpeel-loops -ftracer -funswitch-loops -llo -lasound
static inline void error(int num, const char *m, const char *path); 
static inline int generic_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data); 
static inline int foo_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data); 

static inline int quit_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);



jack_port_t* inbuf;
jack_client_t *client;

float temp=0.f,lfo;
float sampleRate=48000.0f; // only default, going to be overriden by the actual, taken from jack
float tabX = 4096.f / 48000.0f;
float srate = 3.145f/ 48000.f;
int i,delayBufferSize=0,maxDelayBufferSize=0,maxDelayTime=0;
jack_nframes_t 	bufsize;
int done = 0;
static const float anti_denormal = 1e-20;
/* inlined manually
static inline float Oscillator(float frequency,int wave,float *phase)
{
    int i = (int) *phase ;// float to int, cost some cycles

	i=i&tabM;//i%=TableSize;
	//if (i>tabM) i=tabM;
	if (i<0) i=tabM;
    *phase += tabX * frequency;

    if(*phase >= tabF)
    {
   		 *phase -= tabF;
		// if (*phase>=tabF) *phase = 0; //just in case of extreme fm
    }


        if(*phase < 0.f)
                {
                	*phase += tabF;
        	//	if(*phase < 0.f) *phase = tabF-1;
                }
        return table[wave][i] ;
}
*/
//inline float filter (float input,float f, float q)
//{
//
//
//}
/**
 * start the envelope generator
 * called by a note on event for that voice
 * @param the voice number
 * @param the number of envelope generator
 */

static inline void egStart (unsigned int voice,unsigned int number)
{
	     EGtrigger[voice][number]=1;
	     EG[voice][number][0] = 1.f; // triggerd
		 EG[voice][number][5] = 1.f; // target
         EG[voice][number][7] = 0.0f;// state
         EGstate[voice][number] = 0;// state
		 //printf("start %i", voice);
}
/**
 * set the envelope to release mode
 * should be called by a related noteoff event
 * @param the voice number
 * @param the number of envelope generator
 */
static inline void egStop (unsigned int voice,unsigned int number)
{
	    // if (EGrepeat[voice][number] == 0) 
	    EGtrigger[voice][number] = 0; // triggerd
		 EGstate[voice][number] = 0; // target
		// printf("stop %i", voice);
}
/**
 * calculate the envelope, done in audiorate to avoide zippernoise
 * @param the voice number
 * @param the number of envelope generator
*/
static inline float egCalc (unsigned int voice, unsigned int number)
{
	/* EG[x] x:
	 * 0 = trigger
	 * 1 = attack
	 * 2 = decay
	 * 3 = sustain
	 * 4 = release
	 * 5 = target
	 * 6 = state
	 */
	int i = EGstate[voice][number]; 
	if (EGtrigger[voice][number] != 1)
	{
		if (i == 1){ // attack
			 EG[voice][number][6] += EG[voice][number][1];
			 if (EG[voice][number][6]>=1.0f)
			 {
			 	EG[voice][number][6]=1.0f;
			 	EGstate[voice][number]=2;
			 }
		}
		else if (i == 2)
		{ // decay
			if (EG[voice][number][6]>EG[voice][number][3])
			{
				EG[voice][number][6] -= EG[voice][number][2];
			}
			else 
			{
				if (EGrepeat[voice][number]==0)
				{
					EGstate[voice][number]=3; // stay on sustain
				}
				else
				{
					egStop(voice,number);// continue to release
				}
			}
			// what happens if sustain = 0? envelope should go in stop mode when decay reached ground
			if (EG[voice][number][6]<0.0f) 
		    	{	
		    		EG[voice][number][6]=0.0f;
		    		if (EGrepeat[voice][number]==0)
				{
					EGstate[voice][number]=4; // released
				}
				else
				{
					egStart(voice,number);// repeat
				}
		    	}

		} // end of decay
		else if ((i == 0) && (EG[voice][number][6]>0.0f))
		{
		    /* release */
		    
		    EG[voice][number][6] -= EG[voice][number][4];//*EG[number][6];
		    if (EG[voice][number][6]<0.0f) 
		    {	
		    	EG[voice][number][6]=0.0f;
		    	if (EGrepeat[voice][number]==0)
				{
					EGstate[voice][number]=4; // released
				}
				else
				{
					egStart(voice,number);// repeat
				}
		    }
		}
//		if (EG[number][5] == 1.0f){
//		    /* attack */
//		   
//		    EG[number][6] += EG[number][1]*(1.0f - EG[number][6]);
//		    EG[number][5] = (EG[number][6] > 1.0f) ? EG[number][3] : 1.0f;
//		}
//		else if ((EG[number][5] == 0.0f) && (EG[number][6]>0.0f))
//		{
//		    /* release */
//		    
//		    EG[number][6] -= EG[number][4];//*EG[number][6];
//		    if (EG[number][6]<0.0f) EG[number][6]=0.0f;
//		}
//		else{
//		    /* decay */
//		    EG[number][6] -= EG[number][2]*(EG[number][6]-EG[number][3]);
//		}
	}
	else
	{
	//	if (EGtrigger[voice][number] == 1) // declick ramp down processing
	//	{
			
		/*	EG[voice][number][6] -= 0.005f;
			if (EG[voice][number][6]<0.0f) 
			{  
				if  (EG[voice][number][7]< EG[voice][number][6])
				{
			   	 EG[voice][number][7] += EG[voice][number][1]*(1.0f - EG[voice][number][7]);
				}
			    else
			    {*/
					EGtrigger[voice][number] = 0;
					EGstate[voice][number] = 1;
		/*	    }
				
			}*/
	//	}
	//	else if (EG[voice][number][0] == 2.f) // actual start
	//	{
	//		EG[voice][number][0] = 0.f;
			
			EG[voice][number][0] = 1.f; // triggerd
		    EGstate[voice][number] = 1; // target
            //EG[voice][number][6] = 0.0f;// state
	//	}
		
	}
	return EG[voice][number][6];
}
//float d0,d1,d2,c1;

/* this is the heart of the client. the process callback. 
 * this will be called by jack every process cycle.
 * jack provides us with a buffer for out output port, 
 * which we can happily write into. inthis case we just 
 * fill it with 0's to produce.... silence! not to bad, eh? */
int process(jack_nframes_t nframes, void *arg) {
float tf,tf1,tf2,tf3,ta1,ta2,ta3,morph,mo,mf,result,tdelay,clib1,clib2;
float osc1,osc2,delayMod;
unsigned int currentvoice = 0;
unsigned int index;
typedef union
{
	int i;
	float f;
} INTORFLOAT;
INTORFLOAT P1 __attribute__((aligned (16)));
INTORFLOAT P2 __attribute__((aligned (16)));
INTORFLOAT P3 __attribute__((aligned (16)));
INTORFLOAT bias;
bias.i = (23 +127) << 23;
int iP1=0,iP2=0,iP3=0;
#ifdef _VECTOR
	union f4vector g __attribute__((aligned (16)));
	union f4vector h __attribute__((aligned (16)));
	union f4vector i __attribute__((aligned (16)));
	union f4vector j __attribute__((aligned (16)));
	g.f[1] = 2.f; g.f[2] = 2.f; g.f[3] = 2.f; // first entry differs always
	//g.f[1] = 2.f*srate; g.f[2] = 2.f*srate; g.f[3] = 2.f*srate; // first entry differs always
	i.f[0]=1.f; i.f[1] = srate; i.f[2] = srate; i.f[3] = srate; 
	h.f[0]=1.f; h.f[1] = 0.1472725f; h.f[2] = 0.1472725f; h.f[3] = 0.1472725f;

#endif
	float *bufferMixLeft = (float*) jack_port_get_buffer(port[8], nframes);
	float *bufferMixRight = (float*) jack_port_get_buffer(port[9], nframes);
	float *bufferAux1 = (float*) jack_port_get_buffer(port[10], nframes);
	float *bufferAux2 = (float*) jack_port_get_buffer(port[11], nframes);
/*jack_midi_port_info_t* info;
	void* buf;
	jack_midi_event_t ev;
	
	
	buf = jack_port_get_buffer(inbuf, bufsize);
	info = jack_midi_port_get_info(buf, bufsize);
	for(index=0; index<info->event_count; ++index)
	{
		jack_midi_event_get(&ev, buf, index, nframes);
	}
*/
	
		

	/* so we do it :) */
	for (index = 0; index < nframes; ++index) {
	/* this function returns a pointer to the buffer where 
     * we can write our frames samples */

		
	bufferMixLeft[index]=0.f;
	bufferMixRight[index]=0.f;
	bufferAux1[index]=0.f;
	bufferAux2[index]=0.f;
for (currentvoice=0;currentvoice<_MULTITEMP;++currentvoice) // for each voice
{		
//	float *buffer = (float*) jack_port_get_buffer(port[currentvoice], nframes);
//		buffer[index]=0.0f;

// calc the modulators
modulator [currentvoice][8] =egCalc(currentvoice,1);
modulator [currentvoice][9] =egCalc(currentvoice,2);
modulator [currentvoice][10]=egCalc(currentvoice,3);
modulator [currentvoice][11]=egCalc(currentvoice,4);
modulator [currentvoice][12]=egCalc(currentvoice,5);
modulator [currentvoice][13]=egCalc(currentvoice,6);
}
for (currentvoice=0;currentvoice<_MULTITEMP;++currentvoice) // for each voice
{
// get the parameter settings
   float * param = parameter[currentvoice];
P1.f =  phase[currentvoice][1];
P2.f =  phase[currentvoice][2];
P3.f =  phase[currentvoice][3];
P1.f += bias.f;
P2.f += bias.f;
P3.f += bias.f;
P1.i -= bias.i;
P2.i -= bias.i;
P3.i -= bias.i;
	iP1=P1.i&tabM;//i%=TableSize;
	iP2=P2.i&tabM;//i%=TableSize;
	iP3=P3.i&tabM;//i%=TableSize;
/*
int iP1 = (int) phase[currentvoice][1];// float to int, cost some cycles
int iP2 = (int) phase[currentvoice][2];// hopefully this got optimized by compiler
int iP3 = (int) phase[currentvoice][3];// hopefully this got optimized by compiler

	iP1=iP1&tabM;//i%=TableSize;
	iP2=iP2&tabM;//i%=TableSize;
	iP3=iP3&tabM;//i%=TableSize;
	*/
	//if (i>tabM) i=tabM;
	
	if (iP1<0) iP1=tabM;
	if (iP2<0) iP2=tabM;
	if (iP3<0) iP3=tabM;

    phase[currentvoice][3]+= tabX * param[90];
    if(phase[currentvoice][3]  >= tabF)
    {
   		  phase[currentvoice][3]-= tabF;
		// if (*phase>=tabF) *phase = 0; //just in case of extreme fm
    }
    if(phase[currentvoice][3]< 0.f)
                {
                	phase[currentvoice][3]+= tabF;
        	//	if(*phase < 0.f) *phase = tabF-1;
                }
//modulator [currentvoice][14]=Oscillator(parameter[currentvoice][90],choice[currentvoice][12],&phase[currentvoice][3]);
modulator [currentvoice][14] = table[choice[currentvoice][12]][iP3] ;

tf = param[1];
tf *=param[2];
ta1 = param[9];
ta1 *= modulator[currentvoice][choice[currentvoice][2]]; // osc1 first ampmod

//tf+=(midif[currentvoice]*parameter[currentvoice][2]*parameter[currentvoice][3]);
tf+=(midif[currentvoice]*(1.0f-param[2])*param[3]);
ta1+= param[11]*modulator[currentvoice][choice[currentvoice][3]];// osc1 second ampmod
tf+=(param[4]*param[5])*modulator[currentvoice][choice[currentvoice][0]];
tf+=param[7]*modulator[currentvoice][choice[currentvoice][1]];
//tf/=3.f;		
//ta/=2.f;
//static inline float Oscillator(float frequency,int wave,float *phase)
//{
 /*   int iP1 = (int) phase[currentvoice][1];// float to int, cost some cycles
    int iP2 = (int) phase[currentvoice][2];// hopefully this got optimized by compiler

	iP1=iP1&tabM;//i%=TableSize;
	iP2=iP2&tabM;//i%=TableSize;*/
	//if (i>tabM) i=tabM;
    phase[currentvoice][1]+= tabX * tf;
	
//	if (iP1<0) iP1=tabM;
//	if (iP2<0) iP2=tabM;

    if(phase[currentvoice][1]  >= tabF)
    {
   		  phase[currentvoice][1]-= tabF;
   		  //if (param[115]>0.f) phase[currentvoice][2]= 0; // sync osc2 to 1
		  // branchless sync:
		  phase[currentvoice][2]-= phase[currentvoice][2]*param[115];

		// if (*phase>=tabF) *phase = 0; //just in case of extreme fm
    }


        if(phase[currentvoice][1]< 0.f)
                {
                	phase[currentvoice][1]+= tabF;
        	//	if(*phase < 0.f) *phase = tabF-1;
                }
        osc1 = table[choice[currentvoice][4]][iP1] ;
//}
//osc1 = Oscillator(tf,choice[currentvoice][4],&phase[currentvoice][1]);
modulator[currentvoice][3]=osc1*(param[13]*(1.f+ta1));//+parameter[currentvoice][13]*ta1);

tf2 = param[16];
tf2 *=param[17];
ta2 = param[23];
ta2 *=modulator[currentvoice][choice[currentvoice][8]]; // osc2 first amp mod
//tf2+=(midif[currentvoice]*parameter[currentvoice][17]*parameter[currentvoice][18]);
tf2+=(midif[currentvoice]*(1.0f-param[17])*param[18]);
ta3 = param[25];
ta3 *=modulator[currentvoice][choice[currentvoice][9]];// osc2 second amp mod
tf2+=param[15]*param[19]*modulator[currentvoice][choice[currentvoice][6]];
tf2+=param[21]*modulator[currentvoice][choice[currentvoice][7]];
//tf/=3.f;		
//ta/=2.f;
modulator[currentvoice][4] = (param[28]+param[28]*ta3);// osc2 fm out
    phase[currentvoice][2]+= tabX * tf2;
    if(phase[currentvoice][2]  >= tabF)
    {
   		  phase[currentvoice][2]-= tabF;
		// if (*phase>=tabF) *phase = 0; //just in case of extreme fm
    }


        if(phase[currentvoice][2]< 0.f)
                {
                	phase[currentvoice][2]+= tabF;
        	//	if(*phase < 0.f) *phase = tabF-1;
                }
        osc2 = table[choice[currentvoice][5]][iP2] ;
//osc2 = Oscillator(tf2,choice[currentvoice][5],&phase[currentvoice][2]);
modulator[currentvoice][4] *= osc2;

// mix pre filter
//temp=(parameter[currentvoice][14]-parameter[currentvoice][14]*ta1);
temp=(param[14]*(1.f-ta1));
temp*=osc1;
temp+=osc2*(param[29]*(1.f-ta2));
temp*=0.5f;// get the volume of the sum into a normal range	
temp+=anti_denormal;
/* filter settings*/
mf =  (1.f-(param[38]*modulator[currentvoice][ choice[currentvoice][10]]));
mf+= (1.f-(param[48]*modulator[currentvoice][ choice[currentvoice][11]]));
mo = param[56]*mf;


clib1 = fabs (mo);
clib2 = fabs (mo-1.0f);
mo = clib1 + 1.0f;
mo -= clib2;
mo *= 0.5f;
/*
if (mo<0.f) mo = 0.f;
else if (mo>1.f) mo = 1.f;
*/

morph=(1.0f-mo);
/*
tf= (srate * (parameter[currentvoice][30]*morph+parameter[currentvoice][33]*mo) );
f[currentvoice][0] = 2.f * tf - (tf*tf*tf) * 0.1472725f;// / 6.7901358;

tf= (srate * (parameter[currentvoice][40]*morph+parameter[currentvoice][43]*mo) );
f[currentvoice][1] = 2.f * tf - (tf*tf*tf)* 0.1472725f; // / 6.7901358;;

tf = (srate * (parameter[currentvoice][50]*morph+parameter[currentvoice][53]*mo) );
f[currentvoice][2] = 2.f * tf - (tf*tf*tf) * 0.1472725f;// / 6.7901358; 
*/
// parallel calculation:
#ifdef _VECTOR
 union f4vector a __attribute__((aligned (16))), b __attribute__((aligned (16))),  c __attribute__((aligned (16))), d __attribute__((aligned (16))),e __attribute__((aligned (16)));

  b.f[0] = morph; b.f[1] = morph; b.f[2] = morph; b.f[3] = morph;
  a.f[0] = param[30]; a.f[1] =param[31]; a.f[2] = param[32]; a.f[3] = param[40];
  d.f[0] = param[41]; d.f[1] =param[42]; d.f[2] = param[50]; d.f[3] = param[51];

  c.v = a.v * b.v;
//c.v = __builtin_ia32_mulps (a.v, b.v);
  e.v = d.v * b.v;
//e.v = __builtin_ia32_mulps (d.v, b.v);

  tf1 = c.f[0];
  q[currentvoice][0]=c.f[1];
  v[currentvoice][0]=c.f[2];
  tf2 = c.f[3];



q[currentvoice][1] = e.f[0];
v[currentvoice][1] = e.f[1];
tf3 =  e.f[2];
q[currentvoice][2] = e.f[3];

#else
tf1= param[30];
q[currentvoice][0] = param[31];
v[currentvoice][0] = param[32];
tf2= param[40];

q[currentvoice][1] = param[41];
v[currentvoice][1] = param[42];
tf3 =  param[50];
q[currentvoice][2] = param[51];
#endif

v[currentvoice][2] = param[52];

#ifndef _VECTOR
tf1*= morph;
tf2*= morph;
q[currentvoice][0] *= morph;
v[currentvoice][0] *= morph;

tf3 *=  morph;
v[currentvoice][1] *= morph;
q[currentvoice][1] *= morph;
q[currentvoice][2] *= morph;
#endif

v[currentvoice][2] *= morph;

#ifdef _VECTOR
  a.f[0] = param[33]; a.f[1] =param[34]; a.f[2] = param[35]; a.f[3] = param[43];
  d.f[0] = param[44]; d.f[1] =param[45]; d.f[2] = param[53]; d.f[3] = param[54];
  b.f[0] = mo; b.f[1] = mo; b.f[2] = mo; b.f[3] = mo;
  c.v = a.v * b.v;
//c.v = __builtin_ia32_mulps (a.v, b.v);
  e.v = d.v * b.v;
//e.v = __builtin_ia32_mulps (d.v, b.v);

tf1+= c.f[0];
tf2+=c.f[3];
tf3 += e.f[2];
q[currentvoice][0] += c.f[1];//parameter[currentvoice][34]*mo;
q[currentvoice][1] += e.f[0];//parameter[currentvoice][44]*mo;
q[currentvoice][2] += e.f[3];//parameter[currentvoice][54]*mo;
v[currentvoice][0] += c.f[2];//parameter[currentvoice][35]*mo;
v[currentvoice][1] += e.f[1];//parameter[currentvoice][45]*mo;

#else
tf1+= param[33]*mo;
tf2+=param[43]*mo;
tf3 += param[53]*mo;
#endif


#ifndef _VECTOR
q[currentvoice][0] += param[34]*mo;
q[currentvoice][1] += param[44]*mo;
q[currentvoice][2] += param[54]*mo;

v[currentvoice][0] += param[35]*mo;
v[currentvoice][1] += param[45]*mo;

tf1*=srate;
tf2*=srate;
tf3 *= srate;
#endif

#ifdef _VECTOR
// prepare next calculations

  a.f[0] = param[55]; a.f[1] =tf1; a.f[2] = tf2; a.f[3] = tf3;
  g.f[0] = mo;// b.f[1] = 2.f; b.f[2] = 2.f; b.f[3] = 2.f;
  j.v = a.v * i.v; // tf * srate
  c.v = j.v * g.v; // tf * 2
//c.v = __builtin_ia32_mulps (a.v, g.v);

v[currentvoice][2] += c.f[0];//parameter[currentvoice][55]*mo;

//f[currentvoice][0] = c.f[1];//2.f * tf1;
//f[currentvoice][1] = c.f[2];//2.f * tf2;
//f[currentvoice][2] = c.f[3];//2.f * tf3;
//pow(c.v,3);
d.v = c.v - ((j.v * j.v * j.v) * h.v);

f[currentvoice][0] = d.f[1];//(tf1*tf1*tf1) * 0.1472725f;// / 6.7901358;

f[currentvoice][1] = d.f[2];//(tf2*tf2*tf2)* 0.1472725f; // / 6.7901358;;

f[currentvoice][2] = d.f[3];//(tf3*tf3*tf3) * 0.1472725f;// / 6.7901358; 
#else
v[currentvoice][2] += param[55]*mo;

f[currentvoice][0] = 2.f * tf1;
f[currentvoice][1] = 2.f * tf2;
f[currentvoice][2] = 2.f * tf3; 

f[currentvoice][0] -= (tf1*tf1*tf1) * 0.1472725f;// / 6.7901358;

f[currentvoice][1] -= (tf2*tf2*tf2)* 0.1472725f; // / 6.7901358;;

f[currentvoice][2] -= (tf3*tf3*tf3) * 0.1472725f;// / 6.7901358; 
#endif



low[currentvoice][0] = low[currentvoice][0] + f[currentvoice][0] * band[currentvoice][0];
high[currentvoice][0] = (q[currentvoice][0] * temp) - low[currentvoice][0] - (q[currentvoice][0]*band[currentvoice][0]);
band[currentvoice][0]= f[currentvoice][0] * high[currentvoice][0] + band[currentvoice][0];


low[currentvoice][1] = low[currentvoice][1] + f[currentvoice][1] * band[currentvoice][1];
high[currentvoice][1] = (q[currentvoice][1] * temp) - low[currentvoice][1] - (q[currentvoice][1]*band[currentvoice][1]);
band[currentvoice][1]= f[currentvoice][1] * high[currentvoice][1] + band[currentvoice][1];


low[currentvoice][2] = low[currentvoice][2] + f[currentvoice][2] * band[currentvoice][2];
high[currentvoice][2] = (q[currentvoice][2] * temp) - low[currentvoice][2] - (q[currentvoice][2]*band[currentvoice][2]);
band[currentvoice][2]= f[currentvoice][2] * high[currentvoice][2] + band[currentvoice][2];
modulator[currentvoice] [7] = (low[currentvoice][0]*v[currentvoice][0])+band[currentvoice][1]*v[currentvoice][1]+band[currentvoice][2]*v[currentvoice][2];

//---------------------------------- amplitude shaping

result = (1.f-modulator[currentvoice][ choice[currentvoice][13]]*param[100] );///_MULTITEMP;
result *= modulator[currentvoice][7];
result *= egCalc(currentvoice,0);// the final shaping envelope

// --------------------------------- delay unit
if( delayI[currentvoice] >= delayBufferSize )
{
    delayI[currentvoice] = 0;
    
//printf("clear %d : %d : %d\n",currentvoice,delayI[currentvoice],delayJ[currentvoice]);
}
delayMod = 1.f-(param[110]* modulator[currentvoice][choice[currentvoice][14]]);

delayJ[currentvoice] = delayI[currentvoice] - ((param[111]* maxDelayTime)*delayMod);

if( delayJ[currentvoice]  < 0 )
{
    delayJ[currentvoice]  += delayBufferSize;
}
else if (delayJ[currentvoice]>delayBufferSize)
{
	delayJ[currentvoice] = 0;
}

//if (delayI[currentvoice]>95000)
//printf("jab\n");

tdelay = result * param[114] + (delayBuffer[currentvoice] [ delayJ[currentvoice] ] * param[112] );
tdelay += anti_denormal;
//tdelay -= anti_denormal;
delayBuffer[currentvoice] [delayI[currentvoice] ] = tdelay;
/*
if (delayI[currentvoice]>95000)
{
printf("lll %d : %d : %d\n",currentvoice,delayI[currentvoice],delayJ[currentvoice]);
    fflush(stdout);
}
*/
modulator[currentvoice][18]= tdelay;
result += tdelay * param[113];
delayI[currentvoice]=delayI[currentvoice]+1;

// --------------------------------- output
float *buffer = (float*) jack_port_get_buffer(port[currentvoice], nframes);
buffer[index] = result * param[101];
bufferAux1[index] += result * param[108];
bufferAux2[index] += result * param[109];
result *= param[106]; // mix volume
bufferMixLeft[index] += result * (1.f-param[107]);
bufferMixRight[index] += result * param[107];

		//buffer[index] = Oscillator(50.2f,&phase1) * 0.5f;
//Initialization done here is the oscillator loop
                
       }
}
	return 1;
}

/* a flag which will be set by our signal handler when 
 * it's time to exit */
int quit = 0;

/* the signal handler */
void signalled(int signal) {
	quit = 1;
}
/** initialization, preparing for instance the waveforms
 */
void init ()
{
unsigned int i,k;
for (k=0;k<_MULTITEMP;k++)
{
	EG[k][0][1]=0.01f;
	EG[k][0][2]=0.01f;
	EG[k][0][3]=1.0f;
	EG[k][0][4]=0.0001f;
	EGtrigger[k][0]=0;
  
	parameter[k][30]=100.f;
	parameter[k][31]=0.5f;
	parameter[k][33]=100.f; 
	parameter[k][34]=0.5f;
	parameter[k][40]=100.f;
	parameter[k][41]=0.5f;
	parameter[k][43]=100.f; 
	parameter[k][44]=0.5f;
	parameter[k][50]=100.f;
	parameter[k][51]=0.5f;
	parameter[k][53]=100.f; 
	parameter[k][54]=0.5f;
	modulator[k][0] =0.f;// the none modulator, doing nothing
	for (i=0;i<3;++i) 
	{
		low[k][i]=0;
		high[k][i]=0;
	}
}
float PI=3.145;
float increment = (float)(PI*2) / (float)TableSize;
float x = 0.0f;
float tri = -0.9f;
// calculate wavetables
for (i=0; i<TableSize; i++)
{
			table[0][i] = (float)((float)sin(x+(
				(float)2.0f*(float)PI)));
			x += increment;
			table[1][i] = (float)i/tabF*2.f-1.f;// ramp up
			
			table[2][i] = 0.9f-(i/tabF*1.8f-0.5f);// tabF-((float)i/tabF*2.f-1.f);//ramp down
			
			if (i<TableSize/2) 
			{ 
				tri+=(float)1.f/TableSize*3.f; 
				table[3][i] = tri;
				table[4][i]=0.9f;
			}
			else
			{
				 tri-=(float)1.f/TableSize*3.f;
				 table[3][i] = tri;
				 table[4][i]=-0.9f;
			}
			table[5][i] = 0.f;
			table[6][i] = 0.f;
			if (i % 2 == 0)
				table[7][i] = 0.9f;
			else table [7][i] = -0.9f;
			table[8][i]=(float) (
			((float)sin(x+((float)2.0f*(float)PI))) +
			((float)sin(x*2.f+((float)2.0f*(float)PI)))+
			((float)sin(x*3.f+((float)2.0f*(float)PI)))+
			((float)sin(x*4.f+((float)2.0f*(float)PI)))*0.9f+
			((float)sin(x*5.f+((float)2.0f*(float)PI)))*0.8f+
			((float)sin(x*6.f+((float)2.0f*(float)PI)))*0.7f+
			((float)sin(x*7.f+((float)2.0f*(float)PI)))*0.6f+
			((float)sin(x*8.f+((float)2.0f*(float)PI)))*0.5f
			) / 8.0f;	
			table[9][i]=(float) (
			((float)sin(x+((float)2.0f*(float)PI))) +
			((float)sin(x*3.f+((float)2.0f*(float)PI)))+
			((float)sin(x*5.f+((float)2.0f*(float)PI)))+
			((float)sin(x*7.f+((float)2.0f*(float)PI)))*0.9f+
			((float)sin(x*9.f+((float)2.0f*(float)PI)))*0.8f+
			((float)sin(x*11.f+((float)2.0f*(float)PI)))*0.7f+
			((float)sin(x*13.f+((float)2.0f*(float)PI)))*0.6f+
			((float)sin(x*15.f+((float)2.0f*(float)PI)))*0.5f
			) / 8.0f;
			table[10][i]=(float)(sin((double)i/(double)TableSize+(sin((double)i*4))/2))*0.5;
			table[11][i]=(float)(sin((double)i/(double)TableSize*(sin((double)i*6)/4)))*2.;
			table[12][i]=(float)(sin((double)i*(sin((double)i*1.3)/50)));
			table[13][i]=(float)(sin((double)i*(sin((double)i*1.3)/5)));
			table[14][i]=(float)sin((double)i*0.5*(cos((double)i*4)/50));
			table[15][i]=(float)sin((double)i*0.5+(sin((double)i*14)/2));
			table[16][i]=(float)cos((double)i*2*(sin((double)i*34)/400));
			table[17][i]=(float)cos((double)i*4*((double)table[7][i]/150));
			
//printf("%f ",table[17][i]);

}
table[5][0] = -0.9f;
table[5][1] = 0.9f;

table[6][0] = -0.2f;
table[6][1] = -0.6f;
table[6][2] = -0.9f;
table[6][3] = -0.6f;
table[6][4] = -0.2f;
table[6][5] = 0.2f;
table[6][6] = 0.6f;
table[6][7] = 0.9f;
table[6][8] = 0.6f;
table[6][9] = 0.2f;
/*
float pi = 3.145f;
float oscfreq = 1000.0; //%Oscillator frequency in Hz
c1 = 2 * cos(2 * pi * oscfreq / Fs);
//Initialize the unit delays
d1 = sin(2 * pi * oscfreq / Fs);  
d2 = 0;*/
//Initialization done here is the oscillator loop
//% which generates a sinewave



// miditable for notes to frequency
for (i = 0;i<128;++i) midi2freq[i] = 8.1758f * pow(2,(i/12.f));

} // end of initialization
/** handling the midi messages in an extra thread
 */
static void *midiprocessor(void *handle) {
	struct sched_param param;
	int policy;
  snd_seq_t *seq_handle = (snd_seq_t *)handle;
	pthread_getschedparam(pthread_self(), &policy, &param);

	policy = SCHED_FIFO;
	param.sched_priority = 95;

	pthread_setschedparam(pthread_self(), policy, &param);

/*
if (poll(pfd, npfd, 100000) > 0) 
		{
    	  midi_action(seq_handle);
		} */
		
  snd_seq_event_t *ev;

  do {
   while (snd_seq_event_input(seq_handle, &ev))
   {
    switch (ev->type) {
      case SND_SEQ_EVENT_NOTEON:
      {   
      	unsigned int c = ev->data.note.channel;
#ifdef _DEBUG      
        fprintf(stderr, "Note On event on Channel %2d: %5d       \r",
                c, ev->data.note.note);
#endif		
      	if (c <_MULTITEMP)
	{
                if (ev->data.note.velocity>0)
                {
                lastnote[c]=ev->data.note.note;	
                midif[c]=midi2freq[ev->data.note.note];// lookup the frequency
                modulator[c][19]=ev->data.note.note*0.007874f;// fill the value in as normalized modulator
                modulator[c][1]=(float)1.f-(ev->data.note.velocity*0.007874f);// fill in the velocity as modulator
                egStart(c,0);// start the engines!
                if (EGrepeat[c][1] == 0)egStart(c,1);
                if (EGrepeat[c][2] == 0)egStart(c,2);
                if (EGrepeat[c][3] == 0)egStart(c,3);
               	if (EGrepeat[c][4] == 0) egStart(c,4);
               	if (EGrepeat[c][5] == 0) egStart(c,5);
               	if (EGrepeat[c][6] == 0) egStart(c,6);
               
		break;// not the best method but it breaks only when a note on is
                }// if velo == 0 it should be handled as noteoff...
	}
      }      
      // ...so its necessary that here follow the noteoff routine
      case SND_SEQ_EVENT_NOTEOFF: 
      {
      	unsigned int c = ev->data.note.channel;
#ifdef _DEBUG      
        fprintf(stderr, "Note Off event on Channel %2d: %5d      \r",         
                c, ev->data.note.note);
#endif		
               if  (c <_MULTITEMP)
               if (lastnote[c]==ev->data.note.note)
               {
                egStop(c,0);  
               	if (EGrepeat[c][1] == 0) egStop(c,1);  
                if (EGrepeat[c][2] == 0) egStop(c,2); 
               	if (EGrepeat[c][3] == 0) egStop(c,3); 
               	if (EGrepeat[c][4] == 0) egStop(c,4);  
                if (EGrepeat[c][5] == 0) egStop(c,5);  
               	if (EGrepeat[c][6] == 0) egStop(c,6);
               }    
        break;       
    } 
      case SND_SEQ_EVENT_CONTROLLER:
      {
#ifdef _DEBUG      
        fprintf(stderr, "Control event on Channel %2d: %2d %5d       \r",
                ev->data.control.channel,  ev->data.control.param,ev->data.control.value);
#endif		
        if  (ev->data.control.param==1)   
        	  modulator[ev->data.control.channel][ 16]=(float)ev->data.control.value*0.007874f; // /127.f;
        	  else 
        	  if  (ev->data.control.param==12)   
        	  modulator[ev->data.control.channel][ 17]=(float)ev->data.control.value*0.007874f;// /127.f;
        break;
      }
      case SND_SEQ_EVENT_PITCHBEND:
      {
#ifdef _DEBUG      
         fprintf(stderr,"Pitchbender event on Channel %2d: %5d   \r", 
                ev->data.control.channel, ev->data.control.value);
#endif		
                if (ev->data.control.channel<_MULTITEMP)
               	 modulator[ev->data.control.channel][2]=(float)ev->data.control.value*0.0001221f; // /8192.f;
        break;
      }   
      case SND_SEQ_EVENT_CHANPRESS:
      {
#ifdef _DEBUG      
         fprintf(stderr,"touch event on Channel %2d: %5d   \r", 
                ev->data.control.channel, ev->data.control.value);
#endif		
                if (ev->data.control.channel<_MULTITEMP)
               	 modulator[ev->data.control.channel][ 15]=(float)ev->data.control.value*0.007874f;
        break;
      }
#ifdef _DEBUG      
      default:
      {
      	
         fprintf(stderr,"unknown event %d on Channel %2d: %5d   \r",ev->type, 
                ev->data.control.channel, ev->data.control.value);
      }
#endif		
    }// end of switch
    snd_seq_free_event(ev);
   }// end of first while, emptying the seqdata queue
  } while (1==1);// doing forever, was  (snd_seq_event_input_pending(seq_handle, 0) > 0);
  return 0;
}
int main() {
// ------------------------ midi init ---------------------------------
  pthread_t midithread;
  seq_handle = open_seq();
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
    
    // create the thread and tell it to use Midi::work as thread function
	int err = pthread_create(&midithread, NULL, midiprocessor,seq_handle);
// ------------------------ OSC Init ------------------------------------   
  /* start a new server on port definied in _OSCPORT */
    lo_server_thread st = lo_server_thread_new(_OSCPORT, error);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/Minicomputer/choice", "iii", generic_handler, NULL);

    /* add method that will match the path /foo/bar, with two numbers, coerced
     * to float and int */
    lo_server_thread_add_method(st, "/Minicomputer", "iif", foo_handler, NULL);

    /* add method that will match the path /quit with no args */
  //  lo_server_thread_add_method(st, "/quit", "", quit_handler, NULL);
	
    lo_server_thread_start(st);
   





	/* setup our signal handler signalled() above, so 
	 * we can exit cleanly (see end of main()) */
	signal(SIGINT, signalled);

	init();
	/* naturally we need to become a jack client :) */
	client = jack_client_new("Minicomputer");
	if (!client) {
		printf("couldn't connect to jack server. Either it's not running or the client name is already taken\n");
		exit(1);
	}

	/* we register the output ports and tell jack these are 
	 * terminal ports which means we don't 
	 * have any input ports from which we could somhow 
	 * feed our output */
	port[0] = jack_port_register(client, "output1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[1] = jack_port_register(client, "output2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[2] = jack_port_register(client, "output3", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[3] = jack_port_register(client, "output4", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[4] = jack_port_register(client, "output5", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[5] = jack_port_register(client, "output6", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[6] = jack_port_register(client, "output7", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[7] = jack_port_register(client, "output8", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	
	port[8] = jack_port_register(client, "mix out left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[9] = jack_port_register(client, "mix out right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[10] = jack_port_register(client, "aux out 1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[11] = jack_port_register(client, "aux out 2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	//inbuf = jack_port_register(client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	/* jack is callback based. That means we register 
	 * a callback function (see process() above)
	 * which will then get called by jack once per process cycle */
	jack_set_process_callback(client, process, 0);
	bufsize = jack_get_buffer_size(client);

	// handling the sampling frequency
	sampleRate = (float) jack_get_sample_rate (client); 
	tabX = 4096.f / sampleRate;
	srate = 3.145f/ sampleRate;
	
	// depending on it the delaybuffer
	maxDelayTime = (int)sampleRate;
	delayBufferSize = maxDelayTime*2;
	// generate the delaybuffers for each voice
	int k;
	for (k=0; k<_MULTITEMP;++k)
	{
		//float dbuffer[delayBufferSize];
		//delayBuffer[k]=dbuffer;
		delayI[k]=0;
		delayJ[k]=0;
	}
	#ifdef _DEBUG
	printf("bsize:%d %d\n",delayBufferSize,maxDelayTime);
	#endif
	/* tell jack that we are ready to do our thing */
	jack_activate(client);
	
	/* wait until this app receives a SIGINT (i.e. press 
	 * ctrl-c in the terminal) see signalled() above */
	while (!quit) 
	{
	// operate midi
		 /* let's not waste cycles by busy waiting */
		sleep(1);
	
	}
	//printf("bla\n"); fflush(stdout);
	
	/* so we shall quit, eh? ok, cleanup time. otherwise 
	 * jack would probably produce an xrun
	 * on shutdown */
	jack_deactivate(client);

	/* shutdown cont. */
	jack_client_close(client);

	/* done !! */
	return 0;
}
// ******************************************** OSC handling for editors ***********************
static inline void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
static inline int generic_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data)
{
  if ( (argv[0]->i < _MULTITEMP) && (argv[1]->i < _CHOICEMAX) )
  		{
  			
    	choice[argv[0]->i][argv[1]->i]=argv[2]->i;
    	return 0;
    	}
    	else return 1;
    
}


static inline int foo_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    /* example showing pulling the argument values out of the argv array */
   int voice =  argv[0]->i;
   int i =  argv[1]->i;
   if ((voice<_MULTITEMP)&&(i>0) && (i<_PARACOUNT)) 
   {
   	parameter[voice][i]=argv[2]->f;
   	//if ((i==2)||(i==17))
   	//	parameter[voice][i]=1.f-argv[2]->f;
   }

   //if ((i==10) && (parameter[10]!=0)) parameter[10]=1000.f;
   // printf("%s <- f:%f, i:%d\n\n", path, argv[0]->f, argv[1]->i);
   // fflush(stdout);
   switch (i)
   {
   	 // reset the filters 
   	 case 0:{
   	 	low[voice][0]	= 0.f;
   	 	high[voice][0]	= 0.f;
   	 	band[voice][0] = 0.f;
   	 	low[voice][1]	= 0.f;
   	 	high[voice][1]	= 0.f;
   	 	band[voice][1] = 0.f;
   	 	low[voice][2]	= 0.f;
   	 	high[voice][2]	= 0.f;
   	 	band[voice][2] = 0.f;
   	 	phase[voice][1] = 0.f;
   	 	phase[voice][2] = 0.f;
   	 	phase[voice][3] = 0.f;
		memset(delayBuffer[voice],0,sizeof(delayBuffer[voice]));
   	 break;}
   	 
   	 case 60:EG[voice][1][1]=argv[2]->f;break;
   	 case 61:EG[voice][1][2]=argv[2]->f;break;
   	 case 62:EG[voice][1][3]=argv[2]->f;break;
   	 case 63:EG[voice][1][4]=argv[2]->f;break;
   	 case 64:
   	 {
   	 	EGrepeat[voice][1] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[voice][1] > 0 ) egStart(voice,1);
   	 	break;
   	 }
   	 case 65:EG[voice][2][1]=argv[2]->f;break;
   	 case 66:EG[voice][2][2]=argv[2]->f;break;
   	 case 67:EG[voice][2][3]=argv[2]->f;break;
   	 case 68:EG[voice][2][4]=argv[2]->f;break;
   	  case 69:
   	 {
   	 	EGrepeat[voice][2] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[voice][2] > 0 ) egStart(voice,2);
   	 	break;
   	 }
   	 case 70:EG[voice][3][1]=argv[2]->f;break;
   	 case 71:EG[voice][3][2]=argv[2]->f;break;
   	 case 72:EG[voice][3][3]=argv[2]->f;break;
   	 case 73:EG[voice][3][4]=argv[2]->f;break;
   	  case 74:
   	 {
   	 	EGrepeat[voice][3] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[voice][3] > 0 ) egStart(voice,3);
   	 	break;
   	 }
   	 case 75:EG[voice][4][1]=argv[2]->f;break;
   	 case 76:EG[voice][4][2]=argv[2]->f;break;
   	 case 77:EG[voice][4][3]=argv[2]->f;break;
   	 case 78:EG[voice][4][4]=argv[2]->f;break; 
   	  case 79:
   	 {
   	 	EGrepeat[voice][4] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[voice][4] > 0 ) egStart(voice,4);
   	 	break;
   	 }
   	 case 80:EG[voice][5][1]=argv[2]->f;break;
   	 case 81:EG[voice][5][2]=argv[2]->f;break;
   	 case 82:EG[voice][5][3]=argv[2]->f;break;
   	 case 83:EG[voice][5][4]=argv[2]->f;break;
   	  case 84:
   	 {
   	 	EGrepeat[voice][5] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[voice][5] > 0 ) egStart(voice,5);
   	 	break;
   	 }
   	 case 85:EG[voice][6][1]=argv[2]->f;break;
   	 case 86:EG[voice][6][2]=argv[2]->f;break;
   	 case 87:EG[voice][6][3]=argv[2]->f;break;
   	 case 88:EG[voice][6][4]=argv[2]->f;break;
   	  case 89:
   	 {
   	 	EGrepeat[voice][6] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[voice][6] > 0 ) egStart(voice,6);
   	 	break;
   	 }
   	 case 102:EG[voice][0][1]=argv[2]->f;break;
   	 case 103:EG[voice][0][2]=argv[2]->f;break;
   	 case 104:EG[voice][0][3]=argv[2]->f;break;
   	 case 105:EG[voice][0][4]=argv[2]->f;break;
   	 
   }
   //float g=parameter[30]*parameter[56]+parameter[33]*(1.0f-parameter[56]);
#ifdef _DEBUG
   printf("%i %i %f \n",voice,i,argv[2]->f);
#endif   
    return 0;
}

static inline int quit_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    done = 1;
    printf("quitting\n\n");
    fflush(stdout);

    return 0;
}

