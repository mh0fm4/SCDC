/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
 *  
 *  This file is part of the Simulation Component and Data Coupling (SCDC) library.
 *  
 *  The SCDC library is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  The SCDC library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#define SIZEREAD 10
#define BUFFERSIZE 1000000000

int main(int argc, char *argv[])
{

	FILE *datei;
	int i, j, laenge;
	char *mall = NULL;
	char buffer[SIZEREAD]; //for Read
	size_t len;
	
	mall = (char*) malloc(sizeof(char)*BUFFERSIZE);

	if(mall ==NULL){
		printf("Fehler Speichermangel\n");
	}
	else {

		for (j=0; j<BUFFERSIZE; j++){
			mall[j] = 'A';

		}
		mall[BUFFERSIZE]='\0';
	}

	

	size_t dlenght =strlen( mall);

	
	if((datei = fopen("scdc:/home/Shenlongt1/Desktop/Merge/seito_lib/lib/store/ausgabe.txt", "w+"))== NULL)
	{
		printf("Datei kann nicht geoeffnet werden!\n");
    		exit(1);
	}


	fwrite(mall,dlenght,1,datei);	
	printf("Ftell ist: %d\n", ftell(datei));
	fseek(datei, 0, SEEK_SET);
	printf("Ftell ist: %d\n", ftell(datei));
	len = fread(&buffer,sizeof(char),10-1,datei);
	buffer[len]= '\0';
	printf("Inhalt ist:%s\n", buffer);
	printf("len: %d\n",len);


	fclose (datei);
	free(mall);
  	return EXIT_SUCCESS;

}

