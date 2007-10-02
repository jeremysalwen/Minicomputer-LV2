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

#ifndef PATCH_H_
#define PATCH_H_
#include <string> //string
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
//#include <boost/serialization/serialization.hpp>
//#include <boost/serialization/access.hpp>
using namespace std;

class patch
{
	public:
	// friend class boost::serialization::access;
  //template<class Archive>
  //void serialize(Archive & ar, const unsigned int version)
  //{
    //ar & parameter;
    //ar & freq;
    //ar & choice;
    //ar & name;
  //}
	patch();
	float parameter[139];
	float freq[9][2];
	int choice[17];
	string name;
	virtual ~patch();
	//private:
};

#endif /*PATCH_H_*/
