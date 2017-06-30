#!/usr/bin/python

#  
#  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
#  
#  This file is part of the Simulation Component and Data Coupling (SCDC) library.
#  
#  The SCDC library is free software: you can redistribute it and/or
#  modify it under the terms of the GNU Lesser Public License as published
#  by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  The SCDC library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser Public License for more details.
#  
#  You should have received a copy of the GNU Lesser Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#  


import os
import sys
import scdc


#print("path: ", sys.path)

STORE_FS_PATH = os.getenv("MERGE_SCDC_REPO_PATH", "") + "store/"
STORE_MYSQL_CREDENTIALS = os.getenv("MERGE_SCDC_MYSQL_CREDENTIALS", "")


dbstore = False
target = "store_test"
input_file = target + ".in"
output_file = target + ".out"


print("SCDC storage demo for Python");

scdc.init()

dpFS = scdc.dataprov_open("storeFS", "fs", STORE_FS_PATH)
dpDB = scdc.dataprov_open("storeDB", "mysql", STORE_MYSQL_CREDENTIALS)

uri = "scdc:/storeDB" if dbstore else "scdc:/storeFS"


# the loop is only used to be able to break if an error occurs
while True:

  # open dataset
  ds = scdc.dataset_open(uri)
  if not ds:
    print("ERROR: open dataset failed!")
    break
  else:
    print("open dataset '" + uri + "': OK")

  if dbstore:
    # become admin on db service
    cmd = "cd admin"
    if not scdc.dataset_cmd(ds, cmd, None, None):
      print("ERROR: command '" + cmd +"' failed!")
      break
    else:
      print("becoming admin with command '" + cmd + "': OK")

    # remove storage table (do not care whether fails or not)
    cmd = "rm storage_demo"
    scdc.dataset_cmd(ds, cmd, None, None)

    # add storage table
    cmd = "put storage_demo"
    if not scdc.dataset_cmd(ds, cmd, None, None):
      print("ERROR: command '" + cmd +"' failed!")
      break
    else:
      print("adding data base storage table with command '" + cmd + "': OK")

    # add storage table
    cmd = "cd storage_demo"
    if not scdc.dataset_cmd(ds, cmd, None, None):
      print("ERROR: command '" + cmd +"' failed!")
      break
    else:
      print("selecting data base storage table with command '" + cmd + "': OK")

  # create input object to read from file
  ip = scdc.dataset_input_create("file", input_file)
  if not ip:
    print("ERROR: open input file failed!")
    break
    
  # store data from input object to dataset
  cmd = "put " + target
  if not scdc.dataset_cmd(ds, cmd, ip, None):
    print("ERROR: command '" + cmd + "' failed!")
    break
  else:
    print("storing data with command '" + cmd + "': OK")

  # destroy input object
  scdc.dataset_input_destroy(ip)

  # create output object to write to file
  op = scdc.dataset_output_create("file", output_file)
  if not op:
    print("ERROR: open output file failed!")
    break

  # retrieve data from dataset to output object
  cmd = "get " + target
  if not scdc.dataset_cmd(ds, cmd, None, op):
    print("ERROR: command '" + cmd + "' failed!")
    break
  else:
    print("retrieving data with command '" + cmd + "': OK")

  # destroy output object
  scdc.dataset_output_destroy(op)

  # remove the data stored
  cmd = "rm " + target
  if not scdc.dataset_cmd(ds, cmd, None, None):
    print("ERROR: command '" + cmd + "' failed!")
    break
  else:
    print("removing data with command '" + cmd + "': OK")

  if dbstore:
    # become admin on db service
    cmd = "cd admin"
    if not scdc.dataset_cmd(ds, cmd, None, None):
      print("ERROR: command '" + cmd +"' failed!")
      break
    else:
      print("becoming admin with command '" + cmd + "': OK")

    # remove storage table (do not care whether fails or not)
    cmd = "rm storage_demo"
    scdc.dataset_cmd(ds, cmd, None, None)

  # close dataset
  scdc.dataset_close(ds);

  # always quit the loop and the end
  break


scdc.dataprov_close(dpFS)
scdc.dataprov_close(dpDB)
