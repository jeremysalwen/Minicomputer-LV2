/** Minicomputer
 * industrial grade digital synthesizer
 * editorsoftware
 * Copyright 2007 Malte Steiner
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
lo_address t;
// some common definitions
#include "../common.h"

#include "Memory.h"
Memory Speicher;
#include "syntheditor.h"
UserInterface Schaltbrett;

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
int main(int argc, char **argv)
{
   t = lo_address_new(NULL, _OSCPORT);
  Fenster* w =Schaltbrett.make_window();
  //
  for (int i = 0;i<8;++i)
  {
  	printf("bei %i\n",i);
  	fflush(stdout);
  				Schaltbrett.soundchoice[i]->clear();
  } 
  	Speicher.load();
  	printf("und load...\n");
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
  }
  Speicher.loadMulti();
  for (int i=0;i<128;i++) 
  {
  	Schaltbrett.multichoice->add(Speicher.multis[i].name);
  }
  printf("weiter...\n");
  w->show(argc, argv);
    /* an address to send messages to. sometimes it is better to let the server
     * pick a port number for you by passing NULL as the last argument */
    


	
		//lo_send(t, "/a/b/c/d", "f",10.f);
	
    return Fl::run();
}
