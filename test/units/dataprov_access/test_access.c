/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
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


#define TARGET       "access_test"
#define INPUT_FILE   "access_test.in"
#define OUTPUT_FILE  "access_test.out"

#define ACCESS_FS_CONF      "/tmp/"
// #define ACCESS_WEBDAV_CONF  getenv("MERGE_SCDC_WEBDAV_CONF")
#define ACCESS_WEBDAV_CONF  "http://localhost:12345/"
#define ACCESS_NFS_CONF     "nfs://127.0.0.1/srv/nfs_test"


int test_access(const char *uri)
{
  char cmd[512];
  scdc_dataset_t ds;
  scdc_dataset_input_t _ip, *ip = &_ip;
  scdc_dataset_output_t _op, *op = &_op;

#if 1
  printf("TEST command 'cd'\n");
  sprintf(cmd, "%s/foo", uri);
  ds = scdc_dataset_open(uri);
  printf("open: '%s', result: '%s'\n", uri, scdc_last_result());

  sprintf(cmd, "cd bar");
  scdc_dataset_cmd(ds, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

  sprintf(cmd, "pwd");
  scdc_dataset_cmd(ds, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

  sprintf(cmd, "cd /bar");
  scdc_dataset_cmd(ds, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

  sprintf(cmd, "pwd");
  scdc_dataset_cmd(ds, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

  sprintf(cmd, "cd");
  scdc_dataset_cmd(ds, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

  sprintf(cmd, "pwd");
  scdc_dataset_cmd(ds, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

  scdc_dataset_close(ds);
  printf("close: '%s', result: '%s'\n", uri, scdc_last_result());
#endif

#if 1
  printf("TEST command 'ls'\n");
  sprintf(cmd, "%s ls", uri);
  scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());
#endif

#if 1
  printf("TEST command 'info'\n");
  sprintf(cmd, "%s/foo info", uri);
  scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

  sprintf(cmd, "%s/foo info bar", uri);
  scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());
#endif

#if 1
  printf("TEST command 'mkd'\n");
  sprintf(cmd, "%s mkd foo", uri);
  scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

  sprintf(cmd, "%s rm foo", uri);
  scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());
#endif

#if 1
  printf("TEST command 'put'\n");
  /* create input object to read from file */
  ip = scdc_dataset_input_create(ip, "file", INPUT_FILE);
  if (ip != NULL)
  {
    /* store data from input object */
    sprintf(cmd, "%s put " TARGET, uri);
    scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, ip, NULL);
    printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());

    /* destroy input object */
    scdc_dataset_input_destroy(ip);

  } else printf("ERROR: open input file failed!\n");
#endif

#if 1
  printf("TEST command 'get'\n");
  /* create output object to write to file */
  op = scdc_dataset_output_create(op, "file", OUTPUT_FILE);
  if (op != NULL)
  {
    /* retrieve data from dataset to output object */
    sprintf(cmd, "%s get " TARGET, uri);
    scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, op);
    printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());
    
    /* destroy output object */
    scdc_dataset_output_destroy(op);

  } else printf("ERROR: open output file failed!\n");
#endif

#if 1
  /* remove the data stored */
  sprintf(cmd, "%s rm " TARGET, uri);
  scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, op);
  printf("cmd: '%s', result: '%s'\n", cmd, scdc_last_result());
#endif

  return 1;
}


int main(int argc, char *argv[])
{
  const char *uri = "scdc:/access";
  scdc_dataprov_t dp;


  printf("SCDC access demo for C\n");

  scdc_init(SCDC_INIT_DEFAULT);

  // dp = scdc_dataprov_open("access", "access:stub");
  dp = scdc_dataprov_open("access", "access:fs", ACCESS_FS_CONF);
  // dp = scdc_dataprov_open("access", "access:webdav:no100continue", ACCESS_WEBDAV_CONF);
  // dp = scdc_dataprov_open("access", "access:nfs", ACCESS_NFS_CONF);

  test_access(uri);

  scdc_dataprov_close(dp);

  scdc_release();

  return 0;
}
