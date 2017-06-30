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


import sys
import os
import signal
import socket

import scdc


np_tcp = None

def sighandler(sig, frame):
  print("sighandler: sig: " + str(sig));
  interrupt = (sig != 0)
  scdc.nodeport_cancel(np_tcp, interrupt)


def printmsg(s):
  print("minimal_srv: " + s)


printmsg("scdc init")
scdc.init()

basepath = "jobR"
runcmd = "uname -a; " + os.getcwd() + "/minimal.sh"
workdir = os.getcwd() + "/minimal.work"

dp_job = scdc.dataprov_open(basepath, "jobrun", runcmd, workdir)

# configure local jobrun data provider
scdc.dataset_cmd(None, "scdc:///" + basepath + "/CONFIG put max_parallel_jobs 2", None, None)
scdc.dataset_cmd(None, "scdc:///" + basepath + "/CONFIG put show_output 1", None, None)
scdc.dataset_cmd(None, "scdc:///" + basepath + "/CONFIG put xterm 1", None, None)

np_tcp = scdc.nodeport_open("tcp:max_connections:", 2);

signal.signal(signal.SIGABRT, sighandler)
signal.signal(signal.SIGTERM, sighandler)
signal.signal(signal.SIGINT, sighandler)

scdc.nodeport_start(np_tcp, scdc.NODEPORT_START_LOOP_UNTIL_CANCEL)

# running

scdc.nodeport_stop(np_tcp)

scdc.nodeport_close(np_tcp)

scdc.dataprov_close(dp_job)

printmsg("scdc release")
scdc.release()
