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


import signal
import platform
import scdc

import demo
from demo import DEMO_TRACE


demo.DEMO_LOG_PREFIX = platform.node() + " compR: "


np_tcp = None
np_timer = None
np_stream = None

NODEPORT_TCP    = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_TIMER  = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_STREAM = scdc.NODEPORT_START_LOOP_UNTIL_CANCEL


def sighandler(sig, frame):
  print("sighandler: sig: " + str(sig))

  interrupt = (sig != 0)

  if NODEPORT_TCP != None:
    scdc.nodeport_cancel(np_tcp, interrupt)
  if NODEPORT_TIMER != None:
    scdc.nodeport_cancel(np_timer, interrupt)
  if NODEPORT_STREAM != None:
    scdc.nodeport_cancel(np_stream, interrupt)


def cmd_handler(cmd, data):
  print("cmd_handler: cmd: '" + cmd + "', data: " + str(data))

  if cmd == "quit":
    print("quiting server")
    sighandler(0, None);
  else:
    print("unkown server command: '" + cmd + "'")

  return True


def timer_handler(data):
  output = scdc.dataset_output_create("pybuf", 64)
  scdc.dataset_cmd(None, "scdc:///rel/CONFIG/relay info", None, output)
  DEMO_TRACE("relays: " + output.buf2str())
  scdc.dataset_output_destroy(output)


DEMO_TRACE("starting component R on host '" + platform.node() + "'")

signal.signal(signal.SIGABRT, sighandler)
signal.signal(signal.SIGTERM, sighandler)
signal.signal(signal.SIGINT, sighandler)

scdc.init()

ds_rel = scdc.dataprov_open("rel", "jobrun_relay")

np_tcp = scdc.nodeport_open("tcp:max_connections", 2)
np_timer = scdc.nodeport_open("timer", 1.0, timer_handler, 0)
np_stream = scdc.nodeport_open("stream:cmd_handler", cmd_handler, None)

scdc.nodeport_start(np_tcp, NODEPORT_TCP)
scdc.nodeport_start(np_timer, NODEPORT_TIMER)
scdc.nodeport_start(np_stream, NODEPORT_STREAM)

scdc.nodeport_stop(np_stream)
scdc.nodeport_stop(np_timer)
scdc.nodeport_stop(np_tcp)

scdc.nodeport_close(np_stream)
scdc.nodeport_close(np_timer)
scdc.nodeport_close(np_tcp)

scdc.dataprov_close(ds_rel)

scdc.release()

DEMO_TRACE("finishing component R on host '" + platform.node() + "'")
