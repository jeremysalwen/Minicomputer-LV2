/** Minicomputer
 * industrial grade digital synthesizer
 *
 * Copyright 2007,2008,2009,2010 Malte Steiner
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
// a way to compile it was:
//  gcc -o synthesizer synth2.c -ljack -ffast-math -O3 -march=k8 -mtune=k8 -funit-at-a-time -fpeel-loops -ftracer -funswitch-loops -llo -lasound


/**  FIXED
 * start the envelope generator
 * called by a note on event for that voice
 * @param the voice number
 * @param the number of envelope generator
 */

static inline void egStart (engine* voice,const unsigned int number)
{
	voice->EGtrigger[number]=1;
	voice->EG[number][0] = 1.f; // triggerd
	voice->EG[number][5] = 1.f; // target
    voice->EG[number][7] = 0.0f;// state
    voice->EGstate[number] = 0;// state  
	voice->EGFaktor[number] = 0.f;
		 //printf("start %i", voice);
}
/** FIXED
 * set the envelope to release mode
 * should be called by a related noteoff event
 * @param the voice number
 * @param the number of envelope generator
 */
static inline void egStop (engine* voice,const unsigned int number)
{
	// if (EGrepeat[number] == 0) 
	voice->EGtrigger[number] = 0; // triggerd
	voice->EGstate[number] = 0; // target
	// printf("stop %i", voice);
}
/** FIXED
 * calculate the envelope, done in audiorate to avoide zippernoise
 * @param the voice number
 * @param the number of envelope generator
*/
static inline float egCalc (engine* voice, const unsigned int number)
{
	float** EG=voice->EG;
	float * EGFaktor=voice->EGFaktor;
	float* EGstate=voice->EGState;
	float* EGrepeat=voice->EGRepeat;
	float* EGtrigger=voice->EGTrigger;
	/* EG[x] x:
	 * 0 = trigger
	 * 1 = attack
	 * 2 = decay
	 * 3 = sustain
	 * 4 = release
	 * 5 = target
	 * 6 = state
	 */
	if (EGtrigger[number] != 1)
	{
	int i = EGstate[number]; 
		if (i == 1){ // attack
		         if (EGFaktor[number]<1.00f) EGFaktor[number] += 0.002f;
			
			 EG[number][6] += EG[number][1]*srDivisor*EGFaktor[number];

			 if (EG[number][6]>=1.0f)// Attackphase is finished
			 {
			 	EG[number][6]=1.0f;
			 	EGstate[number]=2;
					EGFaktor[number] = 1.f; // triggerd

			 }
		}
		else if (i == 2)
		{ // decay
			if (EG[number][6]>EG[number][3])
			{
				 EG[number][6] -= EG[number][2]*srDivisor*EGFaktor[number];
			}
			else 
			{
				if (EGrepeat[number]==0)
				{
					EGstate[number]=3; // stay on sustain
				}
				else
				{
					EGFaktor[number] = 1.f; // triggerd
					egStop(voice,number);// continue to release
				}
			}
			// what happens if sustain = 0? envelope should go in stop mode when decay reached ground
			if (EG[number][6]<0.0f) 
		    	{	
		    		EG[number][6]=0.0f;
		    		if (EGrepeat[number]==0)
				{
					EGstate[number]=4; // released
				}
				else
				{
					egStart(voice,number);// repeat
				}
		    	}

		} // end of decay
		else if ((i == 0) && (EG[number][6]>0.0f))
		{
		    /* release */
		    
		    if (EGFaktor[number]>0.025f) EGFaktor[number] -= 0.002f;
		    EG[number][6] -= EG[number][4]*srDivisor*EGFaktor[number];//*EG[number][6];

		    if (EG[number][6]<0.0f) 
		    {	
		    	EG[number][6]=0.0f;
		    	if (EGrepeat[number]==0)
				{
					EGstate[number]=4; // released
				}
				else
				{
					egStart(voice,number);// repeat
				}
		    }
		}
	}
	else
	{
		EGtrigger[number] = 0;
		EG[number][0] = 1.f; // triggerd
		EGstate[number] = 1; // target
	}
	return EG[number][6];
}
//float d0,d1,d2,c1;


static void run_minicomputer(LV2_Handle instance, uint32_t nframes) {
	minicomputer* mini= (minicomputer*)instance;
	float tf,tf1,tf2,tf3,ta1,ta2,ta3,morph,mo,mf,result,tdelay,clib1,clib2;
	float osc1,osc2,delayMod;

	// an union for a nice float to int casting trick which should be fast
	typedef union
	{
		int i;
		float f;
	} INTORFLOAT;
	INTORFLOAT P1 __attribute__((aligned (16)));
	INTORFLOAT P2 __attribute__((aligned (16)));
	INTORFLOAT P3 __attribute__((aligned (16)));
	INTORFLOAT bias; // the magic number
	bias.i = (23 +127) << 23;// generating the magic number

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
	float *bufferMixLeft = mini->MixLeft_p;
	float *bufferMixRight = mini->MixRight_p;
	float *bufferAux1 =mini->Aux1_p;
	float *bufferAux2 =mini->Aux2_p;

	/* main loop */
	register unsigned int index;
	for (index = 0; index < nframes; ++index) 
	{
		handlemidi(mini,index);
		bufferMixLeft[index]=0.f;
		bufferMixRight[index]=0.f;
		bufferAux1[index]=0.f;
		bufferAux2[index]=0.f;
		/*
		 * I dont know if its better to separate the calculation blocks, so I try it
		 * first calculating the envelopes
		 */
		register unsigned int currentvoice;
		for (currentvoice=0;currentvoice<_multitemp;++currentvoice) // for each voice
		{		
			engine* voice=engines+currentvoice;

			// calc the modulators
			float * mod = voice->modulator;
			mod[8] =1.f-egcalc(voice,1);
			mod[9] =1.f-egcalc(voice,2);
			mod[10]=1.f-egcalc(voice,3);
			mod[11]=1.f-egcalc(voice,4);
			mod[12]=1.f-egcalc(voice,5);
			mod[13]=1.f-egcalc(voice,6);
			/**
			 * calc the main audio signal
			 */

			// get the parameter settings
			float * param = voice->parameter;
			// casting floats to int for indexing the 3 oscillator wavetables with custom typecaster
			p1.f =  voice->phase[1];
			p2.f =  voice->phase[2];
			p3.f =  voice->phase[3];
			p1.f += bias.f;
			p2.f += bias.f;
			p3.f += bias.f;
			p1.i -= bias.i;
			p2.i -= bias.i;
			p3.i -= bias.i;
			ip1=p1.i&tabm;//i%=tablesize;
			ip2=p2.i&tabm;//i%=tablesize;
			ip3=p3.i&tabm;//i%=tablesize;

			if (ip1<0) ip1=tabm;
			if (ip2<0) ip2=tabm;
			if (ip3<0) ip3=tabm;

			// create the next oscillator phase step for osc 3
			voice->phase[3]+= tabx * param[90];
#ifdef _prefetch
			__builtin_prefetch(&param[1],0,0);
			__builtin_prefetch(&param[2],0,1);
			__builtin_prefetch(&param[3],0,0);
			__builtin_prefetch(&param[4],0,0);
			__builtin_prefetch(&param[5],0,0);
			__builtin_prefetch(&param[7],0,0);
			__builtin_prefetch(&param[11],0,0);
#endif

			if(voice->phase[3]  >= tabf)
			{
				voice->phase[3]-= tabf;
				// if (*phase>=tabf) *phase = 0; //just in case of extreme fm
			}
			if(voice->phase[3]< 0.f)
			{
				voice->phase[3]+= tabf;
				//	if(*phase < 0.f) *phase = tabf-1;
			}

			unsigned int * choi = voice->choice;
			//modulator [currentvoice][14]=oscillator(parameter[currentvoice][90],choice[currentvoice][12],&phase[currentvoice][3]);
			// write the oscillator 3 output to modulators
			mod[14] = table[choi[12]][ip3] ;

			// --------------- calculate the parameters and modulations of main oscillators 1 and 2
			tf = param[1];
			tf *=param[2];
			ta1 = param[9];
			ta1 *= mod[choi[2]]; // osc1 first ampmod

#ifdef _prefetch
			__builtin_prefetch(&voice->phase[1],0,2);
			__builtin_prefetch(&voice->phase[2],0,2);
#endif

			tf+=(voice->midif*(1.0f-param[2])*param[3]);
			ta1+= param[11]*mod[choi[3]];// osc1 second ampmod
			tf+=(param[4]*param[5])*mod[choi[0]];
			tf+=param[7]*mod[choi[1]];

			// generate phase of oscillator 1
			voice->phase[1]+= tabx * tf;

			if(voice->phase[1]  >= tabf)
			{
				voice->phase[1]-= tabf;
				//if (param[115]>0.f) phase[currentvoice][2]= 0; // sync osc2 to 1
				// branchless sync:
				voice->phase[1]-= voice->phase[2]*param[115];

				// if (*phase>=tabf) *phase = 0; //just in case of extreme fm
			}

#ifdef _prefetch
			__builtin_prefetch(&param[15],0,0);
			__builtin_prefetch(&param[16],0,0);
			__builtin_prefetch(&param[17],0,0);
			__builtin_prefetch(&param[18],0,0);
			__builtin_prefetch(&param[19],0,0);
			__builtin_prefetch(&param[23],0,0);
			__builtin_prefetch(&param[25],0,0);
			__builtin_prefetch(&voice->choice[6],0,0);
			__builtin_prefetch(&voice->choice[7],0,0);
			__builtin_prefetch(&voice->choice[8],0,0);
			__builtin_prefetch(&voice->choice[9],0,0);
#endif

			if(voice->phase[1]< 0.f)
			{
				voice->phase[1]+= tabf;
				//	if(*phase < 0.f) *phase = tabf-1;
			}
			osc1 = table[choi[4]][ip1] ;
			//}
			//osc1 = oscillator(tf,choice[currentvoice][4],&phase[currentvoice][1]);
			mod[3]=osc1*(param[13]*(1.f+ta1));//+parameter[currentvoice][13]*ta1);

			// ------------------------ calculate oscillator 2 ---------------------
			// first the modulations and frequencys
			tf2 = param[16];
			tf2 *=param[17];
			ta2 = param[23];
			ta2 *=mod[choi[8]]; // osc2 first amp mod
			//tf2+=(midif[currentvoice]*parameter[currentvoice][17]*parameter[currentvoice][18]);
			tf2+=(voice->midif*(1.0f-param[17])*param[18]);
			ta3 = param[25];
			ta3 *=mod[choi[9]];// osc2 second amp mod
			tf2+=param[15]*param[19]*mod[choi[6]];
			tf2+=param[21]*mod[choi[7]];
			//tf/=3.f;		
			//ta/=2.f;
			mod[4] = (param[28]+param[28]*(1.f-ta3));// osc2 fm out

			// then generate the actual phase:
			voice->phase[2]+= tabx * tf2;
			if(voice->phase[2]  >= tabf)
			{
				voice->phase[2]-= tabf;
				// if (*phase>=tabf) *phase = 0; //just in case of extreme fm
			}

#ifdef _prefetch
			__builtin_prefetch(&param[14],0,0);
			__builtin_prefetch(&param[29],0,0);
			__builtin_prefetch(&param[38],0,0);
			__builtin_prefetch(&param[48],0,0);
			__builtin_prefetch(&param[56],0,0);
#endif

			if(voice->phase[2]< 0.f)
			{
				voice->phase[2]+= tabf;
			}
			osc2 = table[choi[5]][ip2] ;
			mod[4] *= osc2;// osc2 fm out

			// ------------------------------------- mix the 2 oscillators pre filter
			temp=(param[14]*(1.f-ta1));
			temp*=osc1;
			temp+=osc2*(param[29]*(1.f-ta2));
			temp*=0.5f;// get the volume of the sum into a normal range	
			temp+=anti_denormal;

			// ------------- calculate the filter settings ------------------------------
			mf =  (1.f-(param[38]*mod[ choi[10]]));
			mf+= (1.f-(param[48]*mod[ choi[11]]));
			mo = param[56]*mf;


#ifdef _prefetch
			__builtin_prefetch(&param[30],0,0);
			__builtin_prefetch(&param[31],0,0);
			__builtin_prefetch(&param[32],0,0);
			__builtin_prefetch(&param[40],0,0);
			__builtin_prefetch(&param[41],0,0);
			__builtin_prefetch(&param[42],0,0);
			__builtin_prefetch(&param[50],0,0);
			__builtin_prefetch(&param[51],0,0);
			__builtin_prefetch(&param[52],0,0);
#endif

			clib1 = fabs (mo);
			clib2 = fabs (mo-1.0f);
			mo = clib1 + 1.0f;
			mo -= clib2;
			mo *= 0.5f;
			
			morph=(1.0f-mo);

			// parallel calculation:
#ifdef _vector
			union f4vector a __attribute__((aligned (16))), b __attribute__((aligned (16))),  c __attribute__((aligned (16))), d __attribute__((aligned (16))),e __attribute__((aligned (16)));

			b.f[0] = morph; b.f[1] = morph; b.f[2] = morph; b.f[3] = morph;
			a.f[0] = param[30]; a.f[1] =param[31]; a.f[2] = param[32]; a.f[3] = param[40];
			d.f[0] = param[41]; d.f[1] =param[42]; d.f[2] = param[50]; d.f[3] = param[51];
			c.v = a.v * b.v;
			//c.v = __builtin_ia32_mulps (a.v, b.v);
			e.v = d.v * b.v;
			//e.v = __builtin_ia32_mulps (d.v, b.v);

			tf1 = c.f[0];
			voice->q[0]=c.f[1];
			voice->v[0]=c.f[2];
			tf2 = c.f[3];
			voice->q[1] = e.f[0];
			voice->v[1] = e.f[1];
			tf3 =  e.f[2];
			voice->q[2] = e.f[3];

#else
			tf1= param[30];
			voice->q[0] = param[31];
			voice->v[0] = param[32];
			tf2= param[40];

			voice->q[1] = param[41];
			voice->v[1] = param[42];
			tf3 =  param[50];
			voice->q[2] = param[51];
#endif

			voice->v[2] = param[52];

#ifdef _prefetch
			__builtin_prefetch(&param[33],0,0);
			__builtin_prefetch(&param[34],0,0);
			__builtin_prefetch(&param[35],0,0);
			__builtin_prefetch(&param[43],0,0);
			__builtin_prefetch(&param[44],0,0);
			__builtin_prefetch(&param[45],0,0);
			__builtin_prefetch(&param[53],0,0);
			__builtin_prefetch(&param[54],0,0);
			__builtin_prefetch(&param[55],0,0);
#endif

#ifndef _vector
			tf1*= morph;
			tf2*= morph;
			voice->q[0] *= morph;
			voice->v[0] *= morph;

			tf3 *=  morph;
			voice->v[1] *= morph;
			voice->q[1] *= morph;
			voice->q[2] *= morph;
#endif

			voice->v[2] *= morph;

#ifdef _vector
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
			voice->q[0] += c.f[1];//parameter[currentvoice][34]*mo;
			voice->q[1] += e.f[0];//parameter[currentvoice][44]*mo;
			voice->q[2] += e.f[3];//parameter[currentvoice][54]*mo;
			voice->v[0] += c.f[2];//parameter[currentvoice][35]*mo;
			voice->v[1] += e.f[1];//parameter[currentvoice][45]*mo;

#else
			tf1+= param[33]*mo;
			tf2+=param[43]*mo;
			tf3 += param[53]*mo;
#endif


#ifndef _vector
			voice->q[0] += param[34]*mo;
			voice->q[1] += param[44]*mo;
			voice->q[2] += param[54]*mo;

			voice->v[0] += param[35]*mo;
			voice->v[1] += param[45]*mo;

			tf1*=srate;
			tf2*=srate;
			tf3 *= srate;
#endif

#ifdef _vector
			// prepare next calculations

			a.f[0] = param[55]; a.f[1] =tf1; a.f[2] = tf2; a.f[3] = tf3;
			g.f[0] = mo;// b.f[1] = 2.f; b.f[2] = 2.f; b.f[3] = 2.f;
			j.v = a.v * i.v; // tf * srate
			c.v = j.v * g.v; // tf * 2

			voice->v[2] += c.f[0];//parameter[currentvoice][55]*mo;

			d.v = c.v - ((j.v * j.v * j.v) * h.v);

			voice->f0] = d.f[1];//(tf1*tf1*tf1) * 0.1472725f;// / 6.7901358;

			voice->f[1] = d.f[2];//(tf2*tf2*tf2)* 0.1472725f; // / 6.7901358;;

			voice->f[2] = d.f[3];//(tf3*tf3*tf3) * 0.1472725f;// / 6.7901358; 
#else
			voice->v[2] += param[55]*mo;

			voice->f[0] = 2.f * tf1;
			voice->f[1] = 2.f * tf2;
			voice->f[2] = 2.f * tf3; 

			voice->f[0] -= (tf1*tf1*tf1) * 0.1472725f;// / 6.7901358;

			voice->f[1] -= (tf2*tf2*tf2)* 0.1472725f; // / 6.7901358;;

			voice->f[2] -= (tf3*tf3*tf3) * 0.1472725f;// / 6.7901358; 
#endif
			//----------------------- actual filter calculation -------------------------
			// first filter
			float reso = voice->q[0]; // for better scaling the volume with rising q
			voice->low[0] = voice->low[0] + voice->f[0] * voice->band[0];
			voice->high[0] = ((reso + ((1.f-reso)*0.1f))*temp) - voice->low[0] - (reso*voice->band[0]);
			voice->band[0]= voice->f[0] * voice->high[0] + voice->band[0];

			reso = voice->q[1];
			// second filter
			voice->low[1] = voice->low[1] + voice->f[1] * voice->band[1];
			voice->high[1] = ((reso + ((1.f-reso)*0.1f))*temp) - voice->low[1] - (reso*voice->band[1]);
			voice->band[1]= f[currentvoice][1] * high[currentvoice][1] + band[currentvoice][1];

			// third filter
			reso = voice->q[2];
			voice->low[2] = voice->low[2] + voice->f[2] * band[currentvoice][2];
			voice->high[2] = ((reso + ((1.f-reso)*0.1f))*temp) - voice->low[2] - (reso*voice->band[2]);
			voice->band[2]= voice->f[2] * voice->high[currentvoice][2] + voice->ban[2];

			mod [7] = (voice->low[0]*voice->v[0])+voice->band[1]*voice->v[1]+voice->band[2]*voice->v[2];

			//---------------------------------- amplitude shaping

			result = (1.f-mod[ choi[13]]*param[100] );///_multitemp;
				result *= mod[7];
			result *= egcalc(voice,0);// the final shaping envelope

			// --------------------------------- delay unit
			if( voice->delayi>= delaybuffersize ) {
				voice->delayi = 0;
			}
			delaymod = 1.f-(param[110]* mod[choi[14]]);

			voice->delayj = voice->delayi- ((param[111]* maxdelaytime)*delaymod);

			if( voice->delayj  < 0 ) {
				voice->delayj += delaybuffersize;
			}\else if (voice->delayj>delaybuffersize) {
				voice->delayj= 0;
			}
			tdelay = result * param[114] + (voice->delaybuffer [ voice->delayj ] * param[112] );
			tdelay += anti_denormal;
			voice->delaybuffer[voice->delayi ] = tdelay;

			mod[18]= tdelay;
			result += tdelay * param[113];
			voice->delayi=voice->delayi+1;

			// --------------------------------- output
			float *buffer = (float*) jack_port_get_buffer(voice->port, nframes);
			buffer[index] = result * param[101];
			bufferaux1[index] += result * param[108];
			bufferaux2[index] += result * param[109];
			result *= param[106]; // mix volume
			buffermixleft[index] += result * (1.f-param[107]);
			buffermixright[index] += result * param[107];
			}
			}
			}
			}
void initEngine(engine* voice) {
		float** EG=voice->EG;
		float* EGtrigger=voice->EGtrigger;
		float* parameter=voice->parameter;
		float* modulator=voice->modulator;
		float* low=voice->low;
		float* high=voice->high;
		for (i=0;i<8;i++) // i is the number of envelope
		{
		EG[i][1]=0.01f;
		EG[i][2]=0.01f;
		EG[i][3]=1.0f;
		EG[i][4]=0.0001f;
		EGtrigger[i]=0;
  
		EGrepeat[i]=0;
		EGstate[i]=4; // released
		}
		parameter[30]=100.f;
		parameter[31]=0.5f;
		parameter[33]=100.f; 
		parameter[34]=0.5f;
		parameter[40]=100.f;
		parameter[41]=0.5f;
		parameter[43]=100.f; 
		parameter[44]=0.5f;
		parameter[50]=100.f;
		parameter[51]=0.5f;
		parameter[53]=100.f; 
		parameter[54]=0.5f;
		modulator[0] =0.f;// the none modulator, doing nothing
		for (unsigned int i=0;i<3;++i) 
		{
			low[i]=0;
			high[i]=0;
		}
}
void initEngines(minicomputer* mini) {
	minicomputer* mini= malloc(sizeof(minicomputer));
	memset(mini->noteson,0,sizeof(mini->noteson));
	mini->inuse=NULL;
	mini->freeblocks.previous=(minicomputer*)&mini->freeblocks;
	mini->freeblocks.next=(minicomputer*)&mini->freeblocks;
	unsigned int numvoices=8;
	for(unsigned int i=0; i<numvoices; i++) {
		engineblock* e=malloc(sizeof(engineblock));
		initEngine(&e->e);
		e->next=mini->freeblocks.next;
		e->next.previous=e;
		mini->freeblocks.next=e;
		e->previous=(minicomputer*)mini->freeblocks;
	}
}

void waveTableInit() {
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

	// miditable for notes to frequency
	for (i = 0;i<128;++i) midi2freq[i] = 8.1758f * pow(2,(i/12.f));
}

void initOSC(minicomputer* mini) {
	// ------------------------ OSC Init ------------------------------------   
	/* start a new server on port definied where oport points to */
	mini->st = lo_server_thread_new(oport, error);

	/* add method that will match /Minicomputer/choice with three integers */
	lo_server_thread_add_method(mini->st, "/Minicomputer/choice", "iii", generic_handler, mini);

	/* add method that will match the path /Minicomputer, with three numbers, int (voicenumber), int (parameter) and float (value) 
	 */
    lo_server_thread_add_method(mini->st, "/Minicomputer", "iif", foo_handler, mini);

    /* add method that will match the path Minicomputer/quit with one integer */
  	lo_server_thread_add_method(mini->st, "/Minicomputer/quit", "i", quit_handler, mini);
	
	lo_server_thread_start(mini->st);
}
/** @brief initialization
 *
 * preparing for instance the waveforms
 */
LV2_Handle instantiateMinicomputer(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features) 
{
	minicomputer* mini= malloc(sizeof(minicomputer));
	initEngines(mini);
	initOSC(mini);
	static pthread_once_t initialized = PTHREAD_ONCE_INIT;
    pthread_once(&initialized, waveTableInit);

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
	
	port[10] = jack_port_register(client, "aux out 1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[11] = jack_port_register(client, "aux out 2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	
	// would like to create mix ports last because qjackctrl tend to connect automatic the last ports
	port[8] = jack_port_register(client, "mix out left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[9] = jack_port_register(client, "mix out right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);


	bufsize = jack_get_buffer_size(client);

	// handling the sampling frequency
	sampleRate = (float) jack_get_sample_rate (client); 
	tabX = 4096.f / sampleRate;
	srate = 3.145f/ sampleRate;
	srDivisor = 1.f / sampleRate * 100000.f;
	// depending on it the delaybuffer
	maxDelayTime = (int)sampleRate;
	delayBufferSize = maxDelayTime*2;
	// generate the delaybuffers for each voice
	int k;
	for (k=0; k<_MULTITEMP;++k)
	{
		delayI[k]=0;
		delayJ[k]=0;
	}
	#ifdef _DEBUG
	printf("bsize:%d %d\n",delayBufferSize,maxDelayTime);
	#endif
	
} // end of initialization


engine* use_note_minicomputer(minicomputer* mini, unsigned char index) {
	engineblock* result=mini->freeblocks.next;
	if(result==&mini->freeblocks) {
		return NULL;
	}
	mini->freeblocks.next=result->next;
	result->next->previous=(engineblock*)&mini->freeblocks; //using the fact that the next index is stored first;
	result->previous=(engineblock*)&mini->inuse; //using the fact that the next index is stored first;
	result->next=mini->inuse;
	if(mini->inuse) {
		mini->inuse->previous=result;
	}
	mini->inuse=result;
	mini->noteson[index]=result;
	return &result->e;
}

void free_note_minicomputer(minicomputer* mini, unsigned char index) {
	engineblock* result=mini->noteson[index];
	if(result) {
		result->h.previous->h.next=result->h.next;
		if(result->h.next) {
			result->h.next->previous=result->h.previous;
		}
		mini->noteson[index]=NULL;
		mini->freeblocks.previous->h.next=result;
		result->next=(engineblock*)&mini->freeblocks; //using the fact that the next index is stored first;
		result->previous=mini->freeblocks.previous;
		mini->freeblocks.previous=result;
	}	
}

inline void handlemidi(minicomputer* mini, unsigned int maxindex) {
	while(lv2_event_is_valid(&mini->in_iterator)) {
			uint8_t* data;
			LV2_Event* event= lv2_event_get(&mini->in_iterator,&data);
			if (event->type == 0) {
				mini->event_ref->lv2_event_unref(event_ref->callback_data, event);
			} else if(event->type==mini->midi_event_id) {
				if(event->frames > maxindex) {
					break;
				} else {
					const uint8_t* evt=(uint8_t*)data;
					unit8_t command=MIDI_COMMANDMASK & evt[0];
					unsigned int c = evt[0]&MIDI_CHANNELMASK;
					switch (command) 
					{	// first check the controllers
						// they usually come in hordes
						case MIDI_CONTROL:
#ifdef _DEBUG 
							fprintf(stderr, "Control event on Channel %2d: %2d %5d       \r",
							        c,  evt[0,evt[1]);
#endif		
								switch(evt[1]) {
									case 1:
										mini->modulator[ 16]=evt[2]*0.007874f; // /127.f;
										break;
									case 12:
										mini->modulator[ 17]=evt[2]*0.007874f;// /127.f;
										break;
									case 2:
										mini->modulator[ 20]=evt[2]*0.007874f;// /127.f;
										break;
									case 3:
										mini->modulator[ 21]=evt[2]*0.007874f;// /127.f;
										break;
									case 4:  
										mini->modulator[ 22]=evt[2]*0.007874f;// /127.f;
										break;
									case 5: 
										mini->modulator[ 23]=evt[2]*0.007874f;// /127.f;
										break;
									case 14:  
										mini->modulator[ 24]=evt[2]*0.007874f;// /127.f;
										break;
									case 15:
										mini->modulator[ 25]=evt[2]*0.007874f;// /127.f;
										break;
									case 16: 
										mini->modulator[ 26]=evt[2]*0.007874f;// /127.f;
										break;
									case 17:  
										mini->modulator[ 27]=evt[2]*0.007874f;// /127.f;
										break;
								}
						case MIDI_PITCHBEND:
							unsigned int value=evt[1]| (evt[2]<<8);
#ifdef _DEBUG      
							fprintf(stderr,"Pitchbender event on Channel %2d: %5d   \r", 
							        c,value);
#endif		
								mini->modulator[2]=value*0.0001221f; // /8192.f;
							break;
						case MIDI_CHANPRESS:
#ifdef _DEBUG      
							fprintf(stderr,"touch event on Channel %2d: %5d   \r", 
							        c,evt[1]);
#endif	
							mini->modulator[ 15]=(float)evt[1]*0.007874f;
							break;
						case MIDI_NOTEON:

#ifdef _DEBUG      
							fprintf(stderr, "Note On event on Channel %2d: %5d       \r",
							        c, ev->data.note.note);
#endif		
							if (ev->data.note.velocity>0)
						{
							engine* use=use_note_minicomputer(mini,evt[1]);
							if(use) {
								use->lastnote=evt[1];
								use->midif=midi2freq[evt[1]];// lookup the frequency
								mini->modulator[19]=evt[1]*0.007874f;// fill the value in as normalized modulator
								//TODO:  Should a global parameter be set by a single note's velocity?
								mini->modulator[1]=(float)1.f-(evt[2]*0.007874f);// fill in the velocity as modulator
								egStart(use,0);// start the engines!
								float* EGrepeat=use->EGrepeat;
								if (EGrepeat[1] == 0) egStart(use,1);
								if (EGrepeat[2] == 0) egStart(use,2);
								if (EGrepeat[3] == 0) egStart(use,3);
								if (EGrepeat[4] == 0) egStart(use,4);
								if (EGrepeat[5] == 0) egStart(use,5);
								if (EGrepeat[6] == 0) egStart(use,6);
							} else {
#ifdef _DEBUG      
								fprintf(stderr, "Ran out of synth voices!     \r",         
								        c, evt[1]);
#endif		
							}
							break;// not the best method but it breaks only when a note on is
						}// if velo == 0 it should be handled as noteoff...
							// ...so its necessary that here follow the noteoff routine
						case SND_SEQ_EVENT_NOTEOFF: 
#ifdef _DEBUG      
							fprintf(stderr, "Note Off event on Channel %2d: %5d      \r",         
							        c, evt[1]);
#endif							
							engine* voice=&mini->noteson[c]->e;
							if (voice)
						{
							egStop(voice,0);  
							float* EGrepeat=voice->EGrepeat;
							if (EGrepeat[1] == 0) egStop(voice,1);  
							if (EGrepeat[2] == 0) egStop(voice,2); 
							if (EGrepeat[3] == 0) egStop(voice,3); 
							if (EGrepeat[4] == 0) egStop(voice,4);  
							if (EGrepeat[5] == 0) egStop(voice,5);  
							if (EGrepeat[6] == 0) egStop(voice,6);
						}
							break;      
#ifdef _DEBUG      
						default:
							fprintf(stderr,"unknown event %d on Channel %2d: %5d   \r",ev->type, 
							        c, evt[1]);
#endif		
					}// end of switch
				}
			}
	}
	lv2_event_increment(in_iterator);
}

// ******************************************** OSC handling for editors ***********************
//!\name OSC routines
//!{ 
/** @brief OSC error handler 
 *
 * @param num errornumber
 * @param pointer msg errormessage
 * @param pointer path where it occured
 */
static inline void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

/** catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods 
 *
 * @param pointer path osc path
 * @param pointer types
 * @param argv pointer to array of arguments 
 * @param argc amount of arguments
 * @param pointer data
 * @param pointer user_data
 * @return int 0 if everything is ok, 1 means message is not fully handled
 * */
static inline int generic_handler(const char *path, const char *types, lo_arg **argv,
                                  int argc, void *data, void *user_data)
{

	if ( (argv[0]->i < _MULTITEMP) && (argv[1]->i < _CHOICEMAX) )
	{	
		((minicomputer*) user_data)->engines[argv[0]->i]->choice[argv[1]->i]=argv[2]->i;
		return 0;
	}
	else return 1;
	
}

/** specific message handler
 *
 * @param pointer path osc path
 * @param pointer types
 * @param argv pointer to array of arguments 
 * @param argc amount of arguments
 * @param pointer data
 * @param pointer user_data
 * @return int 0 if everything is ok, 1 means message is not fully handled
 */
static inline int foo_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
	minicomputer* mini= (minicomputer*) data;
    /* example showing pulling the argument values out of the argv array */
   int voice =  argv[0]->i;
   engine* voice=mini->engines[voice];
   int i =  argv[1]->i;
   if ((voice<_MULTITEMP)&&(i>0) && (i<_PARACOUNT))  {
   	voice->parameter[i]=argv[2]->f;
   }
	float ** EG=voice->EG;
	float * EGrepeat=voice->EGrepeat;
   switch (i) {
   	 // reset the filters 
   	 case 0:{
   	 	voice->low[0]	= 0.f;
   	 	voice->high[0]	= 0.f;
   	 	voice->band[0] = 0.f;
   	 	voice->low[1]	= 0.f;
   	 	voice->high[1]	= 0.f;
   	 	voice->band[1] = 0.f;
   	 	voice->low[2]	= 0.f;
   	 	voice->high[2]	= 0.f;
   	 	voice->band[2] = 0.f;
   	 	voice->phase[1] = 0.f;
   	 	voice->phase[2] = 0.f;
   	 	voice->phase[3] = 0.f;
		memset(voice->delayBuffer,0,sizeof(voice->delayBuffer));
   	 break;}
   	 
   	 case 60:EG[1][1]=argv[2]->f;break;
   	 case 61:EG[1][2]=argv[2]->f;break;
   	 case 62:EG[1][3]=argv[2]->f;break;
   	 case 63:EG[1][4]=argv[2]->f;break;
   	 case 64:
   	 {
   	 	EGrepeat[1] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[1] > 0 ) egStart(voice,1);
   	 	break;
   	 }
   	 case 65:EG[2][1]=argv[2]->f;break;
   	 case 66:EG[2][2]=argv[2]->f;break;
   	 case 67:EG[2][3]=argv[2]->f;break;
   	 case 68:EG[2][4]=argv[2]->f;break;
   	  case 69:
   	 {
   	 	EGrepeat[2] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[2] > 0 ) egStart(voice,2);
   	 	break;
   	 }
   	 case 70:EG[3][1]=argv[2]->f;break;
   	 case 71:EG[3][2]=argv[2]->f;break;
   	 case 72:EG[3][3]=argv[2]->f;break;
   	 case 73:EG[3][4]=argv[2]->f;break;
   	  case 74:
   	 {
   	 	EGrepeat[3] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[3] > 0 ) egStart(voice,3);
   	 	break;
   	 }
   	 case 75:EG[4][1]=argv[2]->f;break;
   	 case 76:EG[4][2]=argv[2]->f;break;
   	 case 77:EG[4][3]=argv[2]->f;break;
   	 case 78:EG[4][4]=argv[2]->f;break; 
   	  case 79:
   	 {
   	 	EGrepeat[4] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[4] > 0 ) egStart(voice,4);
   	 	break;
   	 }
   	 case 80:EG[5][1]=argv[2]->f;break;
   	 case 81:EG[5][2]=argv[2]->f;break;
   	 case 82:EG[5][3]=argv[2]->f;break;
   	 case 83:EG[5][4]=argv[2]->f;break;
   	  case 84:
   	 {
   	 	EGrepeat[5] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[5] > 0 ) egStart(voice,5);
   	 	break;
   	 }
   	 case 85:EG[6][1]=argv[2]->f;break;
   	 case 86:EG[6][2]=argv[2]->f;break;
   	 case 87:EG[6][3]=argv[2]->f;break;
   	 case 88:EG[6][4]=argv[2]->f;break;
   	  case 89:
   	 {
   	 	EGrepeat[6] = (argv[2]->f>0) ? 1:0;
   	 	if (EGrepeat[6] > 0 ) egStart(voice,6);
   	 	break;
   	 }
   	 case 102:EG[0][1]=argv[2]->f;break;
   	 case 103:EG[0][2]=argv[2]->f;break;
   	 case 104:EG[0][3]=argv[2]->f;break;
   	 case 105:EG[0][4]=argv[2]->f;break;
   	 
   }
#ifdef _DEBUG
   printf("%i %i %f \n",voice,i,argv[2]->f);
#endif   
    return 0;
}

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return miniDescriptor;
	default:
		return NULL;
	}
}

void cleanupMinicomputer(LV2_Handle instance) {
	minicomputer* mini=(minicomputer*)instance;
	lo_server_thread_free(mini->st); 	
	free(instance);
}
