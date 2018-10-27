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
import os
import signal
import socket
import getopt

import scdc


class client:
  def __init__(self):
    print("prepare_client:")

  def release(self):
    print("release_client:")

  def run(self, *cmds):
    print("run_client:")
    i = 0
    while True:
      if (len(cmds) > 0):
        cmdline = cmds[i] if i < len(cmds) else False
        i = i + 1
      else:
        try:
          cmdline = raw_input("Enter command ('<URL> [put|get] <JOBID>'): ")
        except:
          cmdline = False

      if cmdline == False or cmdline == "quit": break
      if cmdline == "": continue

      ret = scdc.dataset_cmd(None, cmdline, None, None)
      print("Command '" + cmdline + "' " + "OK" if ret else "FAILED!")


S = None

def sighandler(sig, frame):
  print("sighandler: sig: " + str(sig));
  interrupt = (sig != 0)
  scdc.nodeport_cancel(S.np_tcp, interrupt)
  scdc.nodeport_cancel(S.np_stream, interrupt)


def cmd_handler(cmd, data):
  print("cmd_handler: cmd: '" + cmd + "', data: " + str(data))
  if cmd == "quit":
    print("quiting")
    sighandler(0, None);
  else:
    print("unkown command: '" + cmd + "'")
  return True


class server:
  def __init__(self):
    self.np_tcp = None
    self.np_stream = None

  def run(self):
    self.np_tcp = scdc.nodeport_open("tcp:max_connections:", 2);
    self.np_stream = scdc.nodeport_open("stream:cmd_handler", cmd_handler, None);

    signal.signal(signal.SIGABRT, sighandler)
    signal.signal(signal.SIGTERM, sighandler)
    signal.signal(signal.SIGINT, sighandler)

    scdc.nodeport_start(self.np_tcp, scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL)
    scdc.nodeport_start(self.np_stream, scdc.NODEPORT_START_LOOP_UNTIL_CANCEL)

    scdc.nodeport_stop(self.np_stream)
    scdc.nodeport_stop(self.np_tcp)

    scdc.nodeport_close(self.np_stream)
    scdc.nodeport_close(self.np_tcp)


class worker(server):
  def __init__(self):
    print("init_worker:")
    self.worker = []

  def release(self):
    print("release_worker:")
    self.del_all()

  def run(self):
    print("run_worker:")
    server.run(self)

  def add(self, basepath, relay, *confs):
    dp_jobrun = scdc.dataprov_open(basepath, "jobrun", "uname -a; sleep", os.getcwd())
    if relay:
      print("registering at relay");
      if relay == True:
        worker_relay = "scdc:///rel"
        url = "scdc://"
      else:
        worker_relay = relay
        url = "scdc+tcp://" + socket.getfqdn()
      url += "/" + basepath
      worker_relay_ID = socket.gethostname() + "_" + basepath
      cmd = worker_relay + "/CONFIG put relay " + worker_relay_ID + " " + url
      ret = scdc.dataset_cmd(None, cmd, None, None)
      if not ret:
        print("error: registering at relay failed! (cmd: '" + cmd + "')")
        worker_relay = None
    else:
      worker_relay = None
      worker_relay_ID = None
    for c in confs:
      if c == "": continue
      print("configuration command '" + c + "'")
      cmd = "scdc:///" + basepath + "/CONFIG " + c
      ret = scdc.dataset_cmd(None, cmd, None, None)
      if not ret:
        print("error: configuration command failed! (cmd: '" + cmd + "')")
    self.worker.append((dp_jobrun, worker_relay, worker_relay_ID))

  def del_all(self):
    for w in self.worker:
      dp_jobrun = w[0]
      worker_relay = w[1]
      worker_relay_ID = w[2]
      if worker_relay != None:
        cmd = worker_relay + "/CONFIG put relay " + worker_relay_ID
        ret = scdc.dataset_cmd(None, cmd, None, None)
        if not ret: print("error: unregistering at relay failed! (cmd: '" + cmd + "')")
      scdc.dataprov_close(dp_jobrun)
    

class relay(server):
  def __init__(self):
    print("init_relay:")
    self.dp_relay = scdc.dataprov_open("rel", "jobrun_relay")

  def release(self):
    print("release_relay:")
    scdc.dataprov_close(self.dp_relay)

  def run(self):
    print("run_relay:")
    server.run(self)


print(str(sys.argv))

print("scdc init")
#scdc.init()
#scdc.init("log_filepath", "scdclog")
scdc.init("log_FILE", sys.stdout, sys.stderr)

relay_do = False
relay_run = False

worker_do = False
worker_run = False
worker_local = []
worker_relay = False

client_do = False
client_run = False
client_cmds = []
client_jobs = False
client_jobs_url = None

demo_do = None

opts, args = getopt.getopt(sys.argv[1:], "", [
  "relay",
  "worker=",
  "worker-relay=",
  "client",
  "demo=",
])


for opt in opts:
  if opt[0] == "--relay":
    relay_do = True
    relay_run = True
  elif opt[0] == "--worker":
    if opt[1] != "":
      worker_local.append((opt[1], ""))
    worker_do = True
    worker_run = True
  elif opt[0] == "--worker-relay":
    worker_relay = opt[1]
  elif opt[0] == "--client":
    client_do = True
    client_run = True
  elif opt[0] == "--demo":
    demo_do = opt[1]
  else:
    print("error: unknown option '" + opt[0] + "'");


if demo_do == "local":
#  relay_do = True

  worker_do = True
  worker_relay = worker_do and relay_do
  worker_local.append(("jobA", "put max_parallel_jobs 3"))
  worker_local.append(("jobB", "put max_parallel_jobs 1"))

  client_do = True
  client_run = True
#  client_jobs = True
  client_jobs_url = "scdc:///"
  client_jobs_url += "rel/amplitude_jobA" if worker_relay else "jobA"

elif demo_do == "localhostR":
  relay_do = True
  relay_run = True
elif demo_do == "localhostW":
  worker_do = True
  worker_run = True
  worker_local.append(("jobA", "put max_parallel_jobs 2"))
  worker_relay = "scdc+tcp://localhost/rel"
elif demo_do == "localhostC":
  client_do = True
  client_run = True
  client_jobs = True
  client_jobs_url = "scdc+tcp://localhost/"

elif demo_do != None:
  print("error: unknown demo '" + demo_do + "'")


if client_jobs:
  client_cmds.append(client_jobs_url + " put ABC1 jobbegin;run 4;jobend")
  client_cmds.append(client_jobs_url + " put ABC2 jobbegin;run 5;jobend")
  client_cmds.append(client_jobs_url + " put ABC3 jobbegin;run 4;jobend")
  client_cmds.append("scdcd:///jobB put ABC3 jobbegin;run 4;jobend")

  client_cmds.append(client_jobs_url + " get ABC1")
  client_cmds.append(client_jobs_url + " get ABC2")
  client_cmds.append(client_jobs_url + " get ABC3")


if client_run: relay_run = worker_run = False

R = None
if relay_do: R = relay()

W = None
if worker_do:
  W = worker()
  for w in worker_local:
    W.add(w[0], worker_relay, *w[1:])

C = None
if client_do: C = client()

if relay_run:
  R.run()
elif worker_run:
  W.run()
elif client_run:
  C.run(*client_cmds)

if C != None: C.release()
if W != None: W.release()
if R != None: R.release()

print("scdc release")
scdc.release()
