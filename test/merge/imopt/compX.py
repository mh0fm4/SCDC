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


import sys
import signal
import platform
import socket
import scdc

import demo
from demo import DEMO_TRACE


demo.DEMO_LOG_PREFIX = platform.node() + " compX: "


np_uds = None
np_tcp = None
np_timer = None
np_stream = None

NODEPORT_UDS    = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_TCP    = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_TIMER  = scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL
NODEPORT_STREAM = scdc.NODEPORT_START_LOOP_UNTIL_CANCEL

comps = None

relay_path = None
relay_url = None


def sighandler(sig, frame):
  print("sighandler: sig: " + str(sig))

  interrupt = (sig != 0)

  if NODEPORT_UDS != None:
    scdc.nodeport_cancel(np_uds, interrupt)
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
  comps.info()


DEMO_TRACE("starting component X on host '" + platform.node() + "'")

i = 1
while i < len(sys.argv):
  if sys.argv[i] == "-r" or sys.argv[i] == "--relay":
    relay_url = sys.argv[i + 1]
    relay_path = socket.gethostname()
    i += 1
  i += 1

signal.signal(signal.SIGABRT, sighandler)
signal.signal(signal.SIGTERM, sighandler)
signal.signal(signal.SIGINT, sighandler)

scdc.init()

comps = demo.demo_comps()
comps.init()

if relay_path and relay_url:
  # register compA
  relay_pathA = relay_path + "_compA"
  url = "scdc+tcp://" + socket.getfqdn() + "/" + comps.compA_path
  cmd = relay_url + "/CONFIG/relay put " + relay_pathA + " " + url
  ret = scdc.dataset_cmd(None, cmd, None, None)
  DEMO_TRACE("register path '" + relay_pathA + "' at relay '" + relay_url + "' to url '" + url + "': " + ("OK" if ret else "FAILED"))
  if not ret: DEMO_TRACE("failing command: '" + cmd + "'")
  # register compB
  relay_pathB = relay_path + "_compB"
  url = "scdc+tcp://" + socket.getfqdn() + "/" + comps.compB_path
  cmd = relay_url + "/CONFIG/relay put " + relay_pathB + " " + url
  ret = scdc.dataset_cmd(None, cmd, None, None)
  DEMO_TRACE("register path '" + relay_pathB + "' at relay '" + relay_url + "' to url '" + url + "': " + ("OK" if ret else "FAILED"))
  if not ret: DEMO_TRACE("failing command: '" + cmd + "'")

np_uds = scdc.nodeport_open("uds:max_connections", 2)
np_tcp = scdc.nodeport_open("tcp:max_connections", 2)
np_timer = scdc.nodeport_open("timer", 1.0, timer_handler, 0)
np_stream = scdc.nodeport_open("stream:cmd_handler", cmd_handler, None)

scdc.nodeport_start(np_uds, NODEPORT_UDS)
scdc.nodeport_start(np_tcp, NODEPORT_TCP)
scdc.nodeport_start(np_timer, NODEPORT_TIMER)
scdc.nodeport_start(np_stream, NODEPORT_STREAM)

scdc.nodeport_stop(np_stream)
scdc.nodeport_stop(np_timer)
scdc.nodeport_stop(np_tcp)
scdc.nodeport_stop(np_uds)

scdc.nodeport_close(np_stream)
scdc.nodeport_close(np_timer)
scdc.nodeport_close(np_tcp)
scdc.nodeport_close(np_uds)

if relay_path and relay_url:
  # unregister compA
  ret = scdc.dataset_cmd(None, relay_url + "/CONFIG/relay put " + relay_pathA, None, None)
  DEMO_TRACE("unregister path '" + relay_path + "' at relay '" + relay_url + "': " + ("OK" if ret else "FAILED"))
  # unregister compB
  ret = scdc.dataset_cmd(None, relay_url + "/CONFIG/relay put " + relay_pathB, None, None)
  DEMO_TRACE("unregister path '" + relay_path + "' at relay '" + relay_url + "': " + ("OK" if ret else "FAILED"))

comps.release()

scdc.release()

DEMO_TRACE("finishing component X on host '" + platform.node() + "'")
