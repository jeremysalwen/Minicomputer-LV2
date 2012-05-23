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

#include "minicomputer.h"

/**  FIXED
 * start the envelope generator
 * called by a note on event for that voice
 * @param the voice number
 * @param the number of envelope generator
 */

static inline void egStart (EG* eg, const unsigned int number)
{
	eg->EGtrigger[number]=1;
	voice->EG[number][7] = 0.0f;// state
	eg->EGstate = 0;// state  
	eg->EGFaktor = 0.f;
	//printf("start %i", voice);
}
/** FIXED
 * set the envelope to release mode
 * should be called by a related noteoff event
 * @param the voice number
 * @param the number of envelope generator
 */
static inline void egStop (EG* voice,const unsigned int number)
{
	// if (EGrepeat[number] == 0) 
	eg->EGtrigger = 0; // triggerd
	eg->EGstate = 0; // target
	// printf("stop %i", voice);
}
/** FIXED
 * calculate the envelope, done in audiorate to avoide zippernoise
 * @param the voice number
 * @param the number of envelope generator
 */
static inline float egCalc (EG* eg, envelope_settings* es, float srDivisor)
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
	if (eg->EGtrigger != 1)
	{
		int i = eg->EGstate; 
		if (i == 1){ // attack
			if (eg->EGFaktor<1.00f) eg->EGFaktor += 0.002f;

			eg->state += es->attack*srDivisor*eg->EGFaktor;

			if (eg->state>=1.0f)// Attackphase is finished
			{
				eg->state=1.0f;
				eg->EGstate=2;
				eg->EGFaktor = 1.f; // triggerd

			}
		}
		else if (i == 2)
		{ // decay
			if (eg->state > es->sustain)
			{
				eg->state -= es->decay*srDivisor*eg->EGFaktor;
			}
			else 
			{
				if (eg->EGrepeat==0)
				{
					eg->EGstate=3; // stay on sustain
				}
				else
				{
					eg->EGFaktor = 1.f; // triggerd
					egStop(voice,number);// continue to release
				}
			}
			// what happens if sustain = 0? envelope should go in stop mode when decay reached ground
			if (eg->state<0.0f) 
			{	
				eg->state=0.0f;
				if (eg->EGrepeat==0)
				{
					eg->EGstate=4; // released
				}
				else
				{
					egStart(voice,number);// repeat
				}
			}

		} // end of decay
		else if ((i == 0) && (eg->state>0.0f))
		{
			/* release */

			if (eg->EGFaktor>0.025f) eg->EGFaktor-= 0.002f;
			eg->state -=es->release*srDivisor*eg->EGFaktor;//*EG[number][6];

			if (eg->state<0.0f) 
			{	
				eg->state=0.0f;
				if (eg->EGrepeat==0)
				{
					eg->EGstate=4; // released
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
		eg->EGtrigger = 0;
		eg->EGstate = 1; // target
	}
	return eg->state;
}

static void normalize_parameters(minicomputer* mini) {
	mini->osc1.boost_factor=mini->osc1.boost_modulation>0?100.0f:1.0f;
	mini->osc2.boost_factor=mini->osc1.boost_modulation>0?100.0f:1.0f;
	//etc...
}
static engine* use_note_minicomputer(minicomputer* mini, unsigned char index) {
	engineblock* result=mini->freeblocks.next;

	if(&result->h==&mini->freeblocks) {
		return NULL;
	}
	listheader rh=result->h;

	mini->freeblocks.next=rh.next;
	rh.next->h.previous=(engineblock*)&mini->freeblocks; //using the fact that the next index is stored first;
	rh.previous=(engineblock*)&mini->inuse; //using the fact that the next index is stored first;
	rh.next=mini->inuse;
	if(mini->inuse) {
		mini->inuse->h.previous=result;
	}
	mini->inuse=result;
	mini->noteson[index]=result;
	return &result->e;
}


static inline void handlemidi(minicomputer* mini, unsigned int maxindex) {
	while(lv2_event_is_valid(&mini->in_iterator)) {
		uint8_t* data;
		LV2_Event* event= lv2_event_get(&mini->in_iterator,&data);
		if (event->type == 0) {
			mini->event_ref->lv2_event_unref(mini->event_ref->callback_data, event);
		} else if(event->type==mini->midi_event_id) {
			if(event->frames > maxindex) {
				break;
			} else {
				const uint8_t* evt=(uint8_t*)data;
				uint8_t command=MIDI_COMMANDMASK & evt[0];
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
								mini->modulator[ mod_wheel]=evt[2]*0.007874f; // /127.f;
								break;
							case 12:
								mini->modulator[ mod_cc12]=evt[2]*0.007874f;// /127.f;
								break;
							case 2:
								mini->modulator[ mod_cc2]=evt[2]*0.007874f;// /127.f;
								break;
							case 3:
								mini->modulator[ mod_cc3]=evt[2]*0.007874f;// /127.f;
								break;
							case 4:  
								mini->modulator[ mod_cc4]=evt[2]*0.007874f;// /127.f;
								break;
							case 5: 
								mini->modulator[ mod_cc5]=evt[2]*0.007874f;// /127.f;
								break;
							case 14:  
								mini->modulator[ mod_cc14]=evt[2]*0.007874f;// /127.f;
								break;
							case 15:
								mini->modulator[ mod_cc15]=evt[2]*0.007874f;// /127.f;
								break;
							case 16: 
								mini->modulator[ mod_cc16]=evt[2]*0.007874f;// /127.f;
								break;
							case 17:  
								mini->modulator[ mod_cc17]=evt[2]*0.007874f;// /127.f;
								break;
						}
					case MIDI_PITCHBEND:
						;//<---PLEASE THE C SYNTAX GODS.
						unsigned int value=evt[1] |  (evt[2]<<8);
#ifdef _DEBUG      
						fprintf(stderr,"Pitchbender event on Channel %2d: %5d   \r", 
						        c,value);
#endif		
						mini->modulator[mod_pitch_bend]=value*0.0001221f; // /8192.f;
						break;
					case MIDI_CHANPRESS:
#ifdef _DEBUG      
						fprintf(stderr,"touch event on Channel %2d: %5d   \r", 
						        c,evt[1]);
#endif	
						mini->modulator[ mod_touch]=(float)evt[1]*0.007874f;
						break;
					case MIDI_NOTEON:

#ifdef _DEBUG      
						fprintf(stderr, "Note On event on Channel %2d: %5d       \r",
						        c, ev->data.note.note);
#endif		
						if (evt[2]>0)
					{
						engine* use=use_note_minicomputer(mini,evt[1]);
						if(use) {
							use->lastnote=evt[1];
							use->midif=midi2freq[evt[1]];// lookup the frequency
							use->mod_midi_note=evt[1]*0.007874f;// fill the value in as normalized modulator
							use->mod_midi_velocity=(float)1.f-(evt[2]*0.007874f);// fill in the velocity as modulator
							egStart(use,0);// start the engines!
							int* EGrepeat=use->EGrepeat;
							if (EGrepeat[1] == 0) egStart(use,1);
							if (EGrepeat[2] == 0) egStart(use,2);
							if (EGrepeat[3] == 0) egStart(use,3);
							if (EGrepeat[4] == 0) egStart(use,4);
							if (EGrepeat[5] == 0) egStart(use,5);
							if (EGrepeat[6] == 0) egStart(use,6);
						} else {
#ifdef _DEBUG      
							fprintf(stderr, "Ran out of synth voices!     \r");
#endif		
						}
						break;// not the best method but it breaks only when a note on is
					}// if velo == 0 it should be handled as noteoff...
						// ...so its necessary that here follow the noteoff routine
					case  MIDI_NOTEOFF: 
						; //<--APPEASE THE C SYNTAX GODS
#ifdef _DEBUG      
						fprintf(stderr, "Note Off event on Channel %2d: %5d      \r",         
						        c, evt[1]);
#endif							
						engine* voice=&mini->noteson[c]->e;
						if (voice)
					{
						egStop(voice,0);  
						int* EGrepeat=voice->EGrepeat;
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
		lv2_event_increment(&mini->in_iterator);
	}
}
float calcfilters(filters* filts) {
	//----------------------- actual filter calculation -------------------------
	// first filter
	float reso = filts->q[0]; // for better scaling the volume with rising q
	filts->low[0] = filts->low[0] + filts->f[0] * filts->band[0];
	filts->high[0] = ((reso + ((1.f-reso)*0.1f))*temp) - filts->low[0] - (reso*filts->band[0]);
	filts->band[0]= filts->f[0] * filts->high[0] + filts->band[0];

	reso = filts->q[1];
	// second filter
	filts->low[1] = filts->low[1] + filts->f[1] * filts->band[1];
	filts->high[1] = ((reso + ((1.f-reso)*0.1f))*temp) - filts->low[1] - (reso*filts->band[1]);
	filts->band[1]= f[currentvoice][1] * high[currentvoice][1] + band[currentvoice][1];

	// third filter
	reso = filts->q[2];
	filts->low[2] = filts->low[2] + filts->f[2] * band[currentvoice][2];
	filts->high[2] = ((reso + ((1.f-reso)*0.1f))*temp) - filts->low[2] - (reso*filts->band[2]);
	filts->band[2]= filts->f[2] * filts->high[currentvoice][2] + filts->ban[2];

	return (filts->low[0]*filts->v[0])+filts->band[1]*filts->v[1]+filts->band[2]*filts->v[2];

}

float approx_sine(float f) {
	return f*(2-f*f*0.1472725f);// dividing by 6.7901358, i.e. the least squares polynomial
}
filter_settings morph_filters(filter_settings in1, filter_settings in2, float mo) {
	float morph=(1.0f-mo);
	
	filter_settings result;
	
	result.f = in1.f*morph+in2.f*mo;
	result.q = in1.q*morph+in2.q*mo;
	result.v = in1.v*morph+in2.v*mo;
}

float wrap_phase(float phase) {
	if(phase  >= tabf)
	{
		phase-= tabf;
		// if (*phase>=tabf) *phase = 0; //just in case 
	}
	if(phase< 0.f)
	{
		phase+= tabf;
		//      if(*phase < 0.f) *phase = tabf-1;
	}
	return phase;
}

int phase_to_table_index(float phase) {
	// an union for a nice float to int casting trick which should be fast
	typedef union
	{
		int i;
		float f;
	} INTORFLOAT;
	INTORFLOAT p1 __attribute__((aligned (16)));
	INTORFLOAT bias; // the magic number
	bias.i = (23 +127) << 23;// generating the magic number
	
	p1.f =  phase;
	p1.f += bias.f;
	p1.i -= bias.i;
	
	int result=p1.i&tabm;//i%=tablesize;
	if (result<0) { result=tabm; }
	/*
	int i = (int) phase;// float to int, cost some cycles
						// hopefully this got optimized by compiler
	i%=TableSize;
	*/
	return result;
	
}

float calc_phase_inc(common_osc_params* osc, float midif, float tabx) {
	float fixed=(*osc->fix_frequency>0);
	float tf = *osc->fixed_frequency * fixed;
	tf+=(midif*(1.0f-fixed)*osc->tune_frequency);//TODO: This isn't really semitones
	tf+=(osc->boost_factor*osc->freq_mod1.amount)*mod[choi[osc->freq_mod1.type_p]];
	tf+=osc->freq_mod2.amount*mod[choi[osc->freq_mod2.type_p]];
	return tabx*tf;
}

float* lookup_mod_table(float [_MODCOUNT] mod, float port_value) {
	int index=(int)port_value;
	if(index>=_MODCOUNT || index<0) {
		index=0;
	}
	return mod+index;
}
static void run_minicomputer(LV2_Handle instance, uint32_t nframes) {
	minicomputer* mini= (minicomputer*)instance;
	float tf,tf1,tf2,tf3,ta1,ta2,ta3,morph,result,tdelay;
	float delayMod;

	float *bufferMixLeft = mini->MixLeft_p;
	float *bufferMixRight = mini->MixRight_p;
	float *bufferAux1 =mini->Aux1_p;
	float *bufferAux2 =mini->Aux2_p;
	
	float mod[_MODCOUNT];
	
	float * osc1_freq1_mod_val=lookup_mod_table (mod,*mini->osc1.freq_mod1.type_p);
	float * osc1_freq2_mod_val=lookup_mod_table (mod,*mini->osc1.freq_mod2.type_p);
	float * osc1_amp1_mod_val=lookup_mod_table (mod,*mini->osc1.amp_mod1.type_p);
	float * osc1_amp2_mod_val=lookup_mod_table (mod,*mini->osc1_amp_mod2.type_p);
	
	float * osc2_freq1_mod_val=lookup_mod_table (mod,*mini->osc2.freq_mod1.type_p);
	float * osc2_freq2_mod_val=lookup_mod_table (mod,*mini->osc2.freq_mod2.type_p);
	float * osc2_amp1_mod_val=lookup_mod_table (mod,*mini->osc2.amp_mod1.type_p);
	float * osc2_fm_amp_mod_val=lookup_mod_table (mod,*mini->osc2_fm_amp_mod.type_p);
	
	float * filter_morph1_mod_val=lookup_mod_table(mod,*mini->morph_mod1.type_p);
	float * filter_morph2_mod_val=lookup_mod_table(mod,*mini->morph_mod2.type_p);

	float * amp_mod_val=lookup_mod_table(mod,*mini->amp_mod.type_p);
	
	float * delay_mod_val=lookup_mod_table(mod,*mini->delay_mod.type_p);


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
		for (currentvoice=0;currentvoice<_MULTITEMP;++currentvoice) // for each voice
		{		
			engine* voice=engines+currentvoice;

			// calc the modulators
			float * mod = voice->modulator;
			mod[mod_envelope1] =1.f-egcalc(voice,1);
			mod[mod_envelope2] =1.f-egcalc(voice,2);
			mod[mod_envelope3]=1.f-egcalc(voice,3);
			mod[mod_envelope4]=1.f-egcalc(voice,4);
			mod[mod_envelope5]=1.f-egcalc(voice,5);
			mod[mod_envelope6]=1.f-egcalc(voice,6);
			/**
			 * calc the main audio signal
			 */
			// get the parameter settings
			float * param = voice->parameter;
			// casting floats to int for indexing the 3 oscillator wavetables with custom typecaster
			int ip1=phase_to_table_index(voice->phase1);
			int ip2=phase_to_table_index(voice->phase2);
			int ip3=phase_to_table_index(voice->mod_osc_phase);
			
			
#ifdef _prefetch
			__builtin_prefetch(&param[1],0,0);
			__builtin_prefetch(&param[2],0,1);
			__builtin_prefetch(&param[3],0,0);
			__builtin_prefetch(&param[4],0,0);
			__builtin_prefetch(&param[5],0,0);
			__builtin_prefetch(&param[7],0,0);
			__builtin_prefetch(&param[11],0,0);
#endif
			voice->mod_osc_phase=wrap_phase(voice->mod_osc_phase);

			
			unsigned int * choi = voice->choice;
			//modulator [currentvoice][14]=oscillator(parameter[currentvoice][90],choice[currentvoice][12],&mod_osc_phase);
			// write the oscillator 3 output to modulators
			mod[mod_osc] = table[choi[12]][ip3] ;

			// --------------- calculate the parameters and modulations of main oscillators 1 and 2
			oscillator_settings* osc1=&mini->osc1;
			
			ta1 = (*osc1_amp1_mod_val)*osc1->amp_mod1.amount; // osc1 first ampmod

#ifdef _prefetch
			__builtin_prefetch(&voice->phase1,0,2);
			__builtin_prefetch(&voice->phase2,0,2);
#endif

			ta1+= osc1->amp_mod2.amount*(*osc1_amp2_mod_val);// osc1 second ampmod


			// generate phase of oscillator 1
			voice->phase1+= calc_phase_inc(&mini->osc1,voice->midif,mini->tabx);

			if(voice->phase1  >= tabf)
			{
				voice->phase1-= tabf;
				//if (mini->osc2_sync_p>0.f) phase2= 0; // sync osc2 to 1
				// branchless sync:
				voice->phase2-= voice->phase2*mini->osc2_sync_p;

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

			if(voice->phase1< 0.f)
			{
				voice->phase1+= tabf;
				//	if(*phase < 0.f) *phase = tabf-1;
			}
			float osc1_sample = table[choi[4]][ip1] ;
			//}
			//osc1 = oscillator(tf,choice[currentvoice][4],&phase1);
			mod[mod_osc1_fm_out]=osc1_sample*(osc1->fm_output_vol_p*(1.f+ta1));//+parameter[currentvoice][13]*ta1);

			// ------------------------ calculate oscillator 2 ---------------------
			// first the modulations and frequencys

			ta2 = param[23]*(*osc2_amp1_mod_val); // osc2 first amp mod
			ta3 = param[25]*(*osc2_fm_amp_mod_val);// osc2 second amp mod
			//tf/=3.f;		
			//ta/=2.f;
			mod[mod_osc2_fm_out] = (param[28]+param[28]*(1.f-ta3));// osc2 fm out

			// then generate the actual phase:
			voice->phase2+= calc_phase_inc(&mini->osc2,voice->midif,mini->tabx);
			voice->phase2=wrap_phase(voice->phase2);

			float osc2_sample = table[choi[5]][ip2] ;
			mod[mod_osc2_fm_out] *= osc2_sample;// osc2 fm out

			// ------------------------------------- mix the 2 oscillators pre filter
			float temp=(param[14]*(1.f-ta1));
			temp*=osc1_sample;
			temp+=osc2_sample*(param[29]*(1.f-ta2));
			temp*=0.5f;// get the volume of the sum into a normal range	
			temp+=anti_denormal;

			// ------------- calculate the filter settings ------------------------------
			float morph_mod_amt = 1.f-(*filter_morph1_mod_val)*param[38];
			morph_mod_amt+= 1.f-(*filter_morph2_mod_val)*param[48];
			float mo = param[56]*morph_mod_amt;


			//The next three lines just clip to the [0,1] interval
			float clib1 = fabs (mo);
			float clib2 = fabs (mo-1.0f);
			mo = 0.5*(clib1 + 1.0f-clib2);

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
			filter_settings s1;
			filter_settings s2;
			filter_settings s3;
			
			morph_filters(s1,voice->filter_settings[0][0],voice->filter_settings[0][1],mo);
			morph_filters(s2,voice->filter_settings[1][0],voice->filter_settings[1][1],mo);
			morph_filters(s3,voice->filter_settings[2][0],voice->filter_settings[2][1],mo);

			float srate=mini->srate;
			s1.f*=srate;
			s2.f*=srate;
			s3.f*=srate;

			s1.f = approx_sine(s1.f);
			s2.f = approx_sine(s2.f);
			s3.f = approx_sine(s3.f);
			
			mod[mod_filter]=calcfilters(&voice->filts);
			//---------------------------------- amplitude shaping

			result = (1.f-(*amp_mod_val)*param[100] );///_multitemp;
				result *= mod[mod_filter];
			result *= egcalc(voice,0);// the final shaping envelope

			// --------------------------------- delay unit
			if( voice->delayi>= delaybuffersize ) {
				voice->delayi = 0;
			}
			delaymod = 1.f-(param[110]* (*delay_mod_val));

			voice->delayj = voice->delayi- ((param[111]* maxdelaytime)*delaymod);

			if( voice->delayj  < 0 ) {
				voice->delayj += delaybuffersize;
			}else if (voice->delayj>delaybuffersize) {
				voice->delayj= 0;
			}
			tdelay = result * param[114] + (voice->delaybuffer [ voice->delayj ] * param[112] );
			tdelay += anti_denormal;
			voice->delaybuffer[voice->delayi ] = tdelay;

			mod[mod_delay]= tdelay;
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

static void initEngine(engine* voice) {
	float* EGtrigger=voice->EGtrigger;
	float* parameter=voice->parameter;
	float* modulator=voice->modulator;
	float* low=voice->low;
	float* high=voice->high;
	for (i=0;i<8;i++) // i is the number of envelope
	{
		voice->envelope_generator[i].EGTrigger=0;
		voice->envelope_generator[i].EGstate=4; // released
	}
	modulator[mod_none] =0.f;// the none modulator, doing nothing
	for (unsigned int i=0;i<3;++i) 
	{
		low[i]=0;
		high[i]=0;
	}
}
static void initEnvelopeSettings(envelope_settings* es) {
	es->attack=0.01f;
	es->decay=0.01f;
	es->sustain=1.0f;
	es->release=0.0001f;
	es->EGrepeat=0;
}

static void initParameters(float parameter[_PARACOUNT]) {
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
}

static void initEngines(minicomputer* mini) {
	minicomputer* mini= malloc(sizeof(minicomputer));
	memset(mini->noteson,0,sizeof(mini->noteson));
	mini->inuse=NULL;
	mini->freeblocks.previous=(minicomputer*)&mini->freeblocks;
	mini->freeblocks.next=(minicomputer*)&mini->freeblocks;

	initEnvelopeSettings(&mini->es);
	initParameters (mini->parameter);
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

static void waveTableInit() {
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

static void initOSC(minicomputer* mini) {
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
static LV2_Handle instantiateMinicomputer(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features) 
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
	port[10] = jack_port_register(client, "aux out 1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[11] = jack_port_register(client, "aux out 2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);

	// would like to create mix ports last because qjackctrl tend to connect automatic the last ports
	port[8] = jack_port_register(client, "mix out left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);
	port[9] = jack_port_register(client, "mix out right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);


	bufsize = jack_get_buffer_size(client);

	// handling the sampling frequency
	tabX = 4096.f / s_rate;
	srate = 3.145f/ s_rate;
	srDivisor = 1.f / s_rate * 100000.f;
	// depending on it the delaybuffer
	maxDelayTime = (int)s_rate;
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

static void free_note_minicomputer(minicomputer* mini, unsigned char index) {
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
			voice->phase1 = 0.f;
			voice->phase2 = 0.f;
			voice->mod_osc_phase = 0.f;
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

static void cleanupMinicomputer(LV2_Handle instance) {
	minicomputer* mini=(minicomputer*)instance;
	lo_server_thread_free(mini->st); 	
	free(instance);
}


static void connect_port_minicomputer(LV2_Handle instance, uint32_t port, void *data){

}	