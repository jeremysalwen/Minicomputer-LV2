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


#ifndef MEMORY_H_
#define MEMORY_H_
#include <iostream> //iostream
#include <fstream> //fstream
#include <sstream> //sstream
#include <string> //string
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
#include <stdio.h>
#include <stdlib.h>
#include "../common.h"
using namespace std;
//#include "patch.h"
/**
 * the struct of an single sound setting
 */
typedef struct setting
{
	float parameter[_PARACOUNT];
	float freq[9][2];
	int choice[17];
	char name[128];
} patch;
/**
 * the struct of a multitemperal setup
 */
typedef struct mode
{
	unsigned int sound[8];// sound ids for the 8 voices
	char name[128];//the name, up to 128 letters
	float settings[8][_MULTISETTINGS];// additional settings for volumes
} multi;

/**
 * the class for the whole memory
 * see Memory.cpp for more documentation
 */
class Memory
{
public:
	Memory();
	void save();
	void load();
	void saveMulti();
	void loadMulti();
	void store(patch Sound);
	void overwrite(patch Sound);
	patch temp;
	string getName(unsigned int voice,unsigned int Eintrag);
	virtual ~Memory();
	patch sounds[512];
	multi multis[128];
	void setChoice(unsigned int voice,unsigned int i);
	unsigned int getChoice(unsigned int voice);
	private:
	unsigned int choice[8];
	bool parseNumbers(string &str,int &iParameter,int &i2Parameter,float &fValue);
};

#endif /*MEMORY_H_*/
