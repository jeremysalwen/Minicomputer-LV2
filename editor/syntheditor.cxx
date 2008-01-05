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

#include "syntheditor.h"

// gcc -o synthEditor2 syntheditor.cxx -lfltk -llo
 Fl_Widget* Knob[8][_PARACOUNT];
 Fl_Widget* tab[8];
 Fl_Tabs* tabs;
 Fl_Value_Input* Display[8][13];

 Fl_Value_Input* paramon;  
 int currentParameter=0;
 unsigned int currentsound=0,currentmulti=0;
// unsigned int multi[128][8];
// string multiname[128];
 bool transmit;
 Fl_Choice* auswahl[8][17];
 Fl_Input_Choice* schoice[8];
//Fl_Chart * EG[7];
static void choicecallback(Fl_Widget* o, void*)
{
	if (transmit) lo_send(t, "/Minicomputer/choice", "iii",currentsound,((Fl_Choice*)o)->argument(),((Fl_Choice*)o)->value());
}

static void tabcallback(Fl_Widget* o, void* )
{
	//int* g;
	//g=(int*)e;
	
	//currentsound=*g;
	//printf("sound %i value  xtab %i\n",(int)((Fl_Group*)o)->argument(),*g);//currentsound);
	//fflush(stdout);
	Fl_Widget* e =((Fl_Tabs*)o)->value();
	for (int i=0; i<8;++i)
	{
		if (e==tab[i]){ 
			currentsound=i;
			break;
		}
	}	
	//else currentsound=1;
	printf("sound %i\n", currentsound );
	fflush(stdout);
}
static void callback(Fl_Widget* o, void*) {
	unsigned d;
	paramon->value(((Fl_Valuator*)o)->value());
	//float wert=-1024;
//(Fl_Valuator*)o)->
 // printf("%li : %g     \r", ((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
  /*
if (strcmp(o->label(),"f1")==0)
{ 
	lo_send(t, "/f1", "f",((Fl_Valuator*)o)->value());
}
if (strcmp(o->label(),"f2")==0)
{ 
	lo_send(t, "/f2", "f",((Fl_Valuator*)o)->value());
}
if (strcmp(o->label(),"f3")==0)
{ 
	lo_send(t, "/f3", "f",((Fl_Valuator*)o)->value());
}*/
/*if (((Fl_Valuator*)o)->argument()!=10)
	lo_send(t, "/Minicomputer", "if",((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
else if (((Fl_Valuator*)o)->value()!=0)
    lo_send(t, "/Minicomputer", "if",10,1000.f);
    else
    lo_send(t, "/Minicomputer", "if",10,0.f);
*/
currentParameter = ((Fl_Valuator*)o)->argument();

switch (currentParameter)
{
	case 256:
	{
		lo_send(t, "/Minicomputer", "iif",currentsound,0,0);
	}
	case 1:
	{
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Valuator*)o)->argument(),((Fl_Valuator*)Knob[1])->value());
		Display[currentsound][0]->value( ((Fl_Valuator* )Knob[currentsound][1])->value() );
		printf("%li : %g     \r", ((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
	break;
	}
	case 2:
	{
		if (((Fl_Light_Button *)o)->value()==0)
		{
			Knob[currentsound][1]->deactivate();
			Knob[currentsound][3]->activate();
			Display[currentsound][1]->activate();
			Display[currentsound][0]->deactivate();
			if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Light_Button*)o)->argument(),0.f);
		}
		else
		{
			Knob[currentsound][3]->deactivate();
			Knob[currentsound][1]->activate();
			Display[currentsound][0]->activate();
			Display[currentsound][1]->deactivate();
			if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Light_Button*)o)->argument(),1.f);
		}
		printf("%li : %i     \r", ((Fl_Light_Button*)o)->argument(),((Fl_Light_Button*)o)->value());
		
	break;	
	}
	case 17:
	{
		if (((Fl_Light_Button *)o)->value()==0)
		{
			Knob[currentsound][16]->deactivate();
			Knob[currentsound][18]->activate();
			Display[currentsound][3]->activate();
			Display[currentsound][2]->deactivate();
			if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Light_Button*)o)->argument(),0.f);
		}
		else
		{
			Knob[currentsound][18]->deactivate();
			Knob[currentsound][16]->activate();
			Display[currentsound][2]->activate();
			Display[currentsound][3]->deactivate();
			if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Light_Button*)o)->argument(),1.f);
		}
		printf("%li : %i     \r", ((Fl_Light_Button*)o)->argument(),((Fl_Light_Button*)o)->value());
		
	break;	
	}
	case 3:
	{
		float f = ((Fl_Positioner*)o)->xvalue() + ((Fl_Positioner*)o)->yvalue();
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Positioner*)o)->argument(),f);
		Display[currentsound][1]->value( f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);
	break;
	}
	case 16:
	{
		/*float f = ((Fl_Positioner*)o)->xvalue() + ((Fl_Positioner*)o)->yvalue();
		lo_send(t, "/Minicomputer", "if",((Fl_Positioner*)o)->argument(),f);
		Display[2]->value( f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);*/
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
		Display[currentsound][2]->value( ((Fl_Valuator*)o)->value() );//Knob[16])->value() );
		printf("%li : %g     \r", ((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
	break;
	}
	case 18:
	{ 
		float f = ((Fl_Positioner*)o)->xvalue() + ((Fl_Positioner*)o)->yvalue();
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Positioner*)o)->argument(),f);
		Display[currentsound][3]->value( f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);
		/*lo_send(t, "/Minicomputer", "if",((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
		Display[3]->value( ((Fl_Valuator* )Knob[18])->value() );
		printf("%li : %g     \r", ((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());*/
	break;
	}
	case 4: // boost button
	case 15:
	{
		if (((Fl_Light_Button *)o)->value()==0)
		{
			if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Light_Button*)o)->argument(),1.f);
		}
		else
		{
			if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Light_Button*)o)->argument(),1000.f);
		}
		printf("%li : %i     \r", ((Fl_Light_Button*)o)->argument(),((Fl_Light_Button*)o)->value());
	break;	
	}
	// the repeat buttons of the mod egs
	case 64:
	case 69:
	case 74:
	case 79:
	case 84:
	case 89:
	{
		if (((Fl_Light_Button *)o)->value()==0)
		{
			if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Light_Button*)o)->argument(),0.f);
		}
		else
		{
			if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Light_Button*)o)->argument(),1.f);
		}
		printf("%li : %i     \r", ((Fl_Light_Button*)o)->argument(),((Fl_Light_Button*)o)->value());
	break;	
	}
	/*
	case 0:
	case 6:
	case 8:
	case 10:
	case 12:
	case 20:
	case 22:
	case 24:
	case 26:
	case 27:
	case 39:
	case 49:
	case 91:
	
	{
		lo_send(t, "/Minicomputer", "if",((Fl_Choice*)o)->argument(),(float)((Fl_Choice*)o)->value());
		printf("%li : %i     \r", ((Fl_Choice*)o)->argument(),((Fl_Choice*)o)->value());
		break;
	}*/
	case 60:
	//case 61:
	case 63:
	case 65:
	//case 66: 
	case 68:
	
	case 70:
	//case 71:
	case 73:
	case 75:
	//case 76: 
	case 78:
	
	case 80:
	//case 81:
	case 83:
	case 85:
	//case 86: 
	case 88:
	
	
	case 102:
	case 105:
	{
		float tr=(((Fl_Valuator*)o)->value());///200.f;//exp(((Fl_Valuator*)o)->value())/200.f;
		tr*= tr*tr/2.f;// tr * tr*20.f;//48000.0f;//trtr*tr/2;
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Valuator*)o)->argument(),tr);
		printf("eg %li : %g     \r", ((Fl_Valuator*)o)->argument(),tr);
		break;
	}	
	
	//************************************ filter cuts *****************************
	case 30:{	float f=((Fl_Positioner*)o)->xvalue()+((Fl_Positioner*)o)->yvalue();
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Positioner*)o)->argument(),f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);
		Display[currentsound][4]->value(f);
		break;
	}
	case 33:{	float f=((Fl_Positioner*)o)->xvalue()+((Fl_Positioner*)o)->yvalue();
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Positioner*)o)->argument(),f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);
		Display[currentsound][5]->value(f);
		break;
	}
	case 40:{	float f=((Fl_Positioner*)o)->xvalue()+((Fl_Positioner*)o)->yvalue();
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Positioner*)o)->argument(),f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);
		Display[currentsound][6]->value(f);
		break;
	}
	case 43:{	float f=((Fl_Positioner*)o)->xvalue()+((Fl_Positioner*)o)->yvalue();
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Positioner*)o)->argument(),f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);
		Display[currentsound][7]->value(f);
		break;
	}
	case 50:{
		float f=0;
		int Argument=0;
		
		//if (!isFine)
		//{
			f=((Fl_Positioner*)o)->xvalue()+((Fl_Positioner*)o)->yvalue();
			Argument=((Fl_Positioner*)o)->argument();
		//}
		//else 
		//{
		//	f=(((Fl_Valuator*)o)->value());
		//	isFine = false;
		//	Argument=((Fl_Valuator*)o)->argument();
		//}*/
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,Argument,f);
		printf("%i : %g     \r", Argument,f);
		//printf(",,do it\n");
						fflush(stdout);
		Display[currentsound][8]->value(f);

		break;
	}
	case 53:{	float f=((Fl_Positioner*)o)->xvalue()+((Fl_Positioner*)o)->yvalue();
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Positioner*)o)->argument(),f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);
		Display[currentsound][9]->value(f);
		break;
	}
	case 90:
	{	float f=((Fl_Positioner*)o)->xvalue()+((Fl_Positioner*)o)->yvalue();
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Positioner*)o)->argument(),f);
		printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);
		Display[currentsound][10]->value(f);
		break;
	}
	default:
	{
		if (transmit)lo_send(t, "/Minicomputer", "iif",currentsound,((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
		printf("%li : %g     \r", ((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
		
		break;
	}
	

}
    
fflush(stdout);
}

/**
 * callback for finetuning the current parameter
 */
static void finetune(Fl_Widget* o, void*)
{

	if (currentParameter<_PARACOUNT)// range check
	{
		switch (currentParameter)
		{
			case 1:
			case 3:
			case 16:
			case 18:
			case 30:
			case 33:
			case 40:
			case 43:
			case 50:
			case 53:
			case 90:
			 break; // do nothin
			default: 
			((Fl_Valuator* )Knob[currentsound][currentParameter])->value(((Fl_Valuator* )o)->value());
			callback(Knob[currentsound][currentParameter],NULL);
			break;
		}
	}
}
/*
static void lfoCallback(Fl_Widget* o, void*)
{
	int Faktor = (int)((Fl_Valuator* )o)->value();
	float Rem = ((Fl_Valuator* )o)->value()-Faktor;
	int Argument = ((Fl_Valuator* )o)->argument();
	((Fl_Positioner* )Knob[currentsound][Argument])->xvalue(Faktor);
	((Fl_Positioner* )Knob[currentsound][Argument])->yvalue(Rem);
	callback(Knob[currentsound][Argument],NULL);
}
*/
static void cutoffCallback(Fl_Widget* o, void*)
{
	int Faktor = ((int)(((Fl_Valuator* )o)->value()/1000)*1000);
	float Rem = ((Fl_Valuator* )o)->value()-Faktor;
	int Argument = ((Fl_Valuator* )o)->argument();
	((Fl_Positioner* )Knob[currentsound][Argument])->xvalue(Faktor);
	((Fl_Positioner* )Knob[currentsound][Argument])->yvalue(Rem);
	callback(Knob[currentsound][Argument],NULL);
}
static void tuneCallback(Fl_Widget* o, void*)
{
	int Faktor = (int)((Fl_Valuator* )o)->value();
	float Rem = ((Fl_Valuator* )o)->value()-Faktor;
	int Argument = ((Fl_Valuator* )o)->argument();
	((Fl_Positioner* )Knob[currentsound][Argument])->xvalue(Faktor);
	((Fl_Positioner* )Knob[currentsound][Argument])->yvalue(Rem);
	callback(Knob[currentsound][Argument],NULL);
}
static void storesound(Fl_Widget* o, void* e)
{
	printf("choice %i\n",((Fl_Input_Choice*)e)->menubutton()->value());
	fflush(stdout);
	Speicher.setChoice(currentsound,((Fl_Input_Choice*)e)->menubutton()->value());
	strcpy(Speicher.sounds[Speicher.getChoice(currentsound)].name,((Fl_Input_Choice*)e)->value());
	printf("input choice %s\n",((Fl_Input_Choice*)e)->value());
	
	((Fl_Input_Choice*)e)->menubutton()->replace(Speicher.getChoice(currentsound),((Fl_Input_Choice*)e)->value());
	
	//Schaltbrett.soundchoice-> add(Speicher.getName(i).c_str());
	int i;
	for (i=0;i<_PARACOUNT;++i)
	{
	if (Knob[currentsound][i] != NULL)
	{
		int j=-1024;
		Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i]=((Fl_Valuator*)Knob[currentsound][i])->value();
		
		switch (i)
{
	
	case 2:
	case 4: // boost button
	case 15:
	case 17:
	{
		if (((Fl_Light_Button *)Knob[currentsound][i])->value()==0)
		{
			Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i]=0;
		}
		else
		{
			Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i]=1;
		}
	break;	
	}
	case 3:
		{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[0][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[0][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	case 18:
			{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[1][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[1][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	//************************************ filter cuts *****************************
	case 30:
			{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[2][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[2][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	case 33:
			{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[3][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[3][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	case 40:
			{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[4][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[4][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	case 43:
			{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[5][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[5][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	case 50:
			{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[6][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[6][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	case 53:
			{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[7][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[7][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	case 90:
	{
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[8][0]=((Fl_Positioner*)Knob[currentsound][i])->xvalue();
		Speicher.sounds[Speicher.getChoice(currentsound)].freq[8][1]=((Fl_Positioner*)Knob[currentsound][i])->yvalue();
	break;
	}
	
	
//	{
//		if (((Fl_Light_Button *)Knob[i])->value()==0)
//		{
//			Speicher.sounds[Speicher.getChoice()].parameter[i]=1;
//		}
//		else
//		{
//			Speicher.sounds[Speicher.getChoice()].parameter[i]=1000;
//		}
//	break;	
//	}
	/*
	case 60:
	//case 61:
	case 63:
	case 65:
	//case 66: 
	case 68:
	
	case 70:
	//case 71:
	case 73:
	case 75:
	//case 76: 
	case 78:
	
	case 80:
	//case 81:
	case 83:
	case 85:
	//case 86: 
	case 88:
	
	
	case 102:
	case 105:
	{
		float tr=(((Fl_Valuator*)o)->value());///200.f;//exp(((Fl_Valuator*)o)->value())/200.f;
		tr *= tr*tr/2.f;// tr * tr*20.f;//48000.0f;//trtr*tr/2;
		Speicher.sounds[Speicher.getChoice()].parameter[i]=tr;
		break;
	}	
	*/
	
	default:
	{
		Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i]=((Fl_Valuator*)Knob[currentsound][i])->value();
		break;
	}
	

	}// end of switch
	}// end of if
	}// end of for

	for (i=0;i<17;++i)
	{
		if (auswahl[currentsound][i] != NULL)
		{
			Speicher.sounds[Speicher.getChoice(currentsound)].choice[i]=auswahl[currentsound][i]->value();
			printf("f:%i:%i ",i,auswahl[currentsound][i]->value());
		}
	}
	printf("\n");
	fflush(stdout);
	Speicher.save();
	
	// ok, now we have saved but we should update the userinterface
  	for (i = 0;i<8;++i)
  	{
  		schoice[i]->clear();
  	} 
  
  	for (i=0;i<512;i++) 
  	{
  		schoice[0]->add(Speicher.getName(0,i).c_str());
	  	schoice[1]->add(Speicher.getName(0,i).c_str());
	  	schoice[2]->add(Speicher.getName(0,i).c_str());
	  	schoice[3]->add(Speicher.getName(0,i).c_str());
	  	schoice[4]->add(Speicher.getName(0,i).c_str());
	  	schoice[5]->add(Speicher.getName(0,i).c_str());
  		schoice[6]->add(Speicher.getName(0,i).c_str());
  		schoice[7]->add(Speicher.getName(0,i).c_str());
  	}
	
}
/**
 *
 */
static void recall(unsigned int preset)
{
	int i,j=-1024;
	printf("choice %i %i\n",currentsound,preset);//((Fl_Input_Choice*)e)->menubutton()->value());
	fflush(stdout);
	Speicher.setChoice(currentsound,preset);
	for(i=0;i<_PARACOUNT;++i)
	{
		if (Knob[currentsound][i] != NULL)
		{
#ifdef _DEBUG		
			printf("i == %i \n",i);
	    		fflush(stdout);
#endif
		switch (i)
		{
	
	
	case 2:
	case 4: // boost button
	case 15:
	case 17:
	{
		if (Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i]==0.0f)
		{
			((Fl_Light_Button*)Knob[currentsound][i])->value(0);
		}
		else
		{
			((Fl_Light_Button*)Knob[currentsound][i])->value(1);
		}
		callback(Knob[currentsound][i],NULL);
		
		break;	
	}
	//{
	//	((Fl_Light_Button*)Knob[i])->value((int)Speicher.sounds[Speicher.getChoice()].parameter[i]);
		
	//	break;	
	//}
	
	case 3:
			{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[0][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[0][1]);
		callback(Knob[currentsound][i],NULL);
		
	break;
	}
	case 18:
			{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[1][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[1][1]);
		callback(Knob[currentsound][i],NULL);
	break;
	}
	//************************************ filter cuts *****************************
	case 30:
			{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[2][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[2][1]);
		callback(Knob[currentsound][i],NULL);
		
	break;
	}
	case 33:
			{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[3][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[3][1]);
		callback(Knob[currentsound][i],NULL);
		
	break;
	}
	case 40:
			{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[4][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[4][1]);
		callback(Knob[currentsound][i],NULL);
	break;
	}
	case 43:
			{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[5][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[5][1]);
		callback(Knob[currentsound][i],NULL);
		
	break;
	}
	case 50:
			{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[6][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[6][1]);
		callback(Knob[currentsound][i],NULL);
						
	break;
	}
	case 53:
	{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[7][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[7][1]);
		callback(Knob[currentsound][i],NULL);
		
	break;
	}
	case 90:
	{
		((Fl_Positioner*)Knob[currentsound][i])->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[8][0]);
		((Fl_Positioner*)Knob[currentsound][i])->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[8][1]);
		callback(Knob[currentsound][i],NULL);
	break;
	}
	
	default:
	{
		((Fl_Valuator*)Knob[currentsound][i])->value(Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i]);
		callback(Knob[currentsound][i],NULL);
		break;
	}
	

}
		}
	}
	printf("so weit so gut");
	for (i=0;i<17;++i)
	{
		if (auswahl[currentsound][i] != NULL)
		{
		auswahl[currentsound][i]->value(Speicher.sounds[Speicher.getChoice(currentsound)].choice[i]);
		choicecallback(auswahl[currentsound][i],NULL);
		printf("l:%i:%i ",i,Speicher.sounds[Speicher.getChoice(currentsound)].choice[i]);
		}
	}
	// send a reset
	if (transmit) lo_send(t, "/Minicomputer", "iif",currentsound,0,0.0f);
	printf("\n");
	fflush(stdout);
}
static void loadsound(Fl_Widget* o, void* e)
{
#ifdef _DEBUG
	printf("choice %i\n",((Fl_Input_Choice*)e)->menubutton()->value());
	fflush(stdout);
#endif	
	Speicher.multis[currentmulti].sound[currentsound]=(unsigned int)((Fl_Input_Choice*)e)->menubutton()->value();
	recall(Speicher.multis[currentmulti].sound[currentsound]);
}
// recall a multitemperal setup
static void loadmulti(Fl_Widget* o, void* e)
{
	currentmulti = (unsigned int)((Fl_Input_Choice*)e)->menubutton()->value();
	//multi[currentmulti][currentsound]=(unsigned int)((Fl_Input_Choice*)e)->menubutton()->value();
	for (int i=0;i<8;++i)
	{
		currentsound = i;
		recall(Speicher.multis[currentmulti].sound[currentsound]);// actual recall
		schoice[i]->value(Speicher.multis[currentmulti].sound[currentsound]);// set gui
	}
#ifdef _DEBUG
	printf("multi choice %i\n",((Fl_Input_Choice*)e)->menubutton()->value());
	fflush(stdout);
#endif
}

static void storemulti(Fl_Widget* o, void* e)
{
	/*printf("choice %i\n",((Fl_Input_Choice*)e)->menubutton()->value());
	fflush(stdout);
	Speicher.setChoice(currentsound,((Fl_Input_Choice*)e)->menubutton()->value());
	*/
	strcpy(Speicher.multis[currentmulti].name,((Fl_Input_Choice*)e)->value());
	//printf("input choice %s\n",((Fl_Input_Choice*)e)->value());
	
	((Fl_Input_Choice*)e)->menubutton()->replace(currentmulti,((Fl_Input_Choice*)e)->value());
	
	//Schaltbrett.soundchoice-> add(Speicher.getName(i).c_str());
	
	// write to disk
	Speicher.saveMulti();
	
}
/*
static void voicecallback(Fl_Widget* o, void* e)
{
	transmit=false;
	currentsound=((unsigned int)((Fl_Valuator*)o)->value());
	recall(multi[currentmulti][currentsound]);
	transmit=true;
}*/
Fl_Menu_Item UserInterface::menu_amod[] = {
 {"midi note", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"velocity", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"pitch bend", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"osc 1 fm out", 0,  0, 0,0 , FL_NORMAL_LABEL , 0, 8, 0},
 {"osc 2 fm out", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"osc 1", 0,  0, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, 0, 8, 0},
 {"osc 2", 0,  0, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, 0, 8, 0},
 {"filter", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 1", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 2", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 3", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 4", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 5", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 6", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"global osc", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"touch", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"mod", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"regler", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Menu_Item UserInterface::menu_fmod[] = {
 {"midi note", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"velocity", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"pitch bend", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"osc 1 fm out", 0,  0, 0,0 , FL_NORMAL_LABEL , 0, 8, 0},
 {"osc 2 fm out", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"osc 1", 0,  0, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, 0, 8, 0},
 {"osc 2", 0,  0, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, 0, 8, 0},
 {"filter", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 1", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 2", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 3", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 4", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 5", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"eg 6", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"global osc", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"touch", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"mod", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"regler", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {0,0,0,0,0,0,0,0,0}
};
Fl_Menu_Item UserInterface::menu_wave[] = {
 {"sine", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"ramp up", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"ramp down", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"tri", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"square", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"bit", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"spike", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"comb", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0}, 
 {"add Saw", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {"add Square", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Menu_Item UserInterface::menu_pitch[] = {
 {"EG1", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"velocity", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};


Fl_Menu_Item UserInterface::menu_pitch1[] = {
 {"EG1", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"velocity", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Menu_Item UserInterface::menu_morph[] = {
 {"EG1", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"velocity", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_SteinerKnob::Fl_SteinerKnob(int x, int y, int w, int h, const char *label)
: Fl_Dial(x, y, w, h, label) {
	
}
int Fl_SteinerKnob::handle(int event) {
	if (event==FL_PUSH)
	{
		if (Fl::event_button2())
		{
			altx=Fl::event_x();
			
			this->set_changed();
			//return 0;
		}
		else if (Fl::event_button3())
		{
			this->step(0.001);
		}
		else this->step(0);
	}
	if (event ==FL_DRAG)
	{
	if (Fl::event_button2())
		{
			if (Fl::event_x()<altx )
			 this->value(this->value()+1);
			else   this->value(this->value()-1);
			altx=Fl::event_x();
			this->set_changed();
			return 0;
		}
		if (Fl::event_button3())
		{
			if (Fl::event_x()<altx )
			 this->value(this->value()+0.01);
			else   this->value(this->value()-0.01);
			altx=Fl::event_x();
			this->set_changed();
			return 0;
		}
	}
	  return Fl_Dial::handle(event);
}

// ---------------------------------------------------------------
// 			screen initialization
// ---------------------------------------------------------------

Fenster* UserInterface::make_window() {
 // Fl_Double_Window* w;
 // {
    currentsound=0;currentmulti=0;
    transmit=true;
  	Fenster* o = new Fenster(995, 515);
   // w = o;
    o->color((Fl_Color)246);
    o->user_data((void*)(this));
	for (int i=0;i<17;i++) {
		printf("%i \n",i);
		auswahl[currentsound][i]=NULL;
     	fflush(stdout);
    }
    for (int i=0;i<_PARACOUNT;i++) {
		Knob[currentsound][i]=NULL;
    }
    // tabs beginning ------------------------------------------------------------
    { Fl_Tabs* o = new Fl_Tabs(0,0,995, 515);
    	 o->callback((Fl_Callback*)tabcallback);
    for (int i=0;i<8;i++)
    {
    	{ 
    	ostringstream oss;
	oss<<"sound "<<(i+1);
	tablabel[i]=oss.str();
	Fl_Group* o = new Fl_Group(1, 10, 995, 515, tablabel[i].c_str());
    	 o->color((Fl_Color)246);
    	 o->labelsize(8);
    	 //o->argument(2);
    	// int xtab=1;
      //o->labelcolor(FL_BACKGROUND2_COLOR); 
     // o->callback((Fl_Callback*)tabcallback,&xtab);
      
    o->box(FL_BORDER_FRAME);
     { Fl_Group* o = new Fl_Group(5, 17, 300, 212);
      o->box(FL_ROUNDED_FRAME);
      o->color(FL_BACKGROUND2_COLOR);
      {
      Fl_Group* d = new Fl_Group(225, 225, 30, 22,"oscillator 1");
      d->labelsize(8);
      d->labelcolor(FL_BACKGROUND2_COLOR);
	d->end();
      }
      { Fl_SteinerKnob* o= new Fl_SteinerKnob(21, 20, 34, 34, "frequency");
        o->labelsize(8);
        o->maximum(1000); 
        o->argument(1);
        o->callback((Fl_Callback*)callback);
        Knob[i][1] = o;
      }
      { Fl_Value_Input* o = new Fl_Value_Input(16, 66, 46, 15);
        o->box(FL_ROUNDED_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->maximum(10000);
	o->step(0.01);
        o->argument(1);
        o->callback((Fl_Callback*)callback);
        Display[i][0]=o;
        
      }
      { Fl_Light_Button* o = new Fl_Light_Button(20, 92, 66, 19, "fix frequency");
        o->box(FL_BORDER_BOX);
        o->selection_color((Fl_Color)89);
        o->labelsize(8);
        o->argument(2);
        o->callback((Fl_Callback*)callback);
        Knob[i][2] = o;
      }
      { Fl_Dial* o = new Fl_Dial(79, 179, 25, 25, "fm output  vol");
        o->labelsize(8);
        o->argument(13);
        //o->maximum(1000);
        o->callback((Fl_Callback*)callback);
         Knob[i][o->argument()] = o;
      }
      { Fl_Light_Button* o = new Fl_Light_Button(80, 27, 40, 15, "boost");
        o->box(FL_BORDER_BOX);
        o->selection_color((Fl_Color)89);
        o->labelsize(8);
        o->argument(4);
        o->callback((Fl_Callback*)callback);
        Knob[i][o->argument()] = o;
      }
      /*{ Fl_Dial* o = new Fl_SteinerKnob(20, 121, 34, 34, "tune");
        o->labelsize(8);
        o->minimum(0.5);
        o->maximum(16);
        o->argument(3);
        o->callback((Fl_Callback*)callback);
        Knob[3] = o;
      }*/
      
      { Fl_Positioner* o = new Fl_Positioner(15,121,40,80,"tune");
    	o->xbounds(0,16);
    	o->ybounds(1,0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1);
    	o->labelsize(8);
    	o->argument(3); o->selection_color(0);
    	o->callback((Fl_Callback*)callback);
    	Knob[i][3]=o;
    	 
      }
      { Fl_Value_Input* o = new Fl_Value_Input(62, 130, 46, 15);
        o->box(FL_ROUNDED_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->maximum(10000);
    	o->argument(3);
	o->step(0.01);
    	o->callback((Fl_Callback*)tuneCallback);
        Display[i][1]=o;
      }
      { Fl_Dial* o = new Fl_Dial(260, 97, 25, 25, "amount");
        o->labelsize(8);
        o->argument(9); 
        o->minimum(-1);
        o->maximum(1);
        o->callback((Fl_Callback*)callback);
        Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(134, 102, 120, 15, "amp modulator 1");
        o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->argument(2);
        o->callback((Fl_Callback*)choicecallback);
         o->menu(menu_amod);
         auswahl[i][o->argument()]=o;
      }
      { Fl_Dial* o = new Fl_Dial(260, 133, 25, 25, "amount");
        o->labelsize(8);
        o->argument(11);  
        o->minimum(-1);
        o->maximum(1);
        o->callback((Fl_Callback*)callback);
        Knob[i][o->argument()] = o;
        
      }
      { Fl_Choice* o = new Fl_Choice(134, 138, 120, 15, "amp modulator 2");
        o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->argument(3);
        o->callback((Fl_Callback*)choicecallback);
        o->menu(menu_amod);
        auswahl[i][o->argument()]=o;
       
      }
      { Fl_Dial* o = new Fl_Dial(247, 23, 25, 25, "amount");
        o->labelsize(8);
        o->argument(5);  
        o->minimum(-1000);
        o->maximum(1000);
        o->callback((Fl_Callback*)callback);
        Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(122, 28, 120, 15, "freq modulator 1");
        o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->argument(0);
        o->callback((Fl_Callback*)choicecallback);
        o->menu(menu_fmod);
        auswahl[i][o->argument()]=o;
      }
      { Fl_Dial* o = new Fl_Dial(248, 59, 25, 25, "amount");
        o->labelsize(8);
        o->argument(7); 
         o->minimum(-1000);
        o->maximum(1000);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(122, 64, 120, 15, "freq modulator 2");
        o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->argument(1);
        o->callback((Fl_Callback*)choicecallback);
        o->menu(menu_fmod);
        auswahl[i][o->argument()]=o;
      }
      
      
      { Fl_Choice* j = new Fl_Choice(120, 184, 120, 15, "waveform");
        j->box(FL_BORDER_BOX);
        j->down_box(FL_BORDER_BOX);
        j->labelsize(8);
        j->textsize(8);
        j->align(FL_ALIGN_TOP_LEFT);
        j->argument(4);
        auswahl[i][j->argument()] = j;
        j->callback((Fl_Callback*)choicecallback);
        j->menu(menu_wave);
      }
      o->end();
    }
   
    //}
   { Fl_Group* o = new Fl_Group(5, 238, 300, 212);
      o->box(FL_ROUNDED_FRAME);
      o->color(FL_BACKGROUND2_COLOR);
      {Fl_Group* d = new Fl_Group(5, 446, 30, 22, "oscillator 2");
      	d->labelsize(8);
      	d->labelcolor(FL_BACKGROUND2_COLOR);
	d->end();
      }
      { Fl_Dial* o = new Fl_SteinerKnob(21, 244, 34, 34, "frequency");
        o->labelsize(8); 
        o->argument(16);
         o->maximum(1000); 
        o->callback((Fl_Callback*)callback);
        Knob[i][16] = o;
      }
      { Fl_Value_Input* o = new Fl_Value_Input(16, 290, 46, 15);
        o->box(FL_ROUNDED_BOX);
        o->labelsize(8);
        o->textsize(8); 
        o->maximum(10000);
	o->step(0.01);
        o->argument(16);
        o->callback((Fl_Callback*)callback);
        Display[i][2]=o;
      }
      { Fl_Light_Button* o = new Fl_Light_Button(20, 316, 66, 19, "fix frequency");
        o->box(FL_BORDER_BOX);
        o->selection_color((Fl_Color)89);
        o->labelsize(8); 
        o->argument(17);
        o->callback((Fl_Callback*)callback);Knob[1][o->argument()] = o;
      }
      { Fl_Dial* o = new Fl_Dial(79, 403, 25, 25, "fm output  vol");
        o->labelsize(8); 
        o->argument(28);
      //  o->maximum(1000);
        o->callback((Fl_Callback*)callback);
        Knob[i][28] = o;
      }
      { Fl_Light_Button* o = new Fl_Light_Button(80, 251, 40, 15, "boost");
        o->box(FL_BORDER_BOX);
        o->selection_color((Fl_Color)89);
        o->labelsize(8);
         o->argument(15);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Positioner* o = new Fl_Positioner(15,345,40,80,"tune");
    	o->xbounds(0,16);
    	o->ybounds(1,0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1); o->selection_color(0);
    	o->labelsize(8);
    	o->argument(18);
    	o->callback((Fl_Callback*)callback);
    	
    	/*Fl_Dial* o = new Fl_SteinerKnob(20, 345, 34, 34, "tune");
        o->labelsize(8);
        o->minimum(0.5);
        o->maximum(16);   
        o->argument(18);
        o->callback((Fl_Callback*)callback);*/
        Knob[i][18] = o;
      }
      { Fl_Value_Input* o = new Fl_Value_Input(62, 354, 46, 15);
        o->box(FL_ROUNDED_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->maximum(10000);   
    	o->argument(18);
	o->step(0.01);
    	o->callback((Fl_Callback*)tuneCallback);
        Display[i][3]=o;
      }
      { Fl_Dial* o = new Fl_Dial(260, 321, 25, 25, "amount");
        o->labelsize(8);
         o->argument(23);  o->minimum(-1);
        o->maximum(1);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(134, 326, 120, 15, "amp modulator 1");
        o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
         o->argument(8);
        o->callback((Fl_Callback*)choicecallback);
        o->menu(menu_amod);
         auswahl[i][o->argument()]=o;
      }
      { Fl_Dial* o = new Fl_Dial(260, 357, 25, 25, "amount");
        o->labelsize(8);
         o->argument(25);  o->minimum(-1);
        o->maximum(1);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(134, 362, 120, 15, "amp modulator 2");
          o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
         o->argument(9);
        o->callback((Fl_Callback*)choicecallback);
        o->menu(menu_amod);
        auswahl[i][o->argument()]=o;
      }
      { Fl_Dial* o = new Fl_Dial(247, 247, 25, 25, "amount");
        o->labelsize(8);
         o->argument(19);  o->minimum(-1000);
        o->maximum(1000);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(122, 252, 120, 15, "freq modulator 1");
           o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
         o->argument(6);
        o->callback((Fl_Callback*)choicecallback);
        o->menu(menu_fmod);
        auswahl[i][o->argument()]=o;
      }
      { Fl_Dial* o = new Fl_Dial(248, 283, 25, 25, "amount");
        o->labelsize(8);
         o->argument(21);  o->minimum(-1000);
        o->maximum(1000);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(122, 288, 120, 15, "freq modulator 2");
          o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
         o->argument(7);
        o->callback((Fl_Callback*)choicecallback);
        o->menu(menu_fmod);
        auswahl[i][o->argument()]=o;
      }
      { Fl_Choice* o = new Fl_Choice(120, 408, 120, 15, "waveform");
        o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
         o->argument(5);
        o->callback((Fl_Callback*)choicecallback);
        o->menu(menu_wave);auswahl[i][o->argument()]=o;
      }
      o->end();
    }
   
  { Fl_Group* o = new Fl_Group(312, 17, 277, 433);
      o->box(FL_ROUNDED_FRAME);
      o->color(FL_BACKGROUND2_COLOR);
  	{ Fl_Group* d = new Fl_Group(312, 446, 277, 435, "filters");
      	d->labelsize(8);
      	d->labelcolor(FL_BACKGROUND2_COLOR);
	d->end();
	}
      { Fl_Group* o = new Fl_Group(330, 28, 239, 92, "filter 1");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        {Fl_Positioner* o = new Fl_Positioner(340,35,40,55,"cut");
    	o->xbounds(0,10000);
    	o->ybounds(1000,0); o->selection_color(0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1000);
    	o->labelsize(8);
    	o->argument(30);
    	o->callback((Fl_Callback*)callback);
    	Knob[i][o->argument()] = o;
    	
    	/* Fl_Dial* o = f1cut1 = new Fl_SteinerKnob(344, 51, 34, 34, "cut");
          o->labelsize(8);
          o->argument(30);
			o->maximum(10000);
		o->value(50);
		o->callback((Fl_Callback*)callback);
		*/
        }
        { Fl_Dial* o = f1q1 = new Fl_Dial(382, 33, 25, 25, "q");
          o->labelsize(8);
          o->argument(31);
          o->minimum(0.9);
          o->value(0.5);
          o->maximum(0.01);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = f1vol1 = new Fl_Dial(396, 72, 25, 25, "vol");
          o->labelsize(8);
           o->argument(32);
		o->callback((Fl_Callback*)callback);o->minimum(-1);
          o->value(0);
          o->maximum(1);Knob[i][o->argument()] = o;
        }
        
        
        { Fl_Positioner* o = new Fl_Positioner(475,35,40,55,"cut");
    	o->xbounds(0,10000);
    	o->ybounds(1000,0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1000); o->selection_color(0);
    	o->labelsize(8);
    	o->callback((Fl_Callback*)callback);
       /* Fl_Dial* o = f1cut2 = new Fl_SteinerKnob(481, 50, 34, 34, "cut");
          o->labelsize(8); 
          o->value(50);
           o->maximum(10000);
		o->callback((Fl_Callback*)callback);*/
          o->argument(33);
	  Knob[i][o->argument()] = o;
		
        }
        { Fl_Dial* o = f1q2 = new Fl_Dial(517, 32, 25, 25, "q");
          o->labelsize(8); 
          
          o->argument(34);
          o->minimum(0.9);
          o->value(0.5);
          o->maximum(0.01);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = f1vol2 = new Fl_Dial(536, 71, 25, 25, "vol");
          o->labelsize(8); 
          o->labelsize(8);
          o->minimum(-1);
          o->value(0);
          o->maximum(1);
           o->argument(35);
	   o->callback((Fl_Callback*)callback);
	   Knob[i][o->argument()] = o;
        }
        { Fl_Value_Input* o = new Fl_Value_Input(342, 100, 38, 15);
          o->box(FL_ROUNDED_BOX);
          o->labelsize(8);
          o->textsize(8);
	  o->maximum(10000);
	  o->step(0.01);
          o->argument(30);
	   o->callback((Fl_Callback*)cutoffCallback);
          Display[i][4]=o;
        }
        { Fl_Value_Input* o = new Fl_Value_Input(479, 100, 38, 15);
          o->box(FL_ROUNDED_BOX);
          o->labelsize(8);
          o->textsize(8);
	  o->maximum(10000);
	  o->step(0.01);
          o->argument(33);
	   o->callback((Fl_Callback*)cutoffCallback);
           Display[i][5]=o;
        }
        { Fl_Button* o = new Fl_Button(426, 35, 45, 15, "copy ->");
          o->box(FL_BORDER_BOX);
          o->labelsize(8);
        }
        { Fl_Button* o = new Fl_Button(426, 59, 45, 15, "<- copy");
          o->box(FL_BORDER_BOX);
          o->labelsize(8);
        }
        o->end();
      }
      { Fl_Dial* o = new Fl_Dial(418, 360, 60, 57, "morph");
        o->type(1);
        o->labelsize(8);
        o->argument(56);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Dial* o = new Fl_Dial(326, 392, 25, 25, "amount");
        o->labelsize(8);  o->minimum(-1);
        o->maximum(1);
        o->argument(38);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(325, 366, 85, 15, "morph mod 1");
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->menu(menu_amod);
        o->argument(10);
        o->callback((Fl_Callback*)choicecallback);auswahl[i][o->argument()]=o;
      }
      { Fl_Dial* o = new Fl_Dial(551, 392, 25, 25, "amount");
        o->labelsize(8);
        o->argument(48);  o->minimum(-1);
        o->maximum(1);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(492, 366, 85, 15, "morph mod 2");
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->menu(menu_amod);
        o->argument(11);
        o->callback((Fl_Callback*)choicecallback);auswahl[i][o->argument()]=o;
      }
      { Fl_Group* o = new Fl_Group(330, 132, 239, 92, "filter 2");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        { Fl_Positioner* o = new Fl_Positioner(340,139,40,55,"cut");
    	o->xbounds(0,10000);
    	o->ybounds(1000,0); o->selection_color(0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1000);
    	o->labelsize(8);
    	/*Fl_Dial* o = f1cut1 = new Fl_SteinerKnob(344, 155, 34, 34, "cut");
          o->labelsize(8);
			o->labelsize(8);
           o->value(50);
          o->maximum(10000);*/
          o->argument(40);Knob[i][o->argument()] = o;
		o->callback((Fl_Callback*)callback);
        }
        { Fl_Dial* o = f1q1 = new Fl_Dial(382, 137, 25, 25, "q");
          o->labelsize(8);
           o->argument(41);
          o->minimum(0.9);
          o->maximum(0.001);
          o->value(0.5);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = f1vol1 = new Fl_Dial(396, 176, 25, 25, "vol");
          o->labelsize(8);
           o->argument(42);o->maximum(2);
		o->callback((Fl_Callback*)callback);o->minimum(-2);
          o->value(0);
          o->maximum(2);Knob[i][o->argument()] = o;
        }
        { Fl_Positioner* o = new Fl_Positioner(475,139,40,55,"cut");
    	o->xbounds(0,10000);
    	o->ybounds(1000,0); o->selection_color(0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1000);
    	o->labelsize(8);
    	
    	/*Fl_Dial* o = f1cut2 = new Fl_SteinerKnob(481, 154, 34, 34, "cut");
          o->labelsize(8);
           o->labelsize(8);
           o->value(50);
          o->maximum(10000);*/
          o->argument(43);Knob[i][o->argument()] = o;
		o->callback((Fl_Callback*)callback);
        }
        { Fl_Dial* o = f1q2 = new Fl_Dial(517, 136, 25, 25, "q");
          o->labelsize(8);
           o->labelsize(8);
          o->argument(44);
          o->minimum(0.9);
          o->value(0.5);
          o->maximum(0.001);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = f1vol2 = new Fl_Dial(536, 175, 25, 25, "vol");
          o->labelsize(8);
           o->labelsize(8);
          o->argument(45);o->maximum(2);
		o->callback((Fl_Callback*)callback);o->minimum(-2);
          o->value(0);
          o->maximum(2);
          Knob[i][o->argument()] = o;
        }
        { Fl_Value_Input* o = new Fl_Value_Input(342, 204, 38, 15);
          o->box(FL_ROUNDED_BOX);
          o->labelsize(8);
	  o->maximum(10000);
          o->argument(40);
	  o->step(0.01);
	   o->callback((Fl_Callback*)cutoffCallback);
          o->textsize(8);Display[i][6]=o;
        }
        { Fl_Value_Input* o = new Fl_Value_Input(479, 204, 38, 15);
          o->box(FL_ROUNDED_BOX);
          o->labelsize(8);
          o->argument(43);
	  o->maximum(10000);
	  o->step(0.01);
	   o->callback((Fl_Callback*)cutoffCallback);
          o->textsize(8);Display[i][7]=o;
        }
        { Fl_Button* o = new Fl_Button(426, 139, 45, 15, "copy ->");
          o->box(FL_BORDER_BOX);
          o->labelsize(8);
        }
        { Fl_Button* o = new Fl_Button(426, 163, 45, 15, "<- copy");
          o->box(FL_BORDER_BOX);
          o->labelsize(8);
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(330, 238, 239, 92, "filter 3");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        {Fl_Positioner* o = new Fl_Positioner(340,245,40,55,"cut");
    	o->xbounds(0,10000);
    	o->ybounds(1000,0); o->selection_color(0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1000);
    	o->labelsize(8);
    	/* Fl_Dial* o = f1cut1 = new Fl_SteinerKnob(344, 261, 34, 34, "cut");
          o->labelsize(8); 
          o->value(50);
          o->maximum(10000);*/
          o->argument(50);
		o->callback((Fl_Callback*)callback);
		Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = f1q1 = new Fl_Dial(382, 243, 25, 25, "q");
          o->labelsize(8); 
           o->argument(51);
          o->minimum(0.9);
          o->maximum(0.001);
          o->value(0.5);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = f1vol1 = new Fl_Dial(396, 282, 25, 25, "vol");
          o->labelsize(8);
           o->argument(52);
           o->maximum(2);o->minimum(-2);
          o->value(0);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Positioner* o = new Fl_Positioner(475,245,40,55,"cut");
    	o->xbounds(0,10000);
    	o->ybounds(1000,0); o->selection_color(0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1000);
    	o->labelsize(8);
    	/*Fl_Dial* o = f1cut2 = new Fl_SteinerKnob(481, 260, 34, 34, "cut");
          o->labelsize(8);
          o->value(50);
          o->maximum(10000);*/
           o->argument(53);
           Knob[i][o->argument()] = o;
		o->callback((Fl_Callback*)callback);
        }
        { Fl_Dial* o = f1q2 = new Fl_Dial(517, 242, 25, 25, "q");
          o->labelsize(8);
          o->argument(54);
          o->minimum(0.9);
          o->value(0.5);
          o->maximum(0.001);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = f1vol2 = new Fl_Dial(536, 281, 25, 25, "vol");
          o->labelsize(8); 
          o->labelsize(8);
          o->argument(55);o->maximum(2);o->minimum(-2);
          o->value(0);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Value_Input* o = new Fl_Value_Input(342, 310, 38, 15);
          o->box(FL_ROUNDED_BOX);
          o->labelsize(8);
	  o->maximum(10000);
	  o->step(0.01);
          o->argument(50);
	   o->callback((Fl_Callback*)cutoffCallback);
          o->textsize(8);Display[i][8]=o;
        }
        { Fl_Value_Input* o = new Fl_Value_Input(479, 310, 38, 15);
          o->box(FL_ROUNDED_BOX);
          o->labelsize(8);
	  o->maximum(10000);
          o->argument(53);
	  o->step(0.01);
	   o->callback((Fl_Callback*)cutoffCallback);
          o->textsize(8);Display[i][9]=o;
        }
        { Fl_Button* o = new Fl_Button(426, 245, 45, 15, "copy ->");
          o->box(FL_BORDER_BOX);
          o->labelsize(8);
        }
        { Fl_Button* o = new Fl_Button(426, 269, 45, 15, "<- copy");
          o->box(FL_BORDER_BOX);
          o->labelsize(8);
        }
        o->end();
      } 
      { Fl_Button* o = new Fl_Button(486, 430, 50, 15, "clear filter");
        o->tooltip("reset the filter");
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->labelcolor((Fl_Color)186);
        o->argument(0);
        o->callback((Fl_Callback*)callback);
      }
      o->end();
    }
    { Fl_Group* o = new Fl_Group(595, 17, 225, 433);
      o->box(FL_ROUNDED_FRAME);
      o->color(FL_BACKGROUND2_COLOR);
    { Fl_Group* d = new Fl_Group(595, 446, 225, 432, "modulators");
      d->labelsize(8);
      d->labelcolor(FL_BACKGROUND2_COLOR);
	d->end();
      }
      { Fl_Group* o = new Fl_Group(608, 31, 200, 45, "EG 1");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        { Fl_Dial* o = new Fl_Dial(618, 37, 25, 25, "A");
          o->labelsize(8); 
          o->argument(60);  
          o->minimum(0.5);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(648, 37, 25, 25, "D");
          o->labelsize(8);
          o->argument(61);
          o->minimum(0.00025);
          o->maximum(0.00001);
          
		  o->callback((Fl_Callback*)callback);
		  Knob[i][o->argument()] = o;
        }
        
        { Fl_Dial* o = new Fl_Dial(678, 37, 25, 25, "S");
          o->labelsize(8);
          o->argument(62);
         // o->minimum(0);
         // o->maximum(0.001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(708, 37, 25, 25, "R");
          o->labelsize(8);
          o->argument(63);
          o->minimum(0.25);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Light_Button* o = new Fl_Light_Button(744, 42, 55, 15, "repeat");
          o->box(FL_BORDER_BOX);
          o->selection_color((Fl_Color)89);
          o->labelsize(8);
          o->argument(64);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(608, 90, 200, 45, "EG 2");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        { Fl_Dial* o = new Fl_Dial(618, 96, 25, 25, "A");
          o->labelsize(8);o->argument(65); o->minimum(0.5);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(648, 96, 25, 25, "D");
          o->labelsize(8);o->argument(66); o->minimum(0.001);
          o->maximum(0.00001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(678, 96, 25, 25, "S");
          o->labelsize(8);o->argument(67);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(708, 96, 25, 25, "R");
          o->labelsize(8);o->argument(68); o->minimum(0.25);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Light_Button* o = new Fl_Light_Button(744, 101, 55, 15, "repeat");
          o->box(FL_BORDER_BOX);
          o->selection_color((Fl_Color)89);
          o->labelsize(8);o->argument(69);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(608, 147, 200, 45, "EG 3");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        { Fl_Dial* o = new Fl_Dial(618, 153, 25, 25, "A");
          o->labelsize(8);o->argument(70); o->minimum(0.5);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(648, 153, 25, 25, "D");
          o->labelsize(8);o->argument(71); o->minimum(0.00025);
          o->maximum(0.00001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(678, 153, 25, 25, "S");
          o->labelsize(8);o->argument(72);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(708, 153, 25, 25, "R");
          o->labelsize(8);o->argument(73); o->minimum(0.25);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Light_Button* o = new Fl_Light_Button(744, 158, 55, 15, "repeat");
          o->box(FL_BORDER_BOX);
          o->selection_color((Fl_Color)89);
          o->labelsize(8);o->argument(74);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(608, 204, 200, 45, "EG 4");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        { Fl_Dial* o = new Fl_Dial(618, 210, 25, 25, "A");
          o->labelsize(8);o->argument(75); o->minimum(0.5);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(648, 210, 25, 25, "D");
          o->labelsize(8);o->argument(76); o->minimum(0.001);
          o->maximum(0.00001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(678, 210, 25, 25, "S");
          o->labelsize(8);o->argument(77);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(708, 210, 25, 25, "R");
          o->labelsize(8);o->argument(78); o->minimum(0.25);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Light_Button* o = new Fl_Light_Button(744, 215, 55, 15, "repeat");
          o->box(FL_BORDER_BOX);
          o->selection_color((Fl_Color)89);
          o->labelsize(8);o->argument(79);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(608, 263, 200, 45, "EG 5");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        { Fl_Dial* o = new Fl_Dial(618, 269, 25, 25, "A");
          o->labelsize(8);o->argument(80); o->minimum(0.5);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(648, 269, 25, 25, "D");
          o->labelsize(8);o->argument(81); o->minimum(0.001);
          o->maximum(0.00001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(678, 269, 25, 25, "S");
          o->labelsize(8);o->argument(82);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(708, 269, 25, 25, "R");
          o->labelsize(8);o->argument(83); o->minimum(0.25);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Light_Button* o = new Fl_Light_Button(744, 274, 55, 15, "repeat");
          o->box(FL_BORDER_BOX);
          o->selection_color((Fl_Color)89);
          o->labelsize(8);o->argument(84);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(608, 324, 200, 45, "EG 6");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        { Fl_Dial* o = new Fl_Dial(618, 330, 25, 25, "A");
          o->labelsize(8);o->argument(85); 
          o->minimum(0.5);
          o->maximum(0.0001);
          Knob[i][o->argument()] = o;
          
		  o->callback((Fl_Callback*)callback);
        }
        { Fl_Dial* o = new Fl_Dial(648, 330, 25, 25, "D");
          o->labelsize(8);o->argument(86); o->minimum(0.00025);
          o->maximum(0.00001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(678, 330, 25, 25, "S");
          o->labelsize(8);o->argument(87);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Dial* o = new Fl_Dial(708, 330, 25, 25, "R");
          o->labelsize(8);o->argument(88); o->minimum(0.25);
          o->maximum(0.0001);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        { Fl_Light_Button* o = new Fl_Light_Button(744, 335, 55, 15, "repeat");
          o->box(FL_BORDER_BOX);
          o->selection_color((Fl_Color)89);
          o->labelsize(8);o->argument(89);
		  o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(608, 380, 200, 54, "mod osc");
        o->box(FL_ROUNDED_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        
        {  Fl_Positioner* o = new Fl_Positioner(620,384,50,40,"tune");
    	o->xbounds(0,128);
    	o->ybounds(0.99,0);
    	o->box(FL_BORDER_BOX);
    	o->xstep(1);
    	o->labelsize(8);
    	o->argument(90);
           o->selection_color(0);
    	o->callback((Fl_Callback*)callback);
    	 Knob[i][o->argument()] = o;
      /*Fl_Dial* o = new Fl_SteinerKnob(627, 392, 34, 34, "frequency");
          o->labelsize(8);o->argument(90);
		  o->callback((Fl_Callback*)callback);
		  o->maximum(500); */
        }
        { Fl_Choice* o = new Fl_Choice(680, 397, 120, 15, "waveform");
          o->box(FL_BORDER_BOX);
          o->down_box(FL_BORDER_BOX);
          o->labelsize(8);
          o->textsize(8);
          o->align(FL_ALIGN_TOP_LEFT);
           o->menu(menu_wave);
           o->argument(12);
		  o->callback((Fl_Callback*)choicecallback);
		  auswahl[i][o->argument()]=o;
        } 
        { Fl_Value_Input* o = new Fl_Value_Input(690, 415, 38, 15);
          o->box(FL_ROUNDED_BOX);
          o->labelsize(8);
	  o->maximum(10000);
	  o->step(0.001);
          o->textsize(8);
    	o->callback((Fl_Callback*)tuneCallback);
    	o->argument(90);
	  Display[i][10]=o;
        }
        o->end();
      }
      o->end();
    }
    
   
    { Fl_Group* d = new Fl_Group(5, 461, 680, 45, "memory");
      d->box(FL_ROUNDED_FRAME);
      d->color(FL_BACKGROUND2_COLOR);
      d->labelsize(8);
      d->labelcolor(FL_BACKGROUND2_COLOR);
      d->begin();
     /* { Fl_Button* o = new Fl_Button(191, 473, 50, 19, "create bank");
        o->tooltip("create a new bank after current one");
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
      }
      { Fl_Button* o = new Fl_Button(26, 476, 53, 14, "delete bank");
        o->tooltip("delete a whole bank of sounds!");
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->labelcolor((Fl_Color)1);
      }
      { Fl_Button* o = new Fl_Button(732, 475, 59, 14, "delete sound");
        o->tooltip("delete current sound");
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->labelcolor((Fl_Color)1);
      }*/
      { Fl_Input_Choice* o = new Fl_Input_Choice(274, 476, 206, 14, "sound");
        o->box(FL_BORDER_FRAME);
        o->down_box(FL_BORDER_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->selection_color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        o->textsize(8);
        o->menubutton()->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        soundchoice[i] = o;
        schoice[i] = o;
	d->add(o);
      }
      { Fl_Button* o = new Fl_Button(606, 473, 55, 19, "store sound");
        o->tooltip("store this sound after current entry");
        o->box(FL_BORDER_BOX);
        o->labelsize(8);o->labelcolor((Fl_Color)1);
        o->callback((Fl_Callback*)storesound,soundchoice[i]);
      }
      { Fl_Button* o = new Fl_Button(526, 473, 70, 19, "load sound");
        o->tooltip("overwrite the last chosen sound");
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->labelcolor((Fl_Color)186);
         o->callback((Fl_Callback*)loadsound,soundchoice[i]);
      }
      /*{ Fl_Input_Choice* o = new Fl_Input_Choice(83, 476, 105, 14, "bank");
        o->box(FL_BORDER_FRAME);
        o->down_box(FL_BORDER_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->selection_color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
      }*/
      d->end();
    }
    { Fl_Group* o = new Fl_Group(825, 17, 160, 433);
      o->box(FL_ROUNDED_FRAME);
      o->color(FL_BACKGROUND2_COLOR);
    { Fl_Group* d = new Fl_Group(825, 446, 160, 435, "amp");
      d->labelsize(8);
      d->labelcolor(FL_BACKGROUND2_COLOR);
	d->end();
    }
    { Fl_Dial* o = new Fl_Dial(844, 103, 25, 25, "A");
        o->labelsize(8);o->argument(102); 
        o->minimum(0.5);
          o->maximum(0.0001);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Dial* o = new Fl_Dial(874, 103, 25, 25, "D");
        o->labelsize(8);o->argument(103); 
        o->minimum(0.001);
          o->maximum(0.00001);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Dial* o = new Fl_Dial(904, 103, 25, 25, "S");
        o->labelsize(8);o->argument(104);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Dial* o = new Fl_Dial(934, 103, 25, 25, "R");
        o->labelsize(8);o->argument(105); o->minimum(0.25);
          o->maximum(0.0001);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Dial* o = new Fl_Dial(950, 58, 25, 25, "amount");
        o->labelsize(8);
        o->argument(100);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      { Fl_Choice* o = new Fl_Choice(844, 35, 120, 15, "amp modulator");
        o->box(FL_BORDER_BOX);
        o->down_box(FL_BORDER_BOX);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->menu(menu_amod);
        o->argument(13);
		o->callback((Fl_Callback*)choicecallback);auswahl[i][o->argument()]=o;
      }
      /*
      { Fl_Counter* o = new Fl_Counter(844, 151, 115, 14, "sound");
        o->type(1);
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->minimum(0);
        o->maximum(7);
        o->step(1);
        o->value(0);
        o->textsize(8);
      //  o->callback((Fl_Callback*)voicecallback,soundchoice[0]);
      }
      { Fl_Counter* o = new Fl_Counter(844, 181, 115, 14, "midichannel");
        o->type(1);
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->minimum(1);
        o->maximum(16);
        o->step(1);
        o->value(1);
        o->textsize(8);
      }*/
      { Fl_Dial* o = new Fl_Dial(889, 220, 25, 25, "vol");
        o->labelsize(8); 
        o->argument(101);
        o->minimum(0);
        o->maximum(2);
        o->value(1);
		o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
      o->end();
    }
        { Fl_Dial* o = new Fl_Dial(295, 191, 25, 25, "osc1  vol");
      o->labelsize(8);
      o->align(FL_ALIGN_TOP);
        o->argument(14);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
      }
       { Fl_Dial* o = new Fl_Dial(295, 262, 25, 25, "osc2  vol");
      o->labelsize(8);
       o->argument(29);
        o->callback((Fl_Callback*)callback);Knob[i][o->argument()] = o;
    }
    o->end(); 
    tab[i]=o;
    } // ==================================== end tab 2
    } // end of for
    
    
      
    o->end();
    tabs = o;
    }
// ---------------------------------------------------------------- end of tabs
    /*{ Fl_Chart * o = new Fl_Chart(600, 300, 70, 70, "eg");
    	o->bounds(0.0,1.0);
    	o->type(Fl::LINE_CHART);
    	o->insert(0, 0.5, NULL, 0);
    	o->insert(1, 0.5, NULL, 0);
    	o->insert(2, 1, NULL, 0);
    	o->insert(3, 0.5, NULL, 0);
    	EG[0]=o;
    	
    }*/



// ----------------------------------------- Multi
      { Fl_Input_Choice* o = new Fl_Input_Choice(10, 480, 106, 14, "Multi");
        o->box(FL_BORDER_FRAME);
        o->down_box(FL_BORDER_FRAME);
        o->color(FL_FOREGROUND_COLOR);
        o->selection_color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        o->textsize(8);
        o->menubutton()->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        multichoice = o;
       
      }
      { Fl_Button* o = new Fl_Button(207, 473, 55, 19, "store multi");
        o->tooltip("overwrite this multi");
        o->box(FL_BORDER_BOX);
        o->labelsize(8);o->labelcolor((Fl_Color)1);
        o->callback((Fl_Callback*)storemulti,multichoice);
      }
      { Fl_Button* o = new Fl_Button(126, 473, 70, 19, "load multi");
        o->tooltip("load current multi");
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->labelcolor((Fl_Color)186);
         o->callback((Fl_Callback*)loadmulti,multichoice);
      }
      // parameter tuning
      { Fl_Value_Input* o = new Fl_Value_Input(710, 480, 106, 14, "current parameter");
        //o->box(FL_BORDER_FRAME);
          o->box(FL_ROUNDED_BOX);
        //o->color(FL_FOREGROUND_COLOR);
        //o->selection_color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        o->textsize(8);
	o->range(-2,2);
        //o->menubutton()->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->step(0.0001);
	o->callback((Fl_Callback*)finetune);
	paramon = o;
      }
      o->end();
  return o;
}

/**
 * constructor
 * calling straight the super class constructor
 */
Fenster::Fenster(int w, int h, const char* t): Fl_Double_Window(w,h,t)
{
}
/**
 * constructor
 * calling straight the super class constructor
 */
Fenster::Fenster(int w, int h): Fl_Double_Window(w,h)
{
}
/**
 * overload the eventhandler for some custom shortcuts
 * @param event
 * @return 1 when all is right
 */
int Fenster::handle (int event)
{
	switch (event)
        {
	case FL_KEYBOARD:
                if (tabs != NULL)
		{
		switch (Fl::event_key ())
                {
                case FL_F+1:
			tabs->value(tab[0]);
			tabcallback(tabs,NULL);
		break;
                case FL_F+2:
			tabs->value(tab[1]);
			tabcallback(tabs,NULL);
		break;
                case FL_F+3:
			tabs->value(tab[2]);
			tabcallback(tabs,NULL);
		break;
                case FL_F+4:
			tabs->value(tab[3]);
			tabcallback(tabs,NULL);
		break;
                case FL_F+5:
			tabs->value(tab[4]);
			tabcallback(tabs,NULL);
		break;
                case FL_F+6:
			tabs->value(tab[5]);
			tabcallback(tabs,NULL);
		break;
                case FL_F+7:
			tabs->value(tab[6]);
			tabcallback(tabs,NULL);
		break;
                case FL_F+8:
			tabs->value(tab[7]);
			tabcallback(tabs,NULL);
		break;
                }// end of switch
		}// end of if
                return 1;

        default:
                // pass other events to the base class...
                
		return Fl_Double_Window::handle(event);
        }
}

void close_cb( Fl_Widget* o, void*) {

   exit(0);
}

