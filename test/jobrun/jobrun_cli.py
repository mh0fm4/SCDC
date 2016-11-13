#!/usr/bin/python

#  
#  Copyright (C) 2014, 2015, 2016 Michael Hofmann
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


import sys
import os
import random
import time

import scdc


def printmsg(s):
  print("jobrun_cli: " + s)


def jobrun_handler(data, jobid, cmd, params, input, output):
  print("jobrun_handler: data: " + str(data) + ", jobid: '" + jobid + "', cmd: '" + cmd + "', params: '" + str(params) + "'")
  print("jobrun_handler: input: " + str(input))
  print("jobrun_handler: output: " + str(output))
  if (cmd == "run"):
    run_cmd = "uname -a; " + os.getcwd() + "/mpirun.sh " + params
    os.system("xterm -e '" + run_cmd + "'")
  return True


hostname = None
#hostname = "localhost"

relay = None
#relay = "rel"


scdc.log_init("log_FILE", sys.stdout, sys.stderr)

printmsg("scdc init")
scdc.init()

basepath = "jobR"

dp_job = scdc.dataprov_open(basepath, "jobrun:system:runcmd:workdir", "uname -a; " + os.getcwd() + "/mpirun.sh -hosts $NODES$:$NPROCS$ $PARAMS$", os.getcwd())
#dp_job = scdc.dataprov_open(basepath, "jobrun:handler:runcmd", jobrun_handler, True, "run hosts: $NODES$:$NPROCS$")

# configure local jobrun data provider
#scdc.dataset_cmd(None, "scdc:///" + basepath + "/CONFIG put cores 3", None, None)
#scdc.dataset_cmd(None, "scdc:///" + basepath + "/CONFIG put xterm 1", None, None)

if hostname is None and relay:
  dp_rel = scdc.dataprov_open(relay, "jobrun_relay")

  rel = "scdc:///" + relay
  printmsg("registering '" + basepath + "' at relay: '" + rel + "'");
  cmd = rel + "/CONFIG put relay " + basepath + " " + "scdc:///" + basepath
  scdc.dataset_cmd(None, cmd, None, None)


if hostname:
  url = "scdc+tcp://" + hostname
else:
  url = "scdc://"

if relay: url += "/" + relay

url += "/" + basepath

# configure remote jobrun data provider
scdc.dataset_cmd(None, url + "/CONFIG put cores local 5", None, None)
scdc.dataset_cmd(None, url + "/CONFIG put xterm 1", None, None)
scdc.dataset_cmd(None, url + "/CONFIG put runjobs 0", None, None)

jobs = 3

for i in range(0, jobs):
  name = "job" + str(i)
  sleep = random.randint(1, 4)
  sleep = 3
  printmsg("sleep: " + str(sleep))
  cmd = url + " put " + name + ":2:1:local jobbegin;run 'echo $JOBID$:$NPT$ && sleep " + str(sleep) + "';jobend"
  printmsg("putting job '" + name + "' to URL '" + url + "'")
  scdc.dataset_cmd(None, cmd, None, None)

#output = scdc.dataset_output()
#output.buf_size = 128
#output.buf = scdc.pybuf(output.buf_size)
#scdc.dataset_cmd(None, url + " ls", None, output)
#printmsg("listing jobs: " + output.buf2str());

time.sleep(1)

scdc.dataset_cmd(None, url + "/CONFIG put runjobs 1", None, None)

for i in range(0, jobs):
  name = "job" + str(i)
#  cmd = url + " get " + name + " run.output"
#  printmsg("getting job '" + name + "' from URL '" + url + "'")
#  scdc.dataset_cmd(None, cmd, None, None)
  cmd = url + " rm " + name
  printmsg("deleting job '" + name + "' from URL '" + url + "'")
  scdc.dataset_cmd(None, cmd, None, None)


if hostname is None and relay:
  rel = "scdc:///" + relay
  printmsg("unregistering '" + basepath + "' at relay: '" + rel + "'");
  cmd = rel + "/CONFIG put relay " + basepath
  scdc.dataset_cmd(None, cmd, None, None)

  scdc.dataprov_close(dp_rel)

scdc.dataprov_close(dp_job)

printmsg("scdc release")
scdc.release()

scdc.log_release()
