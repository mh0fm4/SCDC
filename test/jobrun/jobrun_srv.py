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
import signal
import socket

import scdc


relay = None
#relay = "scdc+tcp://localhost/rel"

np_tcp = None

def sighandler(sig, frame):
  print("sighandler: sig: " + str(sig));
  interrupt = (sig != 0)
  scdc.nodeport_cancel(np_tcp, interrupt)


def printmsg(s):
  print("jobrun_srv: " + s)


scdc.log_init("log_FILE", sys.stdout, sys.stderr)

printmsg("scdc init")
scdc.init()

basepath = "jobR"

dp_job = scdc.dataprov_open(basepath, "jobrun", "uname -a; " + os.getcwd() + "/mpirun.sh", os.getcwd())

# configure local jobrun data provider
#scdc.dataset_cmd(None, "scdc:///" + basepath + "/CONFIG put max_parallel_jobs 3", None, None)
#scdc.dataset_cmd(None, "scdc:///" + basepath + "/CONFIG put xterm 1", None, None)

if relay:
  printmsg("registering at relay");
  url = "scdc+tcp://" + socket.getfqdn() + "/" + basepath
  cmd = relay + "/CONFIG put relay " + basepath + " " + url
  relay_ret = scdc.dataset_cmd(None, cmd, None, None)
  if relay_ret:
    printmsg("registering at relay OK!")
  else:
    printmsg("registering at relay FAILED! (cmd: '" + cmd + "')")

np_tcp = scdc.nodeport_open("tcp:max_connections:", 2);

signal.signal(signal.SIGABRT, sighandler)
signal.signal(signal.SIGTERM, sighandler)
signal.signal(signal.SIGINT, sighandler)

scdc.nodeport_start(np_tcp, scdc.NODEPORT_START_LOOP_UNTIL_CANCEL)

# running

scdc.nodeport_stop(np_tcp)

scdc.nodeport_close(np_tcp)

if relay and relay_ret:
  cmd = relay + "/CONFIG put relay " + basepath
  relay_ret = scdc.dataset_cmd(None, cmd, None, None)
  if relay_ret:
    printmsg("unregistering at relay OK!")
  else:
    printmsg("unregistering at relay FAILED! (cmd: '" + cmd + "')")

scdc.dataprov_close(dp_job)

printmsg("scdc release")
scdc.release()

scdc.log_release()
