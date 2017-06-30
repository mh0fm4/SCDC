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

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZEREAD    10
#define BUFFERSIZE  1000000


#if 0
void test_seito(const char *file)
{
  FILE *datei;
  int j;
  char *mall = NULL;
  char buffer[SIZEREAD];
  size_t len;

  mall = (char*) malloc(sizeof(char)*BUFFERSIZE);

  if (mall == NULL)
  {
    printf("Fehler: Speichermangel\n");
    exit(1);

  }  else
  {
    for (j = 0; j < BUFFERSIZE - 1; j++)
    {
      mall[j] = 'A';
    }
    mall[BUFFERSIZE - 1] = '\0';
  }

  size_t dlenght = strlen(mall);

  if ((datei = fopen(file, "w+")) == NULL)
  {
    printf("Datei kann nicht geoeffnet werden!\n");
    exit(1);
  }

  fwrite(mall, dlenght, 1, datei);

  printf("ftell ist: %ld\n", ftell(datei));

  fseek(datei, 0, SEEK_SET);

  printf("ftell ist: %ld\n", ftell(datei));

  len = fread(buffer, sizeof(char), SIZEREAD - 1, datei);

  buffer[len]= '\0';

  printf("Inhalt ist: %s\n", buffer);

  printf("len: %zu\n", len);

  fflush(datei);

  fclose(datei);

  free(mall);
}


void test_formated(const char *file)
{
  FILE *f;

  f = fopen(file, "w");

  fprintf(f, "%d  %d\n", 1, 2);

  fprintf(f, "%d  %d\n", 3, 4);

  fclose(f);

  f = fopen(file, "r");

  int i, j;
  fscanf(f, "%d  %d\n", &i, &j);

  printf("i = %d, j = %d\n", i, j);

  fscanf(f, "%d  %d\n", &i, &j);

  printf("i = %d, j = %d\n", i, j);

  fclose(f);
}


void test_rename(const char *old, const char *new)
{
  rename(old, new);
}
#endif


void test_stat(const char *path)
{
  struct stat s;

  stat(path, &s);

  printf("st_size: %ld\n", (long) s.st_size);

  lstat(path, &s);

  printf("st_size: %ld\n", (long) s.st_size);
}


int main(int argc, char *argv[])
{
  --argc; ++argv;

/*  if (argc > 0) test_formated(argv[0]);*/

/*  if (argc > 1) test_rename(argv[0], argv[1]);*/

  if (argc > 0) test_stat(argv[0]);

  return EXIT_SUCCESS;
}
