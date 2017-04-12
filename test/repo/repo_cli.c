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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <getopt.h>

#include "z_pack.h"

#include "scdc.h"

#include "repoH.h"


const char *prefix = "repo_cli: ";


void sighandler(int sig)
{
  printf("sighandler: sig: %d\n", sig);

  fclose(stdin);
}


scdcint_t manual_input_next(scdc_dataset_input_t *input)
{
  scdcint_t i = (intptr_t) input->data;

  printf("INPUT #%" scdcint_fmt " + <Enter>: ", i);

  ((char *) SCDC_DATASET_INOUT_BUF_PTR(input))[0] = 0;

  fgets(SCDC_DATASET_INOUT_BUF_PTR(input), SCDC_DATASET_INOUT_BUF_SIZE(input), stdin);

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = strlen(SCDC_DATASET_INOUT_BUF_PTR(input));

  if (!feof(stdin)) --SCDC_DATASET_INOUT_BUF_CURRENT(input);

  input->total_size += SCDC_DATASET_INOUT_BUF_CURRENT(input);

  if (feof(stdin) || SCDC_DATASET_INOUT_BUF_CURRENT(input) <= 0) input->next = NULL;

  input->data = (void *) (intptr_t) (i + 1);

  return SCDC_SUCCESS;
}


scdcint_t manual_output_next(scdc_dataset_output_t *output)
{
  scdcint_t i = (intptr_t) output->data;

  if (strcmp(output->format, "text") == 0 || strcmp(output->format, "fslist") == 0)
    printf("OUTPUT #%" scdcint_fmt ": '%.*s'\n", i, (int) SCDC_DATASET_INOUT_BUF_CURRENT(output), (char *) SCDC_DATASET_INOUT_BUF_PTR(output));
  else
    printf("OUTPUT #%" scdcint_fmt": %" scdcint_fmt " bytes\n", i, SCDC_DATASET_INOUT_BUF_CURRENT(output));

  output->data = (void *) (intptr_t) (i + 1);

  return SCDC_SUCCESS;
}



#define DEFAULT_INPUT_BUF_SIZE   256
#define DEFAULT_OUTPUT_BUF_SIZE  256


static char default_input_buf[DEFAULT_INPUT_BUF_SIZE], default_output_buf[DEFAULT_OUTPUT_BUF_SIZE];


void prepare_dataset_cmd(const char *cmdline, char *cmd, scdc_dataset_input_t **input, int *input_type, scdc_dataset_output_t **output, int *output_type)
{
  char *input_pos, *output_pos, *name_pos, *type_pos;


  input_pos = strchr(cmdline, '<');
  output_pos = strchr(cmdline, '>');

  if (input_pos) { *input_pos = '\0'; ++input_pos; }
  if (output_pos) { *output_pos = '\0'; ++output_pos; }

  *input_type = 0;

  if (input_pos)
  {
    *input_type = 1;

    name_pos = strchr(input_pos, ':');

    if (name_pos) { *name_pos = '\0'; ++name_pos; type_pos = input_pos; }
    else { name_pos = input_pos; type_pos = NULL; }

    if (!type_pos)
    {
      if (strlen(name_pos) > 0)
      {
        if (z_fs_is_directory(name_pos)) type_pos = "fs";
        else if (z_fs_is_file(name_pos)) type_pos = "file";
        else type_pos = "text";

      } else type_pos = "manual";
    }

    if (strcmp(type_pos, "fs") == 0)
    {
      printf("input fs: '%s'\n", name_pos);
      *input = scdc_dataset_input_create(*input, "fs", ".", name_pos);

    } else if (strcmp(type_pos, "file") == 0)
    {
      printf("input file: '%s'\n", name_pos);
      *input = scdc_dataset_input_create(*input, "file", name_pos);

    } else if (strcmp(type_pos, "stream") == 0)
    {
      printf("input stream: '%s'\n", name_pos);
      *input = scdc_dataset_input_create(*input, "stream", stdin);

    } else if (strcmp(type_pos, "text") == 0)
    {
      printf("input text: '%s'\n", name_pos);
      *input = scdc_dataset_input_create(*input, "buffer", name_pos, strlen(name_pos));

      strcpy((*input)->format, "text");

    } else
    {
      printf("input manual:\n");

      scdc_dataset_input_unset(*input);

      strcpy((*input)->format, "text");

      SCDC_DATASET_INOUT_BUF_PTR(*input) = default_input_buf;
      SCDC_DATASET_INOUT_BUF_SIZE(*input) = DEFAULT_INPUT_BUF_SIZE;

      SCDC_DATASET_INOUT_BUF_CURRENT(*input) = 0;
      (*input)->total_size = 0;
      (*input)->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST;

      (*input)->next = manual_input_next;

      (*input)->data = (void *) 0;

      *input_type = -1;
    }

  } else
  {
    printf("input null:\n");
    *input = NULL;
  }


  *output_type = 0;

  if (output_pos)
  {
    *output_type = 1;

    name_pos = strchr(output_pos, ':');

    if (name_pos) { *name_pos = '\0'; ++name_pos; type_pos = output_pos; }
    else { name_pos = output_pos; type_pos = NULL; }

    if (!type_pos) type_pos = "redirect";

    if (strcmp(type_pos, "fs") == 0)
    {
      printf("output fs: '%s'\n", name_pos);
      *output = scdc_dataset_output_create(*output, "fs", name_pos);

    } else if (strcmp(type_pos, "file") == 0)
    {
      printf("output file: '%s'\n", name_pos);
      *output = scdc_dataset_output_create(*output, "file", name_pos);

    } else if (strcmp(type_pos, "stream") == 0)
    {
      printf("output stream: '%s'\n", name_pos);
      *output = scdc_dataset_output_create(*output, "stream", stdout);

    } else if (strcmp(type_pos, "consume") == 0)
    {
      printf("output consume:\n");
      *output = scdc_dataset_output_create(*output, "consume");

    } else if (strcmp(type_pos, "redirect") == 0)
    {
      printf("output redirect:\n");
      *output = scdc_dataset_output_create(*output, "alloc");
      *output_type = 2;

    } else
    {
      printf("output manual:\n");

      scdc_dataset_output_unset(*output);

      SCDC_DATASET_INOUT_BUF_PTR(*output) = default_output_buf;
      SCDC_DATASET_INOUT_BUF_SIZE(*output) = DEFAULT_OUTPUT_BUF_SIZE;

      (*output)->next = manual_output_next;

      (*output)->data = (void *) 0;

      *output_type = -1;
    }

  } else
  {
    printf("output loop manual:\n");

    scdc_dataset_output_unset(*output);

    SCDC_DATASET_INOUT_BUF_PTR(*output) = default_output_buf;
    SCDC_DATASET_INOUT_BUF_SIZE(*output) = DEFAULT_OUTPUT_BUF_SIZE;
  }

  strcpy(cmd, cmdline);
}


void unprepare_dataset_cmd(scdc_dataset_input_t *input, int input_type, scdc_dataset_output_t *output, int output_type)
{
  scdcint_t i;


  printf("cmd input: ");
  scdc_dataset_input_print(input);
  printf("\n");

  if (input_type > 0)
  {
    scdc_dataset_input_destroy(input);

  } else if (input)
  {
    /* manual */
  }

  printf("cmd output: ");
  scdc_dataset_output_print(output);
  printf("\n");

  if (output_type > 0)
  {
    if (output_type == 2) scdc_dataset_output_redirect(output, "stream", stdout);

    scdc_dataset_output_destroy(output);

  } else if (output)
  {
    /* manual */
    if (output_type == 0)
    {
      i = 0;
      do {
        void *data = output->data;
        output->data = (void *) (intptr_t) i;

        manual_output_next(output);
#if 0
        break;
#endif

        i = (intptr_t) output->data;
        output->data = data;

        if (!output->next) break;

#if 1
        printf("Press <Enter>");
        getchar();
#endif

        output->next(output);

      } while (1);
    }
  }
}


void run_cli(scdc_dataset_t dataset, char *cmdline)
{
  int cmdline_given;
  size_t cmdline_size = 256;
  ssize_t n;

#define CMD_SIZE  256
  char cmd[CMD_SIZE];
  scdc_dataset_input_t _input, *input, _output, *output;
  int input_type, output_type;


  cmdline_given = (cmdline && strlen(cmdline) > 0);

  if (!cmdline_given) cmdline = malloc(cmdline_size);

  while (1)
  {
    if (!cmdline_given)
    {
      printf("Enter command for dataset '%p': ", dataset);
      n = getline(&cmdline, &cmdline_size, stdin);

    } else n = strlen(cmdline);

    if (n > 0 && cmdline[n - 1] == '\n') 
    {
      cmdline[n - 1] = '\0';
      --n;
    }

    if (n < 0 || feof(stdin) || strcmp(cmdline, "quit") == 0) break;

    if (n == 0) continue;

    input = &_input;
    output = &_output;

    prepare_dataset_cmd(cmdline, cmd, &input, &input_type, &output, &output_type);

    scdcint_t ret = scdc_dataset_cmd(dataset, cmd, input, output);

    unprepare_dataset_cmd(input, input_type, output, output_type);

    printf("%scommand '%s' %s!\n", prefix, cmd, ((ret == SCDC_SUCCESS)?"success":"failed"));

    if (cmdline_given) break;
  }

  if (!cmdline_given) free(cmdline);
}


int main(int argc, char *argv[])
{
#define DATAPROVS_MAX  16
  int ndataprovs = 0;
  scdc_dataprov_t dataprovs[DATAPROVS_MAX];

  scdc_dataset_t dataset;

#define OPTARG_MAX  256
  char scheme[OPTARG_MAX], authority[OPTARG_MAX], open[OPTARG_MAX], cmdline[OPTARG_MAX];
  char path[OPTARG_MAX], *e;


  printf("start repository client\n");

  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);

#define SETUP  0

#if SETUP == 0
  strcpy(scheme, "scdc");
  strcpy(authority, "");
  strcpy(open, "repoA");
  strcpy(cmdline, "ls>stream:");
#elif SETUP == 1
  strcpy(scheme, "scdc+tcp");
  strcpy(authority, "localhost");
  strcpy(open, "repoA");
  strcpy(cmdline, "");
#elif SETUP == 2
  strcpy(scheme, "scdc");
  strcpy(authority, "");
  strcpy(open, "repoB/xxx");
  strcpy(cmdline, "");
#elif SETUP == 3
  strcpy(scheme, "scdc+tcp");
  strcpy(authority, "localhost");
  strcpy(open, "repoD/");
  strcpy(cmdline, "");
#elif SETUP == 4
  strcpy(scheme, "scdc+tcp");
  strcpy(authority, "localhost");
  strcpy(open, "www/");
  strcpy(cmdline, "");
#elif SETUP == 5
  strcpy(scheme, "scdc+tcp");
  strcpy(authority, "patty");
  strcpy(open, "www");
  strcpy(cmdline, "");
#elif SETUP == 6
  strcpy(scheme, "scdc");
  strcpy(authority, "");
  strcpy(open, "repoD");
  strcpy(cmdline, "");
#elif SETUP == 7
  strcpy(scheme, "scdc+tcp");
  strcpy(authority, "localhost");
  strcpy(open, "repoH");
  strcpy(cmdline, "");
#elif SETUP == 8
  strcpy(scheme, "scdc");
  strcpy(authority, "");
  strcpy(open, "repoC");
  strcpy(cmdline, "");
#elif SETUP == 9
  strcpy(scheme, "scdc");
  strcpy(authority, "");
  strcpy(open, "repoJ");
  strcpy(cmdline, "");
#endif

#if 0
  strcpy(scheme, "scdc+mpi");
  strcpy(authority, "publ:TESTNAME:0");
  strcpy(open, "repoA/");
  strcpy(cmdline, "");
#endif

#if 1
  strcpy(scheme, "");
  strcpy(authority, "");
  strcpy(open, "");
/*  strcpy(cmdline, "scdc+tcp://localhost/repoR/relA ls");*/
  strcpy(cmdline, "scdc+uds:///repoA ls");
#endif

/*  scdc_init(SCDC_INIT_DEFAULT ":log_filepath", "scdclog");*/
  scdc_init(SCDC_INIT_DEFAULT);

#if 1
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoA", "fs_access", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "A"), path));
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoB", "fs_access", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "B"), path));
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoC", "gen");
/*  dataprovs[ndataprovs++] = scdc_dataprov_open("repoD", "mysql", getenv("MERGE_SCDC_MYSQL_CREDENTIALS"));*/
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoH", "hook:id", &repoH, 0x2501);
/*  dataprovs[ndataprovs++] = scdc_dataprov_open("repoJ", "jobrun", "uname -a; sleep 1", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "J"), path));
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoR", "jobrun_relay:relay:relay", "relA scdc:///repoA", "relB scdc:///repoB");*/
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoS", "fs_store", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "S"), path));
#endif

#if 0
  static struct option long_options[] =
  {
    {"scheme",   required_argument, 0, 's'},
    {"authority",     required_argument, 0, 'h'},
    {"dataprov", required_argument, 0, 'd'},
    {"open",     required_argument, 0, 'o'},
    {"cmd",      required_argument, 0, 'c'},
    {0, 0, 0, 0}
  };

  while (1)
  {
    int option_index = 0;

    int c = getopt_long(argc, argv, "s:h:d:o:c:", long_options, &option_index);

    if (c == -1) break;

    char p[256], *s;

    switch (c)
    {
      case 's':
        if (strcmp(optarg, "scdc+tcp") == 0) strncpy(scheme, "scdc+tcp", OPTARG_MAX);
        else if (strcmp(optarg, "scdc+uds") == 0) strncpy(scheme, "scdc+uds", OPTARG_MAX);
        else strncpy(scheme, "scdc", OPTARG_MAX);
        break;
      case 'a':
        strncpy(authority, optarg, OPTARG_MAX);
        break;
      case 'd':
        s = strchr(optarg, ':');
        s[0] = 0;
        ++s;
        sprintf(p, "%s%s", getenv("MERGE_SCDC_REPO_PATH"), s);
        dataprovs[ndataprovs++] = scdc_dataprov_open(optarg, p);
        break;
      case 'o':
        strncpy(open, optarg, OPTARG_MAX);
        break;
      case 'c':
        strncpy(cmdline, optarg, OPTARG_MAX);
        break;
      default:
        printf("unknown parameter: '%c'\n", c);
      }
  }

  if (strlen(scheme) > 0 || strlen(authority) > 0 || strlen(open) > 0)
  {
    char uri[1024];

    if (strlen(authority) > 0) sprintf(uri, "%s://%s/%s", scheme, authority, open);
    else sprintf(uri, "%s:/%s", scheme, open);

    printf("%suri: '%s'\n", prefix, uri);

    dataset = scdc_dataset_open(uri);

  } else dataset = SCDC_DATASET_NULL;

  printf("%sdataset: %p\n", prefix, dataset);

  run_cli(dataset, cmdline);

  scdc_dataset_close(dataset);
#endif

  while (ndataprovs > 0) scdc_dataprov_close(dataprovs[--ndataprovs]);

  scdc_release();

  printf("quit repository client\n");

  return 0;
}
