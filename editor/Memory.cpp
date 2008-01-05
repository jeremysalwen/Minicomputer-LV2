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

Memory::Memory()
{
	int i;
	for (i=0;i<8;i++)
		choice[i] = 0;
		
	char zeichenkette[128];
	for ( i = 0;i<512;i++)
	{
		sprintf(zeichenkette,"%i untitled",i);
		strcpy(sounds[i].name,zeichenkette);
	}
	
	for ( i = 0;i<128;i++)
	{
		sprintf(zeichenkette,"%i untitled",i);
		strcpy(multis[i].name,zeichenkette);
	}
}

Memory::~Memory()
{
}
string Memory::getName(unsigned int voice,unsigned int Eintrag)
{
	return sounds[Eintrag].name;
}
void Memory::setChoice(unsigned int voice,unsigned int i)
{
	if ((i>=0) && (i<512))
	{
		choice[voice] = i;
	}
	else 
	{
		printf("illegal sound choice\n");
		fflush(stdout);
		
	}
}
unsigned int Memory::getChoice(unsigned int voice)
{
	return choice[voice];
}
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
#ifdef _BINFILE
 FILE *fh; // file handle
  system("mv minicomputerMemory.mcm minicomputerMemory.bak");// make a backup
    if ((fh=fopen("minicomputerMemory.mcm","wb")) ==NULL)
	{
		printf("cant open file minicomputerMemory.mcm\n");
		
	}
	else
	{
	for (int i=0;i<512;i++)
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
// new fileoutput

ofstream File ("minicomputerMemory.temp");
int p,j;
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
	for (p=0;p<_PARACOUNT;++p)
		File<< "("<< p << ":"<<sounds[i].parameter[p]<<")"<<endl;
	}

File.close();
	
  system("mv minicomputerMemory.txt minicomputerMemory.txt.bak");// make a backup
  system("mv minicomputerMemory.temp minicomputerMemory.txt");
}
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

#ifdef _BINFILE
	FILE *fh; // file handle
    if ((fh=fopen("minicomputerMemory.mcm","rb")) ==NULL)
	{
		printf("cant open file minicomputerMemory.mcm\n");
		
	}
	else
	{
	for (int i=0;i<512;i++)
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
// new fileinput

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
				sounds[current].choice[iParameter]=fValue;
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

	getline(File,str);
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

void Memory::saveMulti()
{
      int i;
#ifdef _BINFILE      
	FILE *fh; // file handle
  	system("mv minicomputerMulti.mcm minicomputerMulti.bak");
	if ((fh=fopen("minicomputerMulti.mcm","wb")) ==NULL)
	{
		printf("cant open file minicomputerMulti.mcm\n");
		
	}
	else
	{
	for (int i=0;i<128;i++)
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
for (i=0;i<128;++i)
{
	File<< "["<<i<<"]" <<endl;
	File<< "'"<<multis[i].name<<"'"<<endl;
	
	for (p=0;p<8;++p) // store the sound ids
	{
		File<< "("<< p << ":" <<multis[i].sound[p]<<")"<<endl;
	}
	}

File.close();
	
  system("mv minicomputerMulti.txt minicomputerMulti.txt.bak");// make a backup
  system("mv minicomputerMulti.temp minicomputerMulti.txt");
}
void Memory::loadMulti()
{
int i;
#ifdef _BINFILE
FILE *fh; // file handle
    if ((fh=fopen("minicomputerMulti.mcm","rb")) ==NULL)
	{
		printf("cant open file minicomputerMulti.mcm\n");
		
	}
	else
	{
	for (i=0;i<128;i++)
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
string str,sValue,sParameter;
int iParameter,i2Parameter;
float fValue;

ifstream File ("minicomputerMulti.txt");
getline(File,str);
int p,j,current=0;
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

	}

	getline(File,str);
}
File.close();
}
bool Memory::parseNumbers(string &str,int &iParameter,int &i2Parameter,float &fValue)
{
 bool rueck = false;
 if (!str.empty())
 {
	istringstream sStream,fStream,s2Stream;
	string sParameter="";
	string sValue="";
	string sP2="";
	int index = 0;
	bool hasValue = false,hasP2 = false,isValue=false,isP2=false;
	// first getting each digit character
	while (index<str.length())
	{
		if ((str[index]>='0') && (str[index]<='9'))
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
		++index;
	}
//	cout << sParameter<<" sp2 "<<sP2<<" value "<<sValue<<endl;
	// now actually turn them to ints
	sStream.str(sParameter);

	if (sStream>>iParameter)
	{
		rueck = true;
		if (hasValue)
		{
			fStream.str(sValue);
			if (fStream>>fValue)
				rueck = true;
			else
				rueck = false;
		}
		
		if (hasP2)
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
