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
import signal

import scdc
import repoH


#print("path: ", sys.path)

BASE_PATH = os.getenv('MERGE_SCDC_REPO_PATH', '')
MYSQL_CREDENTIALS = os.getenv('MERGE_SCDC_MYSQL_CREDENTIALS', '')


np_direct = None
np_tcp = None
np_uds = None
np_timer = None
np_stream = None

NODEPORT_DIRECT = None
NODEPORT_TCP    = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_UDS    = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_TIMER  = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_STREAM = scdc.NODEPORT_START_LOOP_UNTIL_CANCEL


def sighandler(sig, frame):
  print("sighandler: sig: " + str(sig));

  interrupt = (sig != 0)

  if NODEPORT_DIRECT != None:
    scdc.nodeport_cancel(np_direct, interrupt)
  if NODEPORT_TCP != None:
    scdc.nodeport_cancel(np_tcp, interrupt)
  if NODEPORT_UDS != None:
    scdc.nodeport_cancel(np_uds, interrupt)
  if NODEPORT_TIMER != None:
    scdc.nodeport_cancel(np_timer, interrupt)
  if NODEPORT_STREAM != None:
    scdc.nodeport_cancel(np_stream, interrupt)


def cmd_handler(data, cmd, params):
  print("cmd_handler: data: " + str(data) + ", cmd: '" + cmd + "', params: '" + params + "'")

  if cmd == "quit":
    print("quiting server")
    sighandler(0, None);
  else:
    print("unkown server command: '" + cmd + "'")

  return True


def timer_handler(data):
  print("TIMER: TICK #" + str(data))


signal.signal(signal.SIGABRT, sighandler)
signal.signal(signal.SIGTERM, sighandler)
signal.signal(signal.SIGINT, sighandler)

scdc.log_init("log_FILE", sys.stdout, sys.stderr)

scdc.init()

dataprovs = []

dataprovs.append(scdc.dataprov_open("repoA", "fs", BASE_PATH + "A"))
dataprovs.append(scdc.dataprov_open("repoB", "fs", BASE_PATH + "B"))
dataprovs.append(scdc.dataprov_open("repoC", "gen"))
#dataprovs.append(scdc.dataprov_open("repoD", "mysql", MYSQL_CREDENTIALS))
dataprovs.append(scdc.dataprov_open("repoH", "hook:id", repoH.repoH_hooks, 2501, 2502))
dataprovs.append(scdc.dataprov_open("repoJ", "jobrun", "uname -a; sleep", BASE_PATH + "J"))

if NODEPORT_DIRECT != None: np_direct = scdc.nodeport_open("direct", "directaccess");
if NODEPORT_TCP != None: np_tcp = scdc.nodeport_open("tcp:max_connections", 2);
if NODEPORT_UDS != None: np_uds = scdc.nodeport_open("uds:socketname:max_connections", "repo_srv", 2);
if NODEPORT_TIMER != None: np_timer = scdc.nodeport_open("timer:max_count", 1.0, timer_handler, 0, 5);
if NODEPORT_STREAM != None: np_stream = scdc.nodeport_open("stream:cmd_handler", cmd_handler, None);

scdc.nodeport_start(np_direct, NODEPORT_DIRECT)
scdc.nodeport_start(np_tcp, NODEPORT_TCP)
scdc.nodeport_start(np_uds, NODEPORT_UDS)
scdc.nodeport_start(np_timer, NODEPORT_TIMER)
scdc.nodeport_start(np_stream, NODEPORT_STREAM)

scdc.nodeport_stop(np_stream)
scdc.nodeport_stop(np_timer)
scdc.nodeport_stop(np_uds)
scdc.nodeport_stop(np_tcp)
scdc.nodeport_stop(np_direct)

scdc.nodeport_close(np_stream)
scdc.nodeport_close(np_timer)
scdc.nodeport_close(np_uds)
scdc.nodeport_close(np_tcp)
scdc.nodeport_close(np_direct)

while len(dataprovs) > 0:
  scdc.dataprov_close(dataprovs.pop())

scdc.release()

scdc.log_release()
