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
// *************************************************************
// new fileoutput

ofstream File ("minicomputerMemory.txt");
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
	for (p=0;p<139;++p)
		File<< "("<< p << ":"<<sounds[i].parameter[p]<<")"<<endl;
	}

File.close();
	
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
			
			/*if (fseek(fh,sizeof(patch),SEEK_CUR)==-1)
				{
					break;
				}*/
	}
	fclose(fh);
	}
// *************************************************************
// new fileinput

ifstream File ("minicomputerMemory.txt");
int p,j,current=-1;
int iParameter,iValue;
string str,sParameter,sValue;
getline(File,str);
bool success;
while (File)
{
	switch (str[0])
	{
		case '[':// setting the current sound inde// setting the current sound indexx
		{
			sParameter="";
			if (parseNumbers(str,iParameter,iValue))
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
}
void Memory::loadMulti()
{

	FILE *fh; // file handle
    if ((fh=fopen("minicomputerMulti.mcm","rb")) ==NULL)
	{
		printf("cant open file minicomputerMulti.mcm\n");
		
	}
	else
	{
	for (int i=0;i<128;i++)
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
}
bool Memory::parseNumbers(string &str,int &iParameter,int &iValue)
{
 bool rueck = false;
 if (!str.empty())
 {
	istringstream sStream;
	string sParameter="";
	string sValue="";
	int index = 0;
	bool isValue = false;
	// first getting each digit character
	while (index<str.length())
	{
		if ((str[index]>='0') && (str[index]<='9'))
		{
			if (isValue)
			{
				sValue+=str[index];
			}
			else
			{
				sParameter+=str[index];
			}
		}
		else if (str[index] == ':')
		{
			isValue=true;
		}
		++index;
	}
	
	// now actually turn them to ints
	sStream.str(sParameter);

	if (sStream>>iParameter)
	{
		rueck = true;
		if (isValue)
		{
			sStream.str(sValue);
			if (sStream>>iValue)
				rueck = true;
			else
				rueck = false;
		}
	}
 }
 return rueck;
}	
