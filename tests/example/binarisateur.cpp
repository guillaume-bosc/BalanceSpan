/*
 *  binarisateur.cpp
 *  
 *
 *  Created by Chedy Raïssi on 25/10/10.
 *  Copyright 2010 INRIA. All rights reserved.
 *
 */

#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>

using namespace std;


int main(int argc, char **argv)
{
	if(argc <2)
	{
		cout << "ASCII2QUEST files binary convertern " << endl;
		cout << "Usage " << argv[0] << " [ASCII_FILENAME] "<< endl << endl;
		exit(0);
	}
		
	string filename(argv[1]);
	string asciifilename = filename+".bin";
	ofstream fo(asciifilename.c_str(), ios::out |ios::trunc| ios::binary);
	
	ifstream infile;
	
	infile.open (filename.c_str(), ifstream::in);
	int item;
	
	while (infile >> item)
		fo.write((char *)&item, sizeof(int));
	

	fo.close();
	infile.close();
	cout << "Done. Happy mining..." << endl;
	return 0;
	
	
}
