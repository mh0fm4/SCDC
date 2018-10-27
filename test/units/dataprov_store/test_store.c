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


#define TARGET       "store_test"
#define INPUT_FILE   "store_test.in"
#define OUTPUT_FILE  "store_test.out"


#define STORE_FS_CONF      "/tmp/"
#define STORE_MYSQL_CONF   getenv("MERGE_SCDC_MYSQL_CREDENTIAL")
/* #define STORE_WEBDAV_CONF  getenv("MERGE_SCDC_WEBDAV_CONF") */
#define STORE_WEBDAV_CONF  "http://localhost:12345/"
#define STORE_NFS_CONF     "nfs://127.0.0.1/srv/nfs_test"


int test_store(const char *uri)
{
  char *cmd;
  scdc_dataset_t ds;
  scdc_dataset_input_t _ip, *ip = &_ip;
  scdc_dataset_output_t _op, *op = &_op;


  /* open dataset */
  ds = scdc_dataset_open(uri);
  if (ds == SCDC_DATASET_NULL)
  {
    printf("ERROR: open dataset failed!\n");
    goto do_return;

  } else printf("open dataset '%s': OK\n", uri);

  /* become admin */
  cmd = "cd ADMIN";
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("becoming admin with command '%s': OK\n", cmd);

#if 1
  /* remove store (do not care whether it fails or not) */
  cmd = "rm storage_demo";
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("WARNING: command '%s' failed!\n", cmd);

  } else printf("removing store with command '%s': OK\n", cmd);
#endif

#if 1
  /* add store */
  cmd = "put storage_demo";
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("adding store with command '%s': OK\n", cmd);
#endif

  /* select store */
  cmd = "cd storage_demo";
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("selecting store with command '%s': OK\n", cmd);

  /* create input object to read from file */
  ip = scdc_dataset_input_create(ip, "file", INPUT_FILE);
  if (ip == NULL)
  {
    printf("ERROR: open input file failed!\n");
    goto do_return;
  }

  /* store data from input object to dataset */
  cmd = "put " TARGET;
  if (scdc_dataset_cmd(ds, cmd, ip, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("storing data with command '%s': OK\n", cmd);

  /* destroy input object */
  scdc_dataset_input_destroy(ip);

#if 1
  /* create output object to write to file */
  op = scdc_dataset_output_create(op, "file", OUTPUT_FILE);
  if (op == NULL)
  {
    printf("ERROR: open output file failed!\n");
    goto do_return;
  }

  /* retrieve data from dataset to output object */
  cmd = "get " TARGET;
  if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("retrieving data with command '%s': OK\n", cmd);

  /* destroy output object */
  scdc_dataset_output_destroy(op);
#endif

  /* info entry */
  cmd = "info " TARGET;
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("getting info with command '%s': OK\n", cmd);

  /* list store */
  cmd = "ls";
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("listing data with command '%s': OK\n", cmd);

  /* remove the data stored */
  cmd = "rm " TARGET;
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("removing data with command '%s': OK\n", cmd);

#if 1
  /* become admin */
  cmd = "cd ADMIN";
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("ERROR: command '%s' failed!\n", cmd);
    goto do_return;

  } else printf("becoming admin with command '%s': OK\n", cmd);

  /* remove store (do not care whether it fails or not) */
  cmd = "rm storage_demo";
  if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    printf("WARNING: command '%s' failed!\n", cmd);

  } else printf("removing store with command '%s': OK\n", cmd);
#endif

  scdc_dataset_close(ds);

do_return:
  return 1;
}


int main(int argc, char *argv[])
{
  const char *uri = "scdc:/store";
  scdc_dataprov_t dp;


  printf("SCDC storage demo for C\n");

  scdc_init(SCDC_INIT_DEFAULT);

  // dp = scdc_dataprov_open("store", "store:stub");
  dp = scdc_dataprov_open("store", "store:fs", STORE_FS_CONF);
  // dp = scdc_dataprov_open("store", "store:mysql", STORE_MYSQL_CONF);
  // dp = scdc_dataprov_open("store", "store:webdav:no100continue", STORE_WEBDAV_CONF);
  // dp = scdc_dataprov_open("store", "store:webdav:username:password", STORE_WEBDAV_CONF, "foo", "bar");
  // dp = scdc_dataprov_open("store", "store:webdav:username:password", STORE_WEBDAV_CONF, "xxxdavtest", "dasisteintest");
  // dp = scdc_dataprov_open("store", "nfs:store", STORE_NFS_CONF);

  test_store(uri);

  scdc_dataprov_close(dp);

  scdc_release();

  return 0;
}
