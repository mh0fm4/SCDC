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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "/home/Shenlongt1/Desktop/Merge/SCDC/src/include/scdc.h"

struct help {

	int type ;
	long int offset;
	void *ptr;
};

FILE *datei;

#define MAGIC_CONST  2501

#define TARGET       "store_test"


int  scdc =0;

FILE *fopen(const char *filename, const char *mode){

	printf("In our own fopen, opening %s\n", filename);
	
	if((strstr(filename, "scdc"))==NULL){

		FILE *(*original_fopen)(const char*, const char*);
    		original_fopen = dlsym(RTLD_NEXT, "fopen");

		void *ptr = (*original_fopen)(filename, mode);

		return ptr;
	}

	else{
		
		if(scdc ==0){	
			scdc_init(SCDC_INIT_DEFAULT);
			scdc_dataprov_t dp;
			char *e, path[256];

			dp = scdc_dataprov_open("store", "fs:access", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "/home/Shenlongt1/Desktop/Merge/seito_lib/lib/store"), path));	

			scdc++;	
		}

		struct help* data= (struct help*) malloc(sizeof (struct help));
		struct help *help_zeiger;
		help_zeiger = data;

		const char *uri = "scdc+tcp://192.168.2.108/store";
		scdc_dataset_t ds;
		ds = scdc_dataset_open(uri);

  		if (ds == SCDC_DATASET_NULL)
  		{
   	 		printf("ERROR: open dataset failed!\n");
			scdc_release();
			return 0;
 		} 
		else{ 
			printf("open dataset '%s': OK\n", uri);

			data->type = MAGIC_CONST;
			data->ptr = ds;
			data->offset = 0;
		
			return (FILE*) help_zeiger;
		}
	}		
}

size_t fwrite(const void *ptr, size_t size, size_t nobj, FILE *stream){

	printf("In our own fwrite:\n");
	struct help* data = (struct help *) stream;

	if(data->type != MAGIC_CONST){
		printf("Ohne scdc\n");

		FILE *(*original_fwrite)(const void*, size_t, size_t, FILE *);
    		original_fwrite = dlsym(RTLD_NEXT, "fwrite");
    		size_t rfwrite = (int) (*original_fwrite)(ptr, size, nobj, stream);
		
		if(rfwrite!= nobj){
			printf("Error Writing to File\n");
			return -1;
		}	
		else	
			return nobj;
	}
	else{

		printf("In scdc\n");
		char cmd[256];
		scdc_dataset_input_t ip;

		scdc_dataset_input_unset(&ip);
		ip.buf = (void *) ptr;
		ip.buf_size = size * nobj;
		ip.current_size = size * nobj;
		ip.next =0;

		sprintf(cmd, "put %s %ld:%d", TARGET, data->offset, (size*nobj));
		
		if (scdc_dataset_cmd(data->ptr, cmd , &ip, NULL) != SCDC_SUCCESS)
  		{
    			printf("ERROR: command '%s' failed!\n", cmd);
			scdc_release();
			return -1;

  		} 
		else{ 
			printf("storing data with command '%s': OK\n", cmd);
			data->offset = data->offset + (size*nobj);
			return nobj;
		}		

	}	
	
}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream){

	printf("In our own fread:\n");
	struct help* data = (struct help *) stream;

	if(data->type != MAGIC_CONST){
		printf("Ohne scdc\n");

		FILE *(*original_fread)(void*, size_t, size_t, FILE *);
    		original_fread = dlsym(RTLD_NEXT, "fread");
		size_t rfread = (int)(*original_fread)(ptr, size, count, stream);

		if(rfread != count){
			printf("Reading error\n");
			return -1;
		}
		else
			return rfread;
	}
	else{
	
		printf("In scdc\n");
		char cmd[256];
		scdc_dataset_output_t op;

		scdc_dataset_output_unset(&op);
		op.buf = ptr;
		op.buf_size = size * count;
		op.current_size =0;
		op.next =0;
		
		sprintf(cmd, "get %s %ld:%d", TARGET, data->offset, (size*count));
			
		if (scdc_dataset_cmd(data->ptr, cmd, NULL, &op) != SCDC_SUCCESS)
  		{
    			printf("ERROR: command '%s' failed!\n", cmd);
			scdc_release();
			return -1;
  		} 
		else{
			printf("retrieving data with command '%s': OK\n", cmd);	
			
			if(op.buf != ptr){

				char *dst = (char *)ptr;
				do{	
					memcpy(dst, op.buf, op.current_size);
					dst += op.current_size;	
				}

				while (op.next != NULL && op.next(&op) == SCDC_SUCCESS);

				count = (dst-((char *) ptr)) /size;
				
			}		
			return count;
		}	

	}

}


int fseek(FILE *stream, long int offset, int wherefrom){

	printf("In our own fseek:\n");
	
	struct help* data = (struct help *) stream;

	if(data->type != MAGIC_CONST){
		printf("Ohne scdc\n");	

		FILE *(*original_fseek)(FILE *, long int, int);
    		original_fseek = dlsym(RTLD_NEXT, "fseek");
		int rfseek = (int)(*original_fseek)(stream, offset, wherefrom);	
		printf("Rückgabe fseek:%d \n", rfseek);
		if (rfseek !=0)
			return -1;
		else
			return rfseek;
	}
	else {
		printf("In scdc\n");

		if(wherefrom == SEEK_SET){			
			data->offset =0+offset;
		}

		if(wherefrom == SEEK_CUR){  
			
			data->offset =data->offset + offset; // muss auch dazu gerechnet werden, da offset - ist. - und - würde + ergeben
			if(data->offset < 0){
				printf("Fehler, offset nicht in den Dateigrenzen, Dateizeiger auf anfang gesetzt\n");
				data->offset =0;
			}
		}

		/* muss noch überarbeitet werden		
		if(wherefrom ==2){
			
			char *cmd;
			 inspect target 
			scdc_dataset_output_t op;
  			cmd = "ls " TARGET;
  			scdc_dataset_output_unset(&op);
  			op.buf_size = 32;
  			void *buf = op.buf = malloc(op.buf_size);//lokale variable mit char[32]
  			if (scdc_dataset_cmd(data->ptr, cmd, NULL, &op) != SCDC_SUCCESS)
  			{
  				printf("ERROR: command '%s' failed!\n", cmd);
	    			scdc_release();
				return -1;
  			}
		 
			else {
				printf("inspecting target with command '%s': OK -> '%.*s'\n", cmd, (int) op.current_size, (const char *) op.buf);
				//printf("Buffer_op ist:%d\n",op.current_size);
				// currrent ist größe string
				data->offset = op.current_size; 
  				while (op.next) op.next(&op);
				
  				free(buf);//so weg
				return -1;
			}
		}*/
		return -1;	
	}
}

long int ftell(FILE *stream){

	printf("In our own ftell:\n");
	struct help* data = (struct help *) stream;	

	if(data->type != MAGIC_CONST){
		printf("Ohne scdc\n");

		FILE *(*original_ftell)(FILE *);
    		original_ftell = dlsym(RTLD_NEXT, "ftell");
		long int rftell = (int)(*original_ftell)(stream);

		if (rftell ==-1)
			return -1;
		else
			return rftell;
	}
	else{
		printf("In scdc\n");
		return data->offset;
	}
}

int fclose(FILE *stream){

	printf("In our own fclose:\n");

	struct help* data = (struct help *) stream;
	
	if(data->type != MAGIC_CONST){
		printf("Ohne SCDC\n");

		FILE *(*original_fclose)(FILE *);
    		original_fclose = dlsym(RTLD_NEXT, "fclose");
			int rfclose = (int)(*original_fclose)(stream);
		if (rfclose != 0)
			return EOF;
		else
			return rfclose;
	}

	else {
		printf("In SCDC\n");
		if(scdc !=0){
			scdc_release();
			scdc =0;
			return 0;
		}		
		else
			return -1;
	}
}
