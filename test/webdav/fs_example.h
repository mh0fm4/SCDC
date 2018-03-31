/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
 *  Copyright (C) 2017, 2018 Eric Kunze
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
#define INPUT_FILE   TARGET ".in"
#define OUTPUT_FILE  TARGET ".out"

void run_fs_cmd_ls_example() {
    const char *uri = "scdc:/storeFS";

    const char *cmd;
    char path[256], *e;
    scdc_dataprov_t dpFS;
    scdc_dataset_t ds;
    scdc_dataset_input_t _ip, *ip = &_ip;
    scdc_dataset_output_t _op, *op = &_op;


    printf("SCDC storage demo for C\n");

    scdc_init(SCDC_INIT_DEFAULT);

    dpFS = scdc_dataprov_open("storeFS", "fs", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e ? e : ""), "/home"), path));


    /* open dataset */
    ds = scdc_dataset_open(uri);
    if (ds == SCDC_DATASET_NULL) {
        printf("ERROR: open dataset failed!\n");
        goto quit;

    } else printf("open dataset '%s': OK\n", uri);

    cmd = "ls eric";
    
    /* create output object to write to file */
    op = scdc_dataset_output_create(op, "buffer", OUTPUT_FILE);
    if (op == NULL) {
        printf("ERROR: open output file failed!\n");
        goto quit;
    }
    
    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed!\n", cmd);
        goto quit;

    } else {
        printf("storing data with command '%s': OK\n", cmd);
        printf("size op: %lld\n", op->total_size);
        printf("%s\n", (const char*)op->buf);
    }

//    
//    /* create input object to read from file */
//    ip = scdc_dataset_input_create(ip, "file", INPUT_FILE);
//    if (ip == NULL) {
//        printf("ERROR: open input file failed!\n");
//        goto quit;
//    }
//
//    /* store data from input object to dataset */
//    cmd = "put " TARGET;
//    if (scdc_dataset_cmd(ds, cmd, ip, NULL) != SCDC_SUCCESS) {
//        printf("ERROR: command '%s' failed!\n", cmd);
//        goto quit;
//
//    } else printf("storing data with command '%s': OK\n", cmd);
//
//    /* destroy input object */
//    scdc_dataset_input_destroy(ip);
//
//    /* create output object to write to file */
//    op = scdc_dataset_output_create(op, "file", OUTPUT_FILE);
//    if (op == NULL) {
//        printf("ERROR: open output file failed!\n");
//        goto quit;
//    }
//
//    /* retrieve data from dataset to output object */
//    cmd = "get " TARGET;
//    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
//        printf("ERROR: command '%s' failed!\n", cmd);
//        goto quit;
//
//    } else printf("retrieving data with command '%s': OK\n", cmd);
//
//    /* destroy output object */
//    scdc_dataset_output_destroy(op);
//
//    /* remove the data stored */
//    cmd = "rm " TARGET;
//    if (scdc_dataset_cmd(ds, cmd, NULL, NULL) != SCDC_SUCCESS) {
//        printf("ERROR: command '%s' failed!\n", cmd);
//        goto quit;
//
//    } else printf("removing data with command '%s': OK\n", cmd);

quit:
    scdc_dataset_close(ds);
    scdc_dataprov_close(dpFS);

    scdc_release();
}
