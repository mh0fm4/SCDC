/*
 *  Copyright (C) 2014, 2015, 2016 Michael Hofmann
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

#include "scdc.h"


#define TARGET          "store_test"

#define INPUT_FILE      TARGET ".in"
#define INPUT_OFFSET    319
#define INPUT_SIZE      46

#define PUT_OFFSET      3
/*#define PUT_OFFSET_REL  F*/ /* F = front, B = back */
#define PUT_SIZE        37

#define GET_OFFSET      13 /* = 3+10 */
/*#define GET_OFFSET_REL  B*/ /* F = front, B = back */
#define GET_SIZE        23

#define OUTPUT_FILE     TARGET ".out"
#define OUTPUT_OFFSET   10
#define OUTPUT_SIZE     13

#define REMOVE          1

#define STRINGIFY_(_s_)  #_s_
#define STRINGIFY(_s_)   STRINGIFY_(_s_)


int main(int argc, char *argv[])
{
  const char *uri = "scdc:/store";
  const char *cmd;
  char path[256], *e;
  scdc_dataprov_t dp;
  scdc_dataset_t ds;
  scdc_dataset_input_t _ip, *ip = &_ip;
  scdc_dataset_output_t _op, *op = &_op;


  printf("SCDC filesystem demo for C\n");

  scdc_init(SCDC_INIT_DEFAULT);

  dp = scdc_dataprov_open("store", "fs", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "store/"), path));

  /* open dataset */
  ds = scdc_dataset_open(uri);
  if (ds == SCDC_DATASET_NULL)
  {
    printf("ERROR: open dataset failed!\n");
    goto quit;

  } else printf("open dataset '%s': OK\n", uri);

  /* create input object to read from file */
  ip = scdc_dataset_input_create(ip, "file"
#ifdef INPUT_OFFSET
    ":offset"
#endif
#ifdef INPUT_SIZE
    ":size"
#endif
    , INPUT_FILE
#ifdef INPUT_OFFSET
    , (scdcint_t) INPUT_OFFSET
#endif
#ifdef INPUT_SIZE
    , (scdcint_t) INPUT_SIZE
#endif
    );
  if (ip == NULL)
  {
    printf("ERROR: open input file failed!\n");
    goto quit;
  }

  /* store data from input object to dataset */
  cmd = "put " TARGET " "
#if PUT_OFFSET
    STRINGIFY(PUT_OFFSET)
# ifdef PUT_OFFSET_REL
    STRINGIFY(PUT_OFFSET_REL)
# endif
#endif
#if PUT_SIZE
    ":" STRINGIFY(PUT_SIZE)
#endif
    ;
  if (scdc_dataset_cmd(ds, cmd, ip, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto quit;

  } else printf("storing data with command '%s': OK\n", cmd);

  /* destroy input object */
  scdc_dataset_input_destroy(ip);

  /* inspect target */
  cmd = "ls " TARGET;
  scdc_dataset_output_unset(&_op);
  _op.buf_size = 32;
  void *buf = _op.buf = malloc(_op.buf_size);
  if (scdc_dataset_cmd(ds, cmd, NULL, &_op) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto quit;

  } else printf("inspecting target with command '%s': OK -> '%.*s'\n", cmd, (int) _op.current_size, (const char *) _op.buf);
  while (_op.next) _op.next(&_op);
  free(buf);

  /* create output object to write to file */
  op = scdc_dataset_output_create(op, "file"
#ifdef OUTPUT_OFFSET
    ":offset"
#endif
#ifdef OUTPUT_SIZE
    ":size"
#endif
    , OUTPUT_FILE
#ifdef OUTPUT_OFFSET
    , (scdcint_t) OUTPUT_OFFSET
#endif
#ifdef OUTPUT_SIZE
    , (scdcint_t) OUTPUT_SIZE
#endif
    );
  if (op == NULL)
  {
    printf("ERROR: open output file failed!\n");
    goto quit;
  }

  /* retrieve data from dataset to output object */
  cmd = "get " TARGET " "
#if GET_OFFSET
    STRINGIFY(GET_OFFSET)
# ifdef GET_OFFSET_REL
    STRINGIFY(GET_OFFSET_REL)
# endif
#endif
#if GET_SIZE
    ":" STRINGIFY(GET_SIZE)
#endif
    ;
  if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto quit;

  } else printf("retrieving data with command '%s': OK\n", cmd);

  /* destroy output object */
  scdc_dataset_output_destroy(op);

#if REMOVE
  /* remove the data stored */
  cmd = "rm " TARGET;
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto quit;

  } else printf("removing data with command '%s': OK\n", cmd);
#endif

  /* close dataset */
  scdc_dataset_close(ds);

quit:
  scdc_dataprov_close(dp);

  scdc_release();

  return 0;
}
