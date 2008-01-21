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
#include "Memory.h"
/**
 * constructor
 */
Memory::Memory()
{
	int i;
	for (i=0;i<8;++i)
		choice[i] = 0;
		
	char zeichenkette[128];
	for ( i = 0;i<512;++i)
	{
		sprintf(zeichenkette,"%i untitled",i);
		strcpy(sounds[i].name,zeichenkette);
	}
	
	for ( i = 0;i<128;++i)
	{
		sprintf(zeichenkette,"%i untitled",i);
		strcpy(multis[i].name,zeichenkette);
	}
}
/**
 * destructor
 */
Memory::~Memory()
{
}

/**
 * retreive name of a certain patch
 * @param voicenumber of which we want to know the entry (should be redundant)
 * @param soundnumber
 * @return the name as string
 */
string Memory::getName(unsigned int voice,unsigned int Eintrag)
{
	return sounds[Eintrag].name;
}
/**
 * set the soundnumber to a certain voice
 * @param voice number
 * @param sound number
 */
void Memory::setChoice(unsigned int voice,unsigned int i)
{
	if ((i>=0) && (i<512)) // check if its in the range
	{
		choice[voice] = i;
	}
	else // oha, should not happen
	{
		printf("illegal sound choice\n");
		fflush(stdout);
		
	}
}
/**
 * return a sound id of a certain given voice number
 * @param the voice number
 * @return the sound number of that voice
 */
unsigned int Memory::getChoice(unsigned int voice)
{
	return choice[voice];
}
/**
 * save the complete soundmemory to disk
 * two fileformats are possible, the historical
 * but depricated binary format with is just a memory dump
 * and not extensible and the textformat, which takes more space
 * on harddisk but is human editable and extensible
 * per default the binary format is switched off and can
 * be enabled with the _BINFILE option in common.h
 * the filenames are fix
 */
void Memory::save()
{
    /*ofstream ofs("minicomputerMemory.mcm", std::ios::binary);
  //boost::archive::text_oarchive oa(ofs);
	for (int i=0;i<139;i++)
	{
	ofs<<sounds[0].parameter[i];
	//string name = sounds[0].name;
	//ofs.write((char *)&name, sizeof(name));
  //oa << sounds[i];
	}
  ofs.close();*/
  //................................binary format, depricated! ............................
#ifdef _BINFILE
FILE *fh; // file handle
  system("mv minicomputerMemory.mcm minicomputerMemory.bak");// make a backup
    if ((fh=fopen("minicomputerMemory.mcm","wb")) ==NULL)
	{
		printf("cant open file minicomputerMemory.mcm\n");
		
	}
	else
	{
	for (int i=0;i<512;++i) // dump the whole stuff to disk
	{
		if ((fwrite(&sounds[i],sizeof(patch),1,fh)) == -1)
			{
         		printf("cant write into minicomputerMemory.mcm\n");
         		break;
			}
			fseek(fh,0,SEEK_END);
	}
	fclose(fh);
	}
#endif	
// *************************************************************
// new fileoutput as textfile with a certain coding which is
// documented in the docs

ofstream File ("minicomputerMemory.temp"); // temporary file
int p,j;
for (int i=0;i<512;++i)
 {  
	File<< "["<<i<<"]" <<endl;// write the soundnumber
	File<< "'"<<sounds[i].name<<"'"<<endl;// write the name
	
	for (p=0;p<9;++p)
	{
		for (j=0;j<2;++j)
			File<< "<"<< p << ";" << j << ":" <<sounds[i].freq[p][j]<<">"<<endl;
	}
	for (p=0;p<17;++p)
		File<< "{"<< p << ":"<<sounds[i].choice[p]<<"}"<<endl;
	for (p=0;p<_PARACOUNT;++p)// write the remaining parameters
		File<< "("<< p << ":"<<sounds[i].parameter[p]<<")"<<endl;
 }// end of for i

File.close();
	
  system("mv minicomputerMemory.txt minicomputerMemory.txt.bak");// make a backup
  system("mv minicomputerMemory.temp minicomputerMemory.txt");// commit the file finally
}

/**
 * load the soundmemory from disk
 * supports the depricated binary and textformat.
 * @see Memory::save()
 */
void Memory::load()
{
	/*ifstream ifs("minicomputerMemory.mcm", std::ios::binary);
  // boost::archive::text_iarchive ia(ifs, std::ios::binary);
	for (int i=0;i<139;i++)
	{
		ifs>>sounds[0].parameter[i];
		 //ia >> sounds[i];
//		 string name;
//		ifs.read((char *)name, sizeof(name));
//		sounds[0].name=name;
	}
	ifs.close();
	printf("so ...\n");
	choice=2;*/
// the depricated binary format, enabled with the _BINFILE parameter in common.h
#ifdef _BINFILE
	FILE *fh; // file handle
    if ((fh=fopen("minicomputerMemory.mcm","rb")) ==NULL)
	{
		printf("cant open file minicomputerMemory.mcm\n");
		
	}
	else
	{
	for (int i=0;i<512;++i) //read the whole stuff into memory
	{
		if ((fread(&sounds[i],sizeof(patch),1,fh)) == -1)
			{
         		printf("cant read  minicomputerMemory.mcm\n");
         		break;
			}
			
			//if (fseek(fh,sizeof(patch),SEEK_CUR)==-1)
			//	{
			//		break;
			//	}
	}
	fclose(fh);
	}
#endif
// *************************************************************
// new fileinput in textformat which is the way to go

ifstream File ("minicomputerMemory.txt");
int p,j,current=-1;
int iParameter,i2Parameter;
float fValue;
string str,sParameter,sValue;
getline(File,str);
bool success;
while (File)
{
	sParameter="";
	sValue = "";
	switch (str[0])
	{
		case '(':// setting parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].parameter[iParameter]=fValue;
			}
		}
		break;
		case '{':// setting additional parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].choice[iParameter]=(int)fValue;
			}
		}
		break;
		case '<':// setting additional parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				sounds[current].freq[iParameter][i2Parameter]=fValue;
			}
		}
		break;
		case '\'': // setting the name
		{
			j = 1; // important, otherwise it would bail out at the first '	
			while ((j<str.length()) && (str[j]!='\'') && (j<128) )
			{
				sounds[current].name[j-1] = str[j];
				++j;
			}
		}
		break;
		case '[':// setting the current sound index
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				current = iParameter;
			}
		}
		break;

	}

	getline(File,str);// get the next line of the file
/*
for (int i=0;i<512;++i)
	{
	File<< "["<<i<<"]" <<endl;
	File<< "'"<<sounds[i].name<<"'"<<endl;
	
	for (p=0;p<9;++p)
	{
		for (j=0;j<2;++j)
			File<< "<"<< p << ";" << j << ":" <<sounds[i].freq[p][j]<<">"<<endl;
	}
	for (p=0;p<17;++p)
		File<< "{"<< p << ":"<<sounds[i].choice[p]<<"}"<<endl;
	for (p=0;p<139;++p)
		File<< "("<< p << ":"<<sounds[i].parameter[p]<<")"<<endl;
	}
*/
}
File.close();
}
/**
 * the multitemperal setup, the choice of sounds and some settings
 * are saved to an extra file
 * supports the old depricated binary format and the new textformat like the sound saving
 * @see Memory::save()
 */
void Memory::saveMulti()
{
      int i;
      //************* the binary depricated fileformat
#ifdef _BINFILE      
	FILE *fh; // file handle
  	system("minicomputerMulti.mcm minicomputerMulti.bak");
	if ((fh=fopen("minicomputerMulti.mcm","wb")) ==NULL)
	{
		printf("cant open file minicomputerMulti.mcm\n");
		
	}
	else
	{
	for (int i=0;i<128;++i) // write the 128 multis
	{
		if ((fwrite(&multis[i],sizeof(multi),1,fh)) == -1)
			{
         		printf("cant write into minicomputerMulti.mcm\n");
         		break;
			}
			fseek(fh,0,SEEK_END);
	}
	fclose(fh);
	}
#endif
//---------------------- new textformat
// first write in temporary file, just in case
ofstream File ("minicomputerMulti.temp");
int p,j;
for (i=0;i<128;++i)// write the whole 128 multis
{
	File<< "["<<i<<"]" <<endl;// write the multi id number
	File<< "'"<<multis[i].name<<"'"<<endl;// write the name of the multi
	
	for (p=0;p<8;++p) // store the sound ids of all 8 voices
	{
		File<< "("<< p << ":" <<multis[i].sound[p]<<")"<<endl;
	for (j=0;j<_MULTISETTINGS;++j)
		File<< "{"<< p << ";"<< j << ":" <<multis[i].settings[p][j]<<"}"<<endl;
	}
}

File.close();// done
	
  system("mv minicomputerMulti.txt minicomputerMulti.txt.bak");// make a backup of the original file
  system("mv minicomputerMulti.temp minicomputerMulti.txt");// commit the file
}
/**
 * load the multitemporal setups which are stored in an extrafile
 * supports the depricated binary format, enabled via _BINFILE in common.h
 * and the new textformat
 * @see Memory::load
 * @see Memory::save
 */
void Memory::loadMulti()
{
int i;
//***************** the depricated binary format, only for my personal backwards compatility
#ifdef _BINFILE
FILE *fh; // file handle
    if ((fh=fopen("minicomputerMulti.mcm","rb")) ==NULL)
	{
		printf("cant open file minicomputerMulti.mcm\n");
		
	}
	else
	{
	for (i=0;i<128;++i)// load all the setups
	{
		if ((fread(&multis[i],sizeof(multi),1,fh)) == -1)
			{
         		printf("cant read  minicomputerMulti.mcm\n");
         		break;
			}
			
			/*if (fseek(fh,sizeof(patch),SEEK_CUR)==-1)
				{
					break;
				}*/
	}
	fclose(fh);
	}
#endif
// *********************************** the new text format **********************
string str,sValue,sParameter;
int iParameter,i2Parameter;
float fValue;

ifstream File ("minicomputerMulti.txt");
getline(File,str);// get the first line from the file
int p,j,current=0;
while (File)// as long there is anything in the file
{
	// reset some variables
	sParameter="";
	sValue = "";
	// parse the entry (line) based on the first character
	switch (str[0])
	{
		case '(':// setting parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				multis[current].sound[iParameter]=(int)fValue;
			}
		}
		break;
		case '\'': // setting the name
		{
			j = 1; // important, otherwise it would bail out at the first '	
			while ((j<str.length()) && (str[j]!='\'') && (j<128) )
			{
				multis[current].name[j-1] = str[j];
				++j;
			}
		}
		break;
		case '[':// setting the current sound index
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				current = iParameter;
			}
		}
		break;
		case '{':// setting additional parameter
		{
			if (parseNumbers(str,iParameter,i2Parameter,fValue))
			{
				multis[current].settings[iParameter][i2Parameter]=fValue;
			}
		}
		break;

	}// end of switch

	getline(File,str);// get the next line
}// end of while (file)
File.close();// done
}

/**
 * parse parameter and values out of a given string
 * parameters are addresses of the actual variable to return the values with it
 *
 * @param string the line
 * @param int the first parameter
 * @param int the second, optional parameter
 * @param float the actual value
 * @return bool, true if the parsing worked
 */
bool Memory::parseNumbers(string &str,int &iParameter,int &i2Parameter,float &fValue)
{
 bool rueck = false;
 if (!str.empty())// just to make sure
 {
	istringstream sStream,fStream,s2Stream;
	string sParameter="";
	string sValue="";
	string sP2="";
	int index = 0;
	bool hasValue = false,hasP2 = false,isValue=false,isP2=false;
	// first getting each digit character
	while (index<str.length())// examine character per character
	{
		if ((str[index]>='0') && (str[index]<='9'))// is it a number?
		{
			if (isValue)
			{
				sValue+=str[index];
			}
			else if (isP2)
			{
				sP2+=str[index];
			}
			else
			{	
				sParameter+=str[index];
			}
		}
		else if (str[index] == '.')
		{
			if (isValue)
			{
				sValue+=str[index];
			}
		}
		else if (str[index] == ':')
		{
			hasValue = true;
			isValue = true;
			isP2 = false;
		}
		else if (str[index] == ';')
		{
			hasP2 	= true;
			isP2	= true;
			isValue = false;
		}
		++index;// next one please
	}
//	cout << sParameter<<" sp2 "<<sP2<<" value "<<sValue<<endl;
	// now actually turn them to ints or float
	sStream.str(sParameter);

	if (sStream>>iParameter)
	{
		rueck = true;
		if (hasValue)// turn the value into a float
		{
			fStream.str(sValue);
			if (fStream>>fValue)
				rueck = true;
			else
				rueck = false;
		}
		
		if (hasP2)// turn the second parameter into an int
		{
			s2Stream.str(sP2);
			if (s2Stream>>i2Parameter)
				rueck = true;
			else
				rueck = false;
		}
	}
 }
// cout << "p1: " << iParameter << " p2: " << i2Parameter<<" value: " << fValue<<" rueck: "<<rueck<<endl;
 return rueck;
}	
