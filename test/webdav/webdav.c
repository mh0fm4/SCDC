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
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
//#include "fs_example.h"

#include "scdc.h"

#define CHAR_BUFFER_SIZE 1024

void print_stat(struct stat sb) { //source: stat manpage 
    printf("File type : ");

    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK: printf("block device\n");
            break;
        case S_IFCHR: printf("character device\n");
            break;
        case S_IFDIR: printf("directory\n");
            break;
        case S_IFIFO: printf("FIFO / pipe\n");
            break;
        case S_IFLNK: printf("symlink\n");
            break;
        case S_IFREG: printf("regular file\n");
            break;
        case S_IFSOCK: printf("socket\n");
            break;
        default: printf("unknown ? \n");
            break;
    }

    //check permissions
    char r_owner[4], r_group[4], r_other[4];
    r_owner[0] = r_group[0] = r_other[0] = '-';
    r_owner[1] = r_group[1] = r_other[1] = '-';
    r_owner[2] = r_group[2] = r_other[2] = '-';
    r_owner[3] = r_group[3] = r_other[3] = '\0';

    //    S_IRUSR 00400 owner has read permission
    //    S_IWUSR 00200 owner has write permission
    //    S_IXUSR 00100 owner has execute permission
    if (sb.st_mode & S_IRUSR) {
        r_owner[0] = 'r';
    }
    if (sb.st_mode & S_IWUSR) {
        r_owner[1] = 'w';
    }
    if (sb.st_mode & S_IXUSR) {
        r_owner[2] = 'x';
    }

    //    S_IRGRP 00040 group has read permission
    //    S_IWGRP 00020 group has write permission
    //    S_IXGRP 00010 group has execute permission
    if (sb.st_mode & S_IRGRP) {
        r_group[0] = 'r';
    }
    if (sb.st_mode & S_IWGRP) {
        r_group[1] = 'w';
    }
    if (sb.st_mode & S_IXGRP) {
        r_group[2] = 'x';
    }

    //    S_IROTH 00004 others have read permission
    //    S_IWOTH 00002 others have write permission
    //    S_IXOTH 00001 others have execute permission
    if (sb.st_mode & S_IROTH) {
        r_other[0] = 'r';
    }
    if (sb.st_mode & S_IWOTH) {
        r_other[1] = 'w';
    }
    if (sb.st_mode & S_IXOTH) {
        r_other[2] = 'x';
    }

    printf("I - node number : %ld\n", (long) sb.st_ino);
    printf("Mode : %lo (octal) ", (unsigned long) sb.st_mode);
    printf("%s-%s-%s\n", r_owner, r_group, r_other);
    printf("Link count : %ld\n", (long) sb.st_nlink);
    printf("Ownership : UID = %ld GID = %ld\n", (long) sb.st_uid, (long) sb.st_gid);

    printf("Preferred I/O block size : %ld bytes\n", (long) sb.st_blksize);
    printf("File size : %lld bytes\n", (long long) sb.st_size);
    printf("Blocks allocated : %lld\n", (long long) sb.st_blocks);

    printf("Last status change : %s", ctime(&sb.st_ctime));
    printf("Last file access : %s", ctime(&sb.st_atime));
    printf("Last file modification : %s", ctime(&sb.st_mtime));
}

int main(int argc, char *argv[]) {

    const char *cmd;
    char cmd_as_char_array[CHAR_BUFFER_SIZE];
    scdc_dataprov_t dpDAV;
    scdc_dataset_t ds;
    scdc_dataset_input_t _ip, *ip = &_ip;
    scdc_dataset_output_t _op, *op = &_op;
   
    char buffer[CHAR_BUFFER_SIZE];

    int go_up = 1; //used to jump back to cmd_get_buffered after do_cmd_lseek

    char* conf = "https:webdav.magentacloud.de:eric.kunze@gmx.net:erku@mag2webdav";
    //char* conf = "http:localhost";


    printf("SCDC storage demo for C\n");

    scdc_init(SCDC_INIT_DEFAULT);

    dpDAV = scdc_dataprov_open("storeDAV", "webdav", conf);
    const char *uri = "scdc:/storeDAV";

    /* open dataset */
    ds = scdc_dataset_open(uri);
    if (ds == SCDC_DATASET_NULL) {
        printf("ERROR: open dataset failed!\n");
        goto quit;

    } else printf("open dataset '%s': OK\n", uri);

    /* create output object */
    op = scdc_dataset_output_create(op, "buffer", buffer, CHAR_BUFFER_SIZE);
    ip = scdc_dataset_output_create(ip, "none"); 
    //prevents segmentation fault with when goto quit is called because 
    //scdc_dataset_input_destroy(ip) calls 'output->intern' which can't be called before this
    
    /*****************   do_cmd_cd   **********************/

    cmd = "cd webdav";

    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! %s \n ", cmd, buffer);
        goto quit;

    } else {
        printf("changing directory with command '%s': OK\n", cmd);
    }

    /*****************   do_cmd_ls   **********************/

    cmd = "ls";

    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! %s \n ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;

    } else {
        printf("storing data with command '%s': OK\n", cmd);
        printf("%.*s\n", (int) SCDC_DATASET_INOUT_BUF_CURRENT(op), (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
    }


    /*****************   do_cmd_info   **********************/

    cmd = "info test.txt";

    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! %s \n ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;

    } else {
        //on success the output contains a 'struct stat' with the meta data of the requested file
        struct stat* info = (struct stat*) SCDC_DATASET_INOUT_BUF_PTR(op);

        printf("storing data with command '%s': OK\n\n", cmd);
        print_stat(*info);
        printf("\n");
    }

    /*****************   do_cmd_open   **********************/

    int mode = O_RDONLY;

    //convert the mode from integer to a string to use it in the parameter string
    char mode_as_char_array[CHAR_BUFFER_SIZE];
    snprintf(mode_as_char_array, CHAR_BUFFER_SIZE, "%d", mode);

    //build parameter string
    strncpy(cmd_as_char_array, "open karl_marx_kapital.txt ", CHAR_BUFFER_SIZE);
    strncat(cmd_as_char_array, mode_as_char_array, CHAR_BUFFER_SIZE); //result of that is "open karl_marx_kapital.txt 0"

    if (scdc_dataset_cmd(ds, cmd_as_char_array, NULL, op) != SCDC_SUCCESS) { //if no ip is given the file is opened with O_RDONLY by default
        printf("ERROR: command '%s' failed! -> %s ", cmd_as_char_array, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("opened file with command '%s': OK\n", cmd_as_char_array);
    }

    /*****************   do_cmd_get   **********************/

do_cmd_get_buffered:

    /* retrieve data from dataset to output object */
    /* File needs to be opened with do_cmd_open befor this! */
    cmd = "get";
    if (scdc_dataset_cmd(ds, cmd, op, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;

    } else {
        printf("reading first chunk with command '%s': OK\n", cmd);
    }

    int count = 1;

    while (SCDC_DATASET_INOUT_BUF_CURRENT(op) > 0 && count > 0) {
        printf("-------------------------------------------------------------------------------\n");
        printf("%.*s\n", (int) SCDC_DATASET_INOUT_BUF_CURRENT(op), (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        printf("-------------------------------------------------------------------------------\n");
        if (op->next) {
            op->next(op);
        }
        count--;
    }

    /*****************   do_cmd_lseek   **********************/

    scdcint_t offset = 0; //for the result (retrieved on success from output buffer)

    cmd = "lseek SEEK_CUR 0"; //current position with offset 0
    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;

    } else {
        printf("get file cursor with command '%s': OK\n", cmd);
        offset = *((scdcint_t*) SCDC_DATASET_INOUT_BUF_PTR(op));
        printf("current cursor position: %lld \n", offset);
    }

    //lets try cmd_get_buffered again with a new cursor
    if (go_up) {
        go_up = 0; // go up to do_cmd_get_buffered only once

        cmd = "lseek SEEK_SET 100"; //set cursor to byte 100 (from the beginning of the file)
        if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
            printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
            goto quit;

        } else {
            printf("set file cursor with command '%s': OK\n", cmd);
        }

        goto do_cmd_get_buffered;
    }

    /*****************   do_cmd_close   **********************/

    cmd = "close";
    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("closing file with command '%s': OK\n", cmd);
    }

    /*****************   do_cmd_put   **********************/
cmd_put:

    mode = O_RDWR | O_APPEND | O_CREAT; //read and write, create if not exsiting or append if existing

    snprintf(mode_as_char_array, CHAR_BUFFER_SIZE, "%d", mode);

    //build parameter string
    strncpy(cmd_as_char_array, "open file_for_writing.txt ", CHAR_BUFFER_SIZE);
    strncat(cmd_as_char_array, mode_as_char_array, CHAR_BUFFER_SIZE);

    if (scdc_dataset_cmd(ds, cmd_as_char_array, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd_as_char_array, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("opened file with command '%s': OK\n", cmd_as_char_array);
    }

    //writing file
    char input_buffer[CHAR_BUFFER_SIZE];
    ip = scdc_dataset_input_create(ip, "buffer", input_buffer, CHAR_BUFFER_SIZE);
    
    strncpy(input_buffer, "This is a test text. \n", CHAR_BUFFER_SIZE); //22 byte
    SCDC_DATASET_INOUT_BUF_CURRENT(ip) = strlen(input_buffer);

    cmd = "put";
    if (scdc_dataset_cmd(ds, cmd, ip, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s \n", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("writing file with command '%s': OK\n", cmd);
    }

    strncpy(input_buffer, "This is the second line \n", CHAR_BUFFER_SIZE); //22 byte
    SCDC_DATASET_INOUT_BUF_CURRENT(ip) = strlen(input_buffer);

    cmd = "put";
    if (scdc_dataset_cmd(ds, cmd, ip, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s \n", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("writing file with command '%s': OK\n", cmd);
    }

    //close opened file    
    cmd = "close";
    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("closing file with command '%s': OK\n", cmd);
    }

    ///////////////////////////////////////////////////////////////////

    //check the file on the server and compare its content
    cmd = "open file_for_writing.txt 0"; // 0 -> O_RDONLY

    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("opened file with command '%s': OK\n", cmd);
    }

    char *str = "This is a test text. \nThis is the second line \n";

    cmd = "get";
    if (scdc_dataset_cmd(ds, cmd, op, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;

    } else {
        printf("reading first chunk with command '%s': OK\n", cmd);
    }

    if (SCDC_DATASET_INOUT_BUF_CURRENT(op) > 0) {

        int len_buf = CHAR_BUFFER_SIZE;
        if (SCDC_DATASET_INOUT_BUF_CURRENT(op) < CHAR_BUFFER_SIZE) {
            buffer[SCDC_DATASET_INOUT_BUF_CURRENT(op)] = '\0';
            len_buf = strlen(buffer);
        }

        int len_str = strlen(str);

        if (len_str == len_buf && strncmp(str, buffer, strlen(str)) == 0) {
            printf("compare written file: OK\n");
        } else {
            printf("compare written file: failed! two differnt strings: \n");
            printf("expected: |%s| length: %d\n", str, len_str);
            printf("file content: |%s| length: %d\n", buffer, len_buf);
        }
    }

    cmd = "close";
    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("closing file with command '%s': OK\n", cmd);
    }

    /*****************   do_cmd_rm   **********************/

    cmd = "rm file_for_writing.txt";
    if (scdc_dataset_cmd(ds, cmd, NULL, op) != SCDC_SUCCESS) {
        printf("ERROR: command '%s' failed! -> %s ", cmd, (char*) SCDC_DATASET_INOUT_BUF_PTR(op));
        goto quit;
    } else {
        printf("deleting file with command '%s': OK\n", cmd);
    }

quit:

    scdc_dataset_output_destroy(op); //FIXME segmentation fault if no output was set before
    scdc_dataset_input_destroy(ip); //FIXME segmentation fault if no input was set before
    scdc_dataset_close(ds);
    scdc_dataprov_close(dpDAV);

    scdc_release();
    
    return 0;
}
