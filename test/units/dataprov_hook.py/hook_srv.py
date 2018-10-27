#!/usr/bin/python

#  
#  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
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


#import sys
import signal

from z_pack import Z_LOG_PREFIX, Z_TRACE, Z_TRACE_F
import scdc
import hookdemo


PREFIX = "hook_srv: "

np_tcp = None
np_stream = None

NODEPORT_TCP = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_STREAM = scdc.NODEPORT_START_LOOP_UNTIL_CANCEL

def sighandler(sig, frame):
  Z_TRACE_F("sig: " + str(sig));

  interrupt = (sig != 0)

  if NODEPORT_TCP != None:
    scdc.nodeport_cancel(np_tcp, interrupt)
  if NODEPORT_STREAM != None:
    scdc.nodeport_cancel(np_stream, interrupt)


def cmd_handler(data, cmd, params):
  Z_TRACE_F("data: " + str(data) + ", cmd: '" + cmd + "', params: '" + params + "'")

  if cmd == "quit":
    Z_TRACE("quiting server")
    sighandler(0, None);
  else:
    Z_TRACE("unkown server command: '" + cmd + "'")

  return True


signal.signal(signal.SIGABRT, sighandler)
signal.signal(signal.SIGTERM, sighandler)
signal.signal(signal.SIGINT, sighandler)


Z_TRACE("start hook demo")
Z_TRACE()

scdc.init()

dp_hook = scdc.dataprov_open("hookdemo", "hook:id", hookdemo.hook, 0x2501)

if NODEPORT_TCP != None:
    np_tcp = scdc.nodeport_open("tcp:max_connections", 2)
if NODEPORT_STREAM != None:
    np_stream = scdc.nodeport_open("stream:cmd_handler", cmd_handler, None)

scdc.nodeport_start(np_tcp, NODEPORT_TCP)
scdc.nodeport_start(np_stream, NODEPORT_STREAM)

scdc.nodeport_stop(np_stream)
scdc.nodeport_stop(np_tcp)

scdc.nodeport_close(np_stream)
scdc.nodeport_close(np_tcp)

scdc.dataprov_close(dp_hook)

scdc.release()

#scdc.log_release()

Z_TRACE("quit hook demo")
Z_TRACE()
