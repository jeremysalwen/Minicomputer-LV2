/** Minicomputer
 * industrial grade digital synthesizer
 * editorsoftware
 * Copyright 2007,2008 Malte Steiner
 * This file is part of Minicomputer, which is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Minicomputer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <lo/lo.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
snd_seq_t *open_seq();
 snd_seq_t *seq_handle;
  int npfd;
  struct pollfd *pfd;

lo_address t;
// some common definitions
#include "../common.h"

#include "Memory.h"
Memory Speicher;
#include "syntheditor.h"
UserInterface Schaltbrett;
/** open an Alsa Midiport for accepting programchanges and later more...
 */
snd_seq_t *open_seq() {

  snd_seq_t *seq_handle;
  int portid;

  if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "MinicomputerEditor");
  if ((portid = snd_seq_create_simple_port(seq_handle, "MinicomputerEditor",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return(seq_handle);
}
/*
Fl_Double_Window* make_window() {
  Fl_Double_Window* w;
  { Fl_Double_Window* o = new Fl_Double_Window(475, 330);
    w = o;
   
     { Fl_Dial* knob1 = new Fl_Dial(25, 25, 25, 25,"f1");
      knob1->callback((Fl_Callback*)callback);
}
    {Fl_Dial* o =new Fl_Dial(65, 25, 25, 25, "f2");
	o->callback((Fl_Callback*)callback);
}
   {Fl_Dial* o = new Fl_Dial(105, 25, 25, 25, "f3");
o->callback((Fl_Callback*)callback);
}
    o->end();

  
}
  return w;
}*/
/*
void reloadSoundNames()
{
  int i;
  for (i = 0;i<8;++i)
  {
  	Schaltbrett.soundchoice[i]->clear();
  } 
  
  Speicher.load();
  
  for (i=0;i<512;i++) 
  {
  	Schaltbrett.soundchoice[0]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[1]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[2]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[3]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[4]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[5]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[6]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[7]->add(Speicher.getName(0,i).c_str());
  }
}*/
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
    case SND_SEQ_EVENT_PGMCHANGE:
      {
		int channel = ev->data.control.channel;
		int value = ev->data.control.value;
#ifdef _DEBUG      
        fprintf(stderr, "Programchange event on Channel %2d: %2d %5d       \r",
                channel,  ev->data.control.param,value);
#endif		
	// see if its the control channel
	if (ev->data.control.channel == 8)
	{ // perform multi program change
		// first a range check
		if ((value>-1) && (value <128))
		{
			Schaltbrett.changeMulti(value);
		}
	}
	else if ((channel>=0) && (channel<8)) 
	{
		//program change on the sounds
		if ((value>-1) && (value <128))
		{
			Schaltbrett.changeSound(channel,value);
		}

	}
        break;
      }
    /*
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
      }*/
      /*
      case SND_SEQ_EVENT_PITCHBEND:
      {
#ifdef _DEBUG      
         fprintf(stderr,"Pitchbender event on Channel %2d: %5d   \r", 
                ev->data.control.channel, ev->data.control.value);
#endif		
                if (ev->data.control.channel<_MULTITEMP)
               	 modulator[ev->data.control.channel][2]=(float)ev->data.control.value*0.0001221f; // /8192.f;
        break;
      }*/   
     }// end of switch
    snd_seq_free_event(ev);
   }// end of first while, emptying the seqdata queue
  } while (1==1);// doing forever, was  (snd_seq_event_input_pending(seq_handle, 0) > 0);
  return 0;
}

int main(int argc, char **argv)
{
// ------------------------ midi init ---------------------------------
  pthread_t midithread;
  seq_handle = open_seq();
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
    
    // create the thread and tell it to use Midi::work as thread function
	int err = pthread_create(&midithread, NULL, midiprocessor,seq_handle);
// ------------------------ osc init ---------------------------------
   t = lo_address_new(NULL, _OSCPORT);

// ------------------------ create gui --------------
	Fenster* w =Schaltbrett.make_window();
  //
  for (int i = 0;i<8;++i)
  {
  	//printf("bei %i\n",i);
  	//fflush(stdout);
  	//Schaltbrett.soundchoice[i]->clear();
  } 
  	Speicher.load();
  	//printf("und load...\n");
  /*
  for (int i=0;i<512;i++) 
  {
  	Schaltbrett.soundchoice[0]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[1]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[2]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[3]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[4]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[5]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[6]->add(Speicher.getName(0,i).c_str());
  	Schaltbrett.soundchoice[7]->add(Speicher.getName(0,i).c_str());
  }*/
  Speicher.loadMulti();
  /*for (int i=0;i<128;i++) 
  {
  	Schaltbrett.multichoice->add(Speicher.multis[i].name);
  }*/
  //printf("weiter...\n");
  Fl::lock();
  w->show(argc, argv);
    /* an address to send messages to. sometimes it is better to let the server
     * pick a port number for you by passing NULL as the last argument */
    


	
		//lo_send(t, "/a/b/c/d", "f",10.f);
	
    return Fl::run();
}
