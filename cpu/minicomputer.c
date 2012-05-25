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

#include "minicomputer.h"

/**  FIXED
 * start the envelope generator
 * called by a note on event for that voice
 * @param the voice number
 * @param the number of envelope generator
 */

static inline void egStart (EG* eg)
{
	eg->EGtrigger=1;
	eg->EGstate = 0;// state  
	eg->Faktor = 0.f;
	//printf("start %i", voice);
}
/** FIXED
 * set the envelope to release mode
 * should be called by a related noteoff event
 * @param the voice number
 * @param the number of envelope generator
 */
static inline void egStop (EG* eg)
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
			if (eg->Faktor<1.00f) eg->Faktor += 0.002f;

			eg->state += (*es->attack_p)*srDivisor*eg->Faktor;

			if (eg->state>=1.0f)// Attackphase is finished
			{
				eg->state=1.0f;
				eg->EGstate=2;
				eg->Faktor = 1.f; // triggerd

			}
		}
		else if (i == 2)
		{ // decay
			if (eg->state > (*es->sustain_p))
			{
				eg->state -= (*es->decay_p)*srDivisor*eg->Faktor;
			}
			else 
			{
				if (es->EGrepeat_c==0)
				{
					eg->state=3; // stay on sustain
				}
				else
				{
					eg->Faktor = 1.f; // triggerd
					egStop(eg);// continue to release
				}
			}
			// what happens if sustain = 0? envelope should go in stop mode when decay reached ground
			if (eg->state<0.0f) 
			{	
				eg->state=0.0f;
				if (es->EGrepeat_c==0)
				{
					eg->EGstate=4; // released
				}
				else
				{
					egStart(eg);// repeat
				}
			}

		} // end of decay
		else if ((i == 0) && (eg->state>0.0f))
		{
			/* release */

			if (eg->Faktor>0.025f) eg->Faktor-= 0.002f;
			eg->state -=(*es->release_p)*srDivisor*eg->Faktor;//*EG[number][6];

			if (eg->state<0.0f) 
			{	
				eg->state=0.0f;
				if (es->EGrepeat_c==0)
				{
					eg->EGstate=4; // released
				}
				else
				{
					egStart(eg);// repeat
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
/**
 *  Note that this assumes that we have already done lookup_mod_table, so
 *  the mod_val field references the actual location of the modulator value.
 **/
float modulator_get_val(mod_selector selector) {
	return (*selector.amount_p)*(*selector.mod_val);
}
static void calc_osc_factors(common_osc_params* osc) {
	osc->boost_factor_c=(*osc->boost_modulation_p>0)?100.0f:1.0f;
	osc->fixed_c=(*osc->fix_frequency_p)>0;
	osc->volume_c=(*osc->volume_p);
}

static engine* use_note_minicomputer(minicomputer* mini, unsigned char index) {
	engineblock* result=mini->freeblocks.next;

	if(&result->h==&mini->freeblocks) {
		return NULL;
	}
	listheader rh=result->h;

	mini->freeblocks.next=rh.next;
	rh.next->h.previous=(engineblock*)&mini->freeblocks; //using the fact that the next index is stored first;

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
							
							egStart(use->envelope_generator+0);// start the engines!
							for(int i=1; i<7; i++) {
								if (mini->es[i].EGrepeat_c == 0) {
									egStart(use->envelope_generator+i);
								}
							}
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
						if (voice) {
							egStop(voice->envelope_generator+0);  
							for(int i=1; i<7; i++) {
								if (mini->es[i].EGrepeat_c == 0) egStop(voice->envelope_generator+i);
							}
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
void dofilter(filter_settings settings, filter* filter,float sample) {
	float reso = settings.q; // for better scaling the volume with rising q
	filter->low = filter->low + settings.f * filter->band;
	filter->high = ((reso + ((1.f-reso)*0.1f))*sample) - filter->low - (reso*filter->band);
	filter->band= settings.f * filter->high + filter->band;
}

float approx_sine(float f) {
	return f*(2-f*f*0.1472725f);// dividing by 6.7901358, i.e. the least squares polynomial
}
filter_settings get_filter_settings(filter_ports ports) {
	filter_settings result;
	result.f=*ports.f_p;
	result.q=*ports.q_p;
	result.v=*ports.v_p;
}

filter_settings morph_filters(filter_settings in1, filter_settings in2, float mo) {
	float morph=(1.0f-mo);
	
	filter_settings result;
	
	result.f = in1.f*morph+in2.f*mo;
	result.q = in1.q*morph+in2.q*mo;
	result.v = in1.v*morph+in2.v*mo;
}

float wrap_phase(float phase) {
	if(phase  >= tabF)
	{
		phase-= tabF;
		// if (*phase>=tabf) *phase = 0; //just in case 
	}
	if(phase< 0.f)
	{
		phase+= tabF;
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
	
	int result=p1.i&tabM;//i%=tablesize;
	if (result<0) { result=tabM; }
	/*
	int i = (int) phase;// float to int, cost some cycles
						// hopefully this got optimized by compiler
	i%=TableSize;
	*/
	return result;
	
}

float calc_phase_inc(common_osc_params* osc, float midif, float tabx) {
	float tf = *osc->fixed_frequency_p * osc->fixed_c;
	tf+=*osc->tuned_frequency_p*(midif*(1.0f-osc->fixed_c));//TODO: This isn't really semitones
	tf+=osc->boost_factor_c*modulator_get_val(osc->freq_mod1);
	tf+=modulator_get_val(osc->freq_mod2);
	return tabx*tf;
}

void lookup_mod_table(float mod[_MODCOUNT], mod_selector* mod_sel) {
	int index=(int)(*mod_sel->type_p);
	if(index>=_MODCOUNT || index<0) {
		index=0;
	}
	mod_sel->mod_val=mod+index;
}
void lookup_mod_tables_osc(float mod[_MODCOUNT], common_osc_params* osc) {
	lookup_mod_table(mod,&osc->freq_mod1);
	lookup_mod_table(mod,&osc->freq_mod2);
	lookup_mod_table(mod,&osc->amp_mod1);
}
void lookup_mod_tables(minicomputer* mini) {
	lookup_mod_tables_osc(mini->modulator,&mini->osc1);
	lookup_mod_table(mini->modulator,&mini->osc1_amp_mod2);
	
	lookup_mod_tables_osc(mini->modulator,&mini->osc2);
	lookup_mod_table(mini->modulator,&mini->osc2_fm_amp_mod);
	
	lookup_mod_table(mini->modulator,&mini->morph_mod1);
	lookup_mod_table(mini->modulator,&mini->morph_mod2);

	lookup_mod_table(mini->modulator,&mini->amp_mod);
	
	lookup_mod_table(mini->modulator,&mini->delay_mod);
}
void calc_envelope_params(envelope_settings* es) {
	es->EGrepeat_c=(*es->EGrepeat_p)>0;
}
void calc_envelopes_params(minicomputer* mini) {
	for(int i=0; i<7; i++) {
		envelope_settings* es=&mini->es[i];
		calc_envelope_params(es);
		for(int j=0; j<_MULTITEMP; j++) {
			EG* eg=&mini->engines[j].e.envelope_generator[i];
			
			if (es->EGrepeat_c && eg->EGstate == 4) {
				egStart(eg);
			}
		}
	}
}

float* get_wavetable(float* port) {
	int index =(int)*port;
	if(index<0 || index>=_WAVECOUNT) {
		index=0;
	}
	return table[index];
}

static void run_minicomputer(LV2_Handle instance, uint32_t nframes) {
	minicomputer* mini= (minicomputer*)instance;

	float *bufferMixLeft = mini->MixLeft_p;
	float *bufferMixRight = mini->MixRight_p;
	float *bufferAux1 =mini->Aux1_p;
	float *bufferAux2 =mini->Aux2_p;
	
	calc_envelopes_params(mini);
	lookup_mod_tables(mini);
	calc_osc_factors(&mini->osc1);
	calc_osc_factors(&mini->osc2);
	
	filter_settings filt_settings[3][2];
	for(int x=0; x<3; x++) {
		for(int y=0; y<2; y++) {
			filt_settings[x][y]=get_filter_settings(mini->filt_settings[x][y]);
		}
	}

	float morph_c=*mini->morph_p;
	float osc2_sync_factor=1.0f-(*mini->osc2_sync_p>0);
	
	float delayBufferSize=2*mini->maxDelayTime;
	float delay_time_c=*mini->delay_time_p;
	//Clamp to the interval [0,mini->maxDelayTime];
	delay_time_c=0.5*(fabs(delay_time_c)+mini->maxDelayTime-fabs(delay_time_c-mini->maxDelayTime));
	
	
	float* osc1_table=get_wavetable(mini->osc1.waveform_p);
	float* osc2_table=get_wavetable(mini->osc2.waveform_p);
	float* mod_osc_table=get_wavetable(mini->mod_osc_waveform_p);
	
	float * mod = mini->modulator;
	
	/* main loop */
	register unsigned int index;
	for (index = 0; index < nframes; ++index) 
	{
		handlemidi(mini,index);
		bufferMixLeft[index]=0.f;
		bufferMixRight[index]=0.f;
		bufferAux1[index]=0.f;
		bufferAux2[index]=0.f;
		
		int ip3=phase_to_table_index(mini->mod_osc_phase);

		mini->mod_osc_phase+= *mini->mod_osc_freq_p * mini->tabX;
		mini->mod_osc_phase=wrap_phase(mini->mod_osc_phase);
			
		//modulator [currentvoice][14]=oscillator(parameter[currentvoice][90],choice[currentvoice][12],&mod_osc_phase);
		// write the oscillator 3 output to modulators
		mod[mod_osc] = mod_osc_table[ip3];

		mini->delayI++;
		if(mini->delayI >= delayBufferSize ) {
			mini->delayI = 0;
		}
		
		/*
		 * I dont know if its better to separate the calculation blocks, so I try it
		 * first calculating the envelopes
		 */
		register unsigned int currentvoice;
		for (currentvoice=0;currentvoice<_MULTITEMP;++currentvoice) // for each voice
		{		
			engine* voice=&mini->engines[currentvoice].e;

			// calc the modulators
			for(int i=0; i<6; i++) {
				mod[mod_envelope1+i] = 1.f-egCalc(&voice->envelope_generator[i+1],&mini->es[i+1],mini->srDivisor);
			}
			
			/**
			 * calc the main audio signal
			 */

			// casting floats to int for indexing the 3 oscillator wavetables with custom typecaster
			int ip1=phase_to_table_index(voice->phase1);
			int ip2=phase_to_table_index(voice->phase2);
			
			// --------------- calculate the parameters and modulations of main oscillators 1 and 2
			common_osc_params* osc1=&mini->osc1;
			
			float ta1 = modulator_get_val(osc1->amp_mod1);// osc1 first ampmod
			ta1+= modulator_get_val(mini->osc1_amp_mod2);// osc1 second ampmod

			// generate phase of oscillator 1
			voice->phase1+= calc_phase_inc(&mini->osc1,voice->midif,mini->tabX);

			if(voice->phase1  >= tabF)
			{
				voice->phase1-= tabF;
				//if (mini->osc2_sync_p>0.f) phase2= 0; // sync osc2 to 1
				// branchless sync:
				voice->phase2*=osc2_sync_factor;

				// if (*phase>=tabf) *phase = 0; //just in case of extreme fm
			}

			if(voice->phase1< 0.f)
			{
				voice->phase1+= tabF;
				//	if(*phase < 0.f) *phase = tabf-1;
			}
			float osc1_sample = osc1_table[ip1] ;
			//}
			//osc1 = oscillator(tf,choice[currentvoice][4],&phase1);
			mod[mod_osc1_fm_out]=osc1_sample*(*osc1->fm_output_vol_p*(1.f+ta1));//+parameter[currentvoice][13]*ta1);

			// ------------------------ calculate oscillator 2 ---------------------
			common_osc_params* osc2=&mini->osc2;
			
			// first the modulations and frequencys

			float ta2 = modulator_get_val(osc2->amp_mod1); // osc2 first amp mod
			float ta3 = modulator_get_val(mini->osc2_fm_amp_mod);// osc2 second amp mod
			//tf/=3.f;		
			//ta/=2.f;
			mod[mod_osc2_fm_out] = (*mini->osc2_fm_amp_mod.amount_p)*(1.f+1.f-ta3);// osc2 fm out

			// then generate the actual phase:
			voice->phase2+= calc_phase_inc(&mini->osc2,voice->midif,mini->tabX);
			voice->phase2=wrap_phase(voice->phase2);

			float osc2_sample = osc2_table[ip2] ;
			mod[mod_osc2_fm_out] *= osc2_sample;// osc2 fm out

			// ------------------------------------- mix the 2 oscillators pre filter
			float mixed_oscillator_sample=osc1_sample*(osc1->volume_c*(1.f-ta1));
			mixed_oscillator_sample+=osc2_sample*(osc2->volume_c*(1.f-ta2));
			mixed_oscillator_sample*=0.5f;// get the volume of the sum into a normal range	
			mixed_oscillator_sample+=anti_denormal;

			// ------------- calculate the filter settings ------------------------------
			float morph_mod_amt = 1.f-modulator_get_val(mini->morph_mod1);
			morph_mod_amt+= 1.f-modulator_get_val(mini->morph_mod2);
			float mo = morph_c*morph_mod_amt;//TODO: do we really want multiplicative modulation?


			//The next three lines just clip to the [0,1] interval
			float clib1 = fabs (mo);
			float clib2 = fabs (mo-1.0f);
			mo = 0.5*(clib1 + 1.0f-clib2);

			filter_settings s[3];
			
			for(int i=0; i<3; i++) {
				s[i]=morph_filters(filt_settings[i][0],filt_settings[i][1],mo);
				s[i].f*=mini->srate;
				s[i].f=approx_sine(s[i].f);
				dofilter(s[i],&voice->filt[i],mixed_oscillator_sample);
			}
			
			mod[mod_filter]=voice->filt[0].low*s[0].v + 
							voice->filt[1].band*s[1].v +
							voice->filt[2].band*s[2].v;
							
			//---------------------------------- amplitude shaping

			float result = (1.f-modulator_get_val(mini->amp_mod));///_multitemp;
			result *= mod[mod_filter];
			result *= egCalc(&voice->envelope_generator[0],&mini->es[0],mini->srDivisor);// the final shaping envelope

			// --------------------------------- delay unit

			float delaymod = 1.f-modulator_get_val(mini->delay_mod);

			int delayO = mini->delayI - ((delay_time_c* mini->maxDelayTime)*delaymod);

			if(delayO  < 0 ) {
				delayO += delayBufferSize;
			}else if (delayO>delayBufferSize) {
				delayO= 0;
			}
			float tdelay = result * (*mini->delay_amount_p) +
						  (voice->delayBuffer[delayO] * (*mini->delay_feedback_p));
			tdelay += anti_denormal;
			voice->delayBuffer[mini->delayI] = tdelay;

			mod[mod_delay]= tdelay;
			result += tdelay * (*mini->delay_volume_p);

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

static void initEngine(engine* voice,int delayBufferSize) {
	voice->delayBuffer=malloc(delayBufferSize);
	for (int i=0;i<7;i++) // i is the number of envelope
	{
		voice->envelope_generator[i].EGtrigger=0;
		voice->envelope_generator[i].EGstate=4; // released
	}
	for (unsigned int i=0;i<3;++i) 
	{
		voice->filt[i].low=0;
		voice->filt[i].high=0;
		voice->filt[i].band=0;
	}
}

static void initEngines(minicomputer* mini) {
	memset(mini->noteson,0,sizeof(mini->noteson));
	mini->freeblocks.previous=(minicomputer*)&mini->freeblocks;
	mini->freeblocks.next=(minicomputer*)&mini->freeblocks;

	mini->modulator[mod_none] =0.f;
	
	for(unsigned int i=0; i<_MULTITEMP; i++) {
		engineblock* e=&mini->engines[i];
		initEngine(&e->e,mini->maxDelayTime*2);
		e->h.next=mini->freeblocks.next;
		e->h.next->h.previous=e;
		mini->freeblocks.next=e;
		e->h.previous=(engineblock*)mini->freeblocks;
	}
}

static void waveTableInit() {
#define PI 3.14159265358979
	float increment = (float)(PI*2) / (float)TableSize;
	float x = 0.0f;
	float tri = -0.9f;

	// calculate wavetables
	for (int i=0; i<TableSize; i++)
	{
		table[0][i] = sinf(x+(2.0f*PI));
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

		table[8][i]= (
		              sinf(x+(2.0f*PI)) +
		              sinf(x*2.f+(2.0f*PI))+
		              sinf(x*3.f+(2.0f*PI))+
		              sinf(x*4.f+(2.0f*PI))*0.9f+
		              sinf(x*5.f+(2.0f*PI))*0.8f+
		              sinf(x*6.f+(2.0f*PI))*0.7f+
		              sinf(x*7.f+(2.0f*PI))*0.6f+
		              sinf(x*8.f+(2.0f*PI))*0.5f
		              ) / 8.0f;	

		table[9][i]= (
		              sinf(x+(2.0f*PI)) +
		              sinf(x*3.f+(2.0f*PI))+
		              sinf(x*5.f+(2.0f*PI))+
		              sinf(x*7.f+(2.0f*PI))*0.9f+
		              sinf(x*9.f+(2.0f*PI))*0.8f+
		              sinf(x*11.f+(2.0f*PI))*0.7f+
		              sinf(x*13.f+(2.0f*PI))*0.6f+
		              sinf(x*15.f+(2.0f*PI))*0.5f
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
	for (int i = 0;i<128;++i) midi2freq[i] = 8.1758f * pow(2,(i/12.f));
#undef PI
}

/** @brief initialization
 *
 * preparing for instance the waveforms
 */
static LV2_Handle instantiateMinicomputer(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features) 
{
	minicomputer* mini= malloc(sizeof(minicomputer));
	
	// handling the sampling frequency
	mini->tabX = 4096.f / s_rate;
	mini->srate = 3.145f/ s_rate;
	mini->srDivisor = 1.f / s_rate * 100000.f;
	// depending on it the delaybuffer
	mini->maxDelayTime = (int)s_rate;
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

	// generate the delaybuffers for each voice
	int k;
	mini->delayI=0;
#ifdef _DEBUG
	printf("bsize:%d %d\n",delayBufferSize,maxDelayTime);
#endif

} // end of initialization

static void free_note_minicomputer(minicomputer* mini, unsigned char index) {
	engineblock* result=mini->noteson[index];
	if(result) {
		result->h.previous->h.next=result->h.next;
		if(result->h.next) {
			result->h.next->h.previous=result->h.previous;
		}
		mini->noteson[index]=NULL;
		mini->freeblocks.previous->h.next=result;
		result->h.next=(engineblock*)&mini->freeblocks; //using the fact that the next index is stored first;
		result->h.previous=mini->freeblocks.previous;
		mini->freeblocks.previous=result;
	}	
}

// ******************************************** OSC handling for editors ***********************

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
			return &miniDescriptor;
		default:
			return NULL;
	}
}

static void cleanupMinicomputer(LV2_Handle instance) {
	minicomputer* mini=(minicomputer*)instance;
	for(int i=0; i<_MULTITEMP; i++) {
		free(mini->engines[i].e.delayBuffer);
	} 
	lo_server_thread_free(mini->st); 	
	free(instance);
}


static void connect_port_minicomputer(LV2_Handle instance, uint32_t port, void *data){

}