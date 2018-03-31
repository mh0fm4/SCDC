/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
 *  Copyright (C) 2017 Thomas Weber
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


/*
 * This demo application has been inspired by the filesystem demo.
 */


#include "scdc.h"

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>


/* Default size for buffers */
#define DATASET_BUFFER_SIZE 512

// XXX: NFS share is hardcoded, assuming a 192.168.2.x IP address and this
// /etc/exports entry:
// /srv/nfs/test_share 192.168.2.0/24(insecure,rw,no_subtree_check)
/*#define NFS_DEMO_MOUNT_PATH "nfs://192.168.2.103/srv/nfs/test_share/"*/
#define NFS_DEMO_MOUNT_PATH "nfs://127.0.0.1/srv/nfs_test_share/"


/* Helper to leave the current function if the assertion fails */
#define DEMO_TEST(stmt) \
  do { \
    if (!(stmt)) \
    { \
      fprintf(stderr, "error: test failed: '%s'\n", #stmt); \
      _free_output_buffer(print_buf); \
      return false; \
    } \
  } while (0)

/* Helper to leave the current function if the assertion fails */
#define DEMO_TEST_GOTO(goto_target, stmt) \
  do { \
    if (!(stmt)) \
    { \
      fprintf(stderr, "error: test failed: '%s'\n", #stmt); \
      goto goto_target; \
    } \
  } while (0)

/* Expands a dataset output to two arguments for the '%.*s' format */
#define DATASET_OUTPUT_PRINTF_ARGS(x) \
  (int)SCDC_DATASET_INOUT_BUF_CURRENT(x), (const char*)SCDC_DATASET_INOUT_BUF_PTR(x)


static scdc_dataset_output_t *_create_output_buffer(size_t size)
{
  assert(size != 0);

  /* Round data structure size up to a multiple of 16 bytes */
  size_t dataset_struct_size = sizeof(scdc_dataset_output_t);
  dataset_struct_size = (dataset_struct_size + 15u) & ~15u;

  /* Allocate destination buffer, plus space to store the structure to free
     both at once */
  unsigned char *buffer_ptr = malloc(dataset_struct_size + size);
  if (!buffer_ptr)
    return NULL;

  /* Fill in the structure */
  scdc_dataset_output_t *dataset_out = (scdc_dataset_output_t *)buffer_ptr;
  scdc_dataset_output_unset(dataset_out);
  SCDC_DATASET_INOUT_BUF_SIZE(dataset_out) = size;
  SCDC_DATASET_INOUT_BUF_PTR(dataset_out) = &buffer_ptr[dataset_struct_size];
  return dataset_out;
}


static void _free_output_buffer(scdc_dataset_output_t *dataset_out)
{
  if (dataset_out == NULL)
    return;

  /* Discard results of additional partial data transfers */
  while (dataset_out->next)
    dataset_out->next(dataset_out);

  /* Free storage, the data buffer is in the same allocation */
  free(dataset_out);
}


static bool _run_admin_demo(scdc_dataset_t dataset)
{
  scdc_dataset_output_t *print_buf = NULL;

  /* Switch to admin mode */
  DEMO_TEST(scdc_dataset_cmd(dataset, "cd ADMIN", NULL, NULL) == SCDC_SUCCESS);

  /* Create output buffer */
  print_buf = _create_output_buffer(DATASET_BUFFER_SIZE);
  DEMO_TEST(print_buf != NULL);

  /* Print number of stores within this share (there are none for now) */
  DEMO_TEST(scdc_dataset_cmd(dataset, "info", NULL, print_buf) == SCDC_SUCCESS);
  printf("%.*s\n", DATASET_OUTPUT_PRINTF_ARGS(print_buf));

  /* Print empty store list */
  DEMO_TEST(scdc_dataset_cmd(dataset, "ls", NULL, print_buf) == SCDC_SUCCESS);
  printf("%.*s\n", DATASET_OUTPUT_PRINTF_ARGS(print_buf));

  /* Create two stores */
  DEMO_TEST(scdc_dataset_cmd(dataset, "put teststore1", NULL, NULL) == SCDC_SUCCESS);
  DEMO_TEST(scdc_dataset_cmd(dataset, "put teststore2", NULL, NULL) == SCDC_SUCCESS);

  /* This command should fail */
  DEMO_TEST(scdc_dataset_cmd(dataset, "get teststore2", NULL, NULL) != SCDC_SUCCESS);

  /* We only need one store for the test (teststore2 is empty removal test).
     It should not succeed a second time. */
  DEMO_TEST(scdc_dataset_cmd(dataset, "rm teststore2", NULL, NULL) == SCDC_SUCCESS);
  DEMO_TEST(scdc_dataset_cmd(dataset, "rm teststore2", NULL, NULL) != SCDC_SUCCESS);

  /* Clean up */
  _free_output_buffer(print_buf);
  return true;
}


static bool _run_files_demo(scdc_dataset_t dataset)
{
  scdc_dataset_output_t *print_buf = NULL;

  /* Switch to normal mode */
  DEMO_TEST(scdc_dataset_cmd(dataset, "cd teststore1", NULL, NULL) == SCDC_SUCCESS);

  /* Create output buffer */
  print_buf = _create_output_buffer(DATASET_BUFFER_SIZE);
  DEMO_TEST(print_buf != NULL);

  /* Create an input from a small file */
  scdc_dataset_input_t small_file_input, *small_file_input_p = &small_file_input;
  small_file_input_p = scdc_dataset_input_create(small_file_input_p,
    "file", "test_inputs/small_file");
  DEMO_TEST(small_file_input_p != NULL);

  /* Transfer it to the NFS share */
  DEMO_TEST_GOTO(error_input1, scdc_dataset_cmd(dataset, "put testsmall_file", small_file_input_p, NULL) == SCDC_SUCCESS);

  /* Print file list, it should contain our file as well as its file size. */
  DEMO_TEST_GOTO(error_input1, scdc_dataset_cmd(dataset, "ls", NULL, print_buf) == SCDC_SUCCESS);
  printf("%.*s\n", DATASET_OUTPUT_PRINTF_ARGS(print_buf));

  /* Oops. I accidentally put another copy of it in there (should overwrite) */
  DEMO_TEST_GOTO(error_input1, scdc_dataset_cmd(dataset, "put testsmall_file", small_file_input_p, NULL) == SCDC_SUCCESS);

  /* Oops. I accidentally put another copy of it in there, with a different
     filename. I'm totally sorry, will undo this in a moment. */
  DEMO_TEST_GOTO(error_input1, scdc_dataset_cmd(dataset, "put testsmall_file_dup", small_file_input_p, NULL) == SCDC_SUCCESS);
  DEMO_TEST_GOTO(error_input1, scdc_dataset_cmd(dataset, "rm testsmall_file_dup", NULL, NULL) == SCDC_SUCCESS);

  /* Delete the input */
  scdc_dataset_input_destroy(small_file_input_p);

  /* Use a circa 3000 bytes large test file to create a file exceeding
     DEFAULT_DATASET_INOUT_BUF_SIZE (16 MiB), to test partial transfers. */
  FILE *large_file = fopen("test_inputs/large_file", "r");
  if (large_file == NULL)
  {
    perror("test_inputs/large_file");
    goto error;
  }
  char large_file_buffer[3000];
  size_t large_file_buffer_length = fread(large_file_buffer, 1,
    sizeof(large_file_buffer), large_file);
  if (large_file_buffer_length < 2000)
  {
    perror("test_inputs/large_file read not long enough");
    fclose(large_file);
    goto error;
  }
  fclose(large_file);

  FILE *large_temp_file = fopen("test_inputs/large_file.big", "w");
  if (large_temp_file == NULL)
  {
    perror("test_inputs/large_file.big");
    goto error;
  }

  size_t total_written_bytes = 0u;
  while (total_written_bytes < 17 * 1048576)
  {
    if (fwrite(large_file_buffer, large_file_buffer_length, 1, large_temp_file) == 0)
    {
      perror("writing test_inputs/large_file.big failed");
      fclose(large_temp_file);
      goto error;
    }
    total_written_bytes += large_file_buffer_length;
  }
  fclose(large_temp_file);

  /* With the file prepared, perform a large write and close the file. */
  scdc_dataset_input_t large_file_input, *large_file_input_p = &large_file_input;
  large_file_input_p = scdc_dataset_input_create(large_file_input_p,
    "file", "test_inputs/large_file.big");
  DEMO_TEST(large_file_input_p != NULL);

  DEMO_TEST_GOTO(error_input2, scdc_dataset_cmd(dataset, "put testlarge_file", large_file_input_p, NULL) == SCDC_SUCCESS);

  scdc_dataset_input_destroy(large_file_input_p);

  /* Print file list, it should two files with their file sizes. */
  DEMO_TEST(scdc_dataset_cmd(dataset, "ls", NULL, print_buf) == SCDC_SUCCESS);
  printf("%.*s\n", DATASET_OUTPUT_PRINTF_ARGS(print_buf));

  /* Create an output from the large file we transferred before */
  scdc_dataset_input_t large_file_output, *large_file_output_p = &large_file_output;
  large_file_output_p = scdc_dataset_output_create(large_file_output_p,
    "file", "test_inputs/large_file.out");
  DEMO_TEST(large_file_output_p != NULL);

  /* Extract an arbitrary segment out of the large file (offset = 524288,
     size = 376006) */
  DEMO_TEST_GOTO(error_input3, scdc_dataset_cmd(dataset, "get testlarge_file 524288F:376006", NULL, large_file_output_p) == SCDC_SUCCESS);

  /* Delete the input */
  scdc_dataset_output_destroy(large_file_output_p);

  /*
   * Clean up everything done in this demo:
   * - Remove teststore1 from the NFS share
   * - Remove large_file.{big,out}
   */
  DEMO_TEST(scdc_dataset_cmd(dataset, "cd ADMIN", NULL, NULL) == SCDC_SUCCESS);
  DEMO_TEST(scdc_dataset_cmd(dataset, "rm teststore1", NULL, NULL) == SCDC_SUCCESS);

  unlink("test_inputs/large_file.big");
  unlink("test_inputs/large_file.out");

  _free_output_buffer(print_buf);
  return true;

error_input3:
  scdc_dataset_output_destroy(large_file_output_p);
  goto error;

error_input2:
  scdc_dataset_input_destroy(large_file_input_p);
  goto error;

error_input1:
  scdc_dataset_input_destroy(small_file_input_p);

error:
  _free_output_buffer(print_buf);
  return false;
}


static bool _run_demo(scdc_dataset_t dataset)
{
  /*
   * Begin the demo in "admin" mode, in which we can create and delete
   * subdirectories on the NFS share.
   */
  if (!_run_admin_demo(dataset))
  {
    fprintf(stderr, "error: admin demo failed\n");
    return false;
  }

  /*
   * With all "admin" tests performed, operate on files.
   */
  if (!_run_files_demo(dataset))
  {
    fprintf(stderr, "error: admin demo failed\n");
    return false;
  }

  /* All tests succeeded */
  printf("All demos successfully performed.\n");
  return true;
}


int main(int argc, char *argv[])
{
  int ret = 1;


  printf(
    "SCDC (lib)nfs demo\n"
    "Using NFS share %s\n"
    "Plase make sure that the NFS share the demo runs on:\n"
    " - is empty (no files or subdirectories present)\n"
    " - is accessible from this computer\n",
    NFS_DEMO_MOUNT_PATH
  );

  if (scdc_init(SCDC_INIT_DEFAULT) != SCDC_SUCCESS)
  {
    fprintf(stderr, "error: scdc_init() failed\n");
    return 1;
  }

  /*
   * Use the NFS data provider to mount a read-write NFS share hosted at
   * NFS_DEMO_MOUNT_PATH, to be made available through the SCDC library using
   * the (local) dataset path 'scdc:/nfs_demo'.
   */
  scdc_dataprov_t nfs_demo_dataprov = scdc_dataprov_open("nfs_demo", "nfs",
    NFS_DEMO_MOUNT_PATH);
  if (nfs_demo_dataprov == SCDC_DATAPROV_NULL)
  {
    fprintf(stderr, "error: scdc_dataprov_open('%s') failed\n",
      NFS_DEMO_MOUNT_PATH);
    goto exit_no_dataprov;
  }

  /*
   * Open the dataset path; this way, we can access the filesystem on the NFS
   * share mounted earlier.
   * XXX: use macro if this works
   */
  scdc_dataset_t nfs_demo_dataset = scdc_dataset_open("scdc:/nfs_demo");
  if (nfs_demo_dataset == SCDC_DATASET_NULL)
  {
    fprintf(stderr, "error: scdc_dataset_open('%s') failed\n",
      "scdc:/nfs_demo");
    goto exit_no_dataset;
  }

  /* Begin the demo */
  ret = _run_demo(nfs_demo_dataset) ? 0 : 1;

  /*
   * Perform cleanup operations
   */
  scdc_dataset_close(nfs_demo_dataset);

exit_no_dataset:
  scdc_dataprov_close(nfs_demo_dataprov);

exit_no_dataprov:
  scdc_release();

  return ret;
}

/* vi: set shiftwidth=2 tabstop=2 expandtab: */
