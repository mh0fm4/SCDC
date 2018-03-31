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


import platform
import sys
import os
import time

import scdc

import demo
from demo import DEMO_TRACE


demo.DEMO_LOG_PREFIX = platform.node() + " compL: "


DEMO_CCPE_I = 100
DEMO_CCPE_II = 110
DEMO_CCPE_III = 120
DEMO_CCPE_VI = 130


#compL_demo = DEMO_CCPE_I
#compL_demo = DEMO_CCPE_II
compL_demo = DEMO_CCPE_III + 2

compL_case = None
#compL_case = demo.DEMO_BASE_PATH + "cases/dummy/"
#compL_case = demo.DEMO_BASE_PATH + "cases/quader/"
#compL_case = demo.DEMO_BASE_PATH + "cases/quader_fast/"
#compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset/"
#compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_bench/"
#compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_fast/"

compL_jobs_total = 1
compL_jobs_at_once = 1

compL_compA_input = None
compL_compA_runcmd = "dummy -o compA.output -s 2"
compL_compB_input = None
compL_compB_runcmd = "dummy -o compB.output -s 3"
compL_compB_get_compA = True

compL_clean_up = True

compA_CONFIG = ""
compB_CONFIG = ""

compS_URL = None
#compS_URL = "scdc:///storeFS"
#compS_URL = "scdc:///storeDB"
compS_STORE = "demo_io"

host_amplitude="amplitude.informatik.tu-chemnitz.de"
host_mike2="mike2.informatik.tu-chemnitz.de"
host_localhost = "localhost"

if compL_demo == 1:
  # demo I: single laptop, single application, direct function calls
#  compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_fast/"
  compL_jobs_total = 1
  compL_jobs_at_once = 1
  compA_URL = "scdc:///compA/"
  compB_URL = "scdc:///compB/"
  compB_compA_URL = compA_URL

elif compL_demo == 2:
  # demo II: two laptops, two applications, network communication
  compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_fast/"
#  compL_case = demo.DEMO_BASE_PATH + "cases/dummy/"
  compL_jobs_total = 1
  compL_jobs_at_once = 1
#  compA_URL = "scdc+tcp://" + host_mike2 + "/compA/"
#  compB_URL = "scdc+tcp://" + host_mike2 + "/compB/"
  compA_URL = "scdc+tcp://" + host_localhost + "/compA/"
  compB_URL = "scdc+tcp://" + host_localhost + "/compB/"
  compB_compA_URL = "scdc:///compA/"

elif compL_demo == 3:
  # demo III: two laptops and desktop PC, three applications, network communication
  compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_fast/"
  compL_jobs_total = 1
  compL_jobs_at_once = 1
  compA_URL = "scdc+tcp://" + host_amplitude + "/compA/"
  compB_URL = "scdc+tcp://" + host_mike2 + "/compB/"
  compB_compA_URL = compA_URL
  compA_CONFIG = "xterm 0"
#  compB_CONFIG = "xterm 0,Xvfb 1"

elif compL_demo == 4:
  # demo IV: two laptops and desktop PC, data exchange with MySQL database
  compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_fast/"
  compL_jobs_total = 1
  compL_jobs_at_once = 1
  compA_URL = "scdc+tcp://" + host_amplitude + "/compA/"
  compB_URL = "scdc+tcp://" + host_mike2 + "/compB/"
  compB_compA_URL = compA_URL
  compA_CONFIG = "xterm 0"
#  compB_CONFIG = "xterm 0,Xvfb 1"
  compS_URL = "scdc:///storeDB"

elif compL_demo == 5:
  # demo V: two laptops, parallel jobs
  compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_fast/"
  compL_jobs_total = 10
  compL_jobs_at_once = 5
  compA_URL = "scdc+tcp://" + host_mike2 + "/compA/"
  compB_URL = "scdc+tcp://" + host_mike2 + "/compB/"
  compB_compA_URL = "scdc:///compA/"
  compA_CONFIG = "max_parallel_jobs 2"
  compB_CONFIG = "max_parallel_jobs 2"

elif compL_demo == 6:
  # demo V: two laptops, massiv parallel jobs
  compL_case = demo.DEMO_BASE_PATH + "cases/dummy/"
  compL_jobs_total = 50
  compL_jobs_at_once = 25
  compA_URL = "scdc+tcp://" + host_mike2 + "/compA/"
  compB_URL = "scdc+tcp://" + host_mike2 + "/compB/"
  compB_compA_URL = "scdc:///compA/"
  compA_CONFIG = "max_parallel_jobs 8"
  compB_CONFIG = "max_parallel_jobs 8"

elif compL_demo == DEMO_CCPE_I:
  # ccpe I: seq. local
  compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_bench/"
  compL_jobs_total = 4
  compL_jobs_at_once = 1
  compA_URL = "scdc:///compA/"
  compB_URL = "scdc:///compB/"
  compB_compA_URL = "scdc:///compA/"
  compA_CONFIG = "max_parallel_jobs 1,xterm 0,show_output 0"
  compB_CONFIG = "max_parallel_jobs 1,xterm 0,show_output 0,Xvfb 1"

elif compL_demo == DEMO_CCPE_II:
  # ccpe II: seq. ws1
  compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_bench/"
  compL_jobs_total = 4
  compL_jobs_at_once = 1
  compA_URL = "scdc+tcp://fiona.informatik.tu-chemnitz.de/rel/westmere1_compA/"
  compB_URL = "scdc+tcp://fiona.informatik.tu-chemnitz.de/rel/westmere1_compB/"
  compB_compA_URL = "scdc:///compA/"
  compA_CONFIG = "max_parallel_jobs 1,xterm 0,show_output 0"
  compB_CONFIG = "max_parallel_jobs 1,xterm 0,show_output 0,Xvfb 1"

elif compL_demo >= DEMO_CCPE_III and compL_demo < DEMO_CCPE_III + 10:
  # ccpe III: par. ws1, ws2, ws3, ws4, ws5
  compL_case = demo.DEMO_BASE_PATH + "cases/plate_w_inset_bench/"
  workers = ["westmere1", "westmere2", "westmere3", "westmere4", "westmere5"][:compL_demo - DEMO_CCPE_III]
  compL_jobs_total = 4 * 12 * len(workers)
  compL_jobs_at_once = compL_jobs_total
  compA_URL = ["scdc+tcp://fiona.informatik.tu-chemnitz.de/rel/" + x + "_compA/" for x in workers]
  compB_URL = ["scdc+tcp://fiona.informatik.tu-chemnitz.de/rel/" + x + "_compB/" for x in workers]
  compB_compA_URL = "scdc:///compA/"
  compA_CONFIG = "max_parallel_jobs 6,xterm 0,show_output 0"
  compB_CONFIG = "max_parallel_jobs 6,xterm 0,show_output 0,Xvfb 1"


if compL_case:
  compL_compA_input = "compA.input/"
  with open(compL_case + compL_compA_input + "run.cmd") as f: compL_compA_runcmd = f.read()
  compL_compB_input = "compB.input/"
  with open(compL_case + compL_compB_input + "run.cmd") as f: compL_compB_runcmd = f.read()


if not isinstance(compA_URL, list): compA_URL = [compA_URL]
if not isinstance(compB_URL, list): compB_URL = [compB_URL]
if not isinstance(compB_compA_URL, list): compB_compA_URL = [compB_compA_URL]


class demo_jobs:

  jobs_done = 0
  jobs_total = 0
  jobs_at_once = 1

  compA_dataset = []
  compA_dataset_last = -1
  compB_dataset = []
  compB_dataset_last = -1
  compB_compA_URL_last = -1

  def next_compA_dataset(self):
    if len(self.compA_dataset) <= 0: return False
    self.compA_dataset_last = (self.compA_dataset_last + 1) % len(self.compA_dataset)
    return self.compA_dataset[self.compA_dataset_last]

  def next_compB_dataset(self):
    if len(self.compB_dataset) <= 0: return False
    self.compB_dataset_last = (self.compB_dataset_last + 1) % len(self.compB_dataset)
    return self.compB_dataset[self.compB_dataset_last]

  def next_compB_compA_URL_last(self):
    if len(compB_compA_URL) <= 0: return False
    self.compB_compA_URL_last = (self.compB_compA_URL_last + 1) % len(compB_compA_URL)
    return compB_compA_URL[self.compB_compA_URL_last]

  def init(self, total, at_once):
    DEMO_TRACE("demo_jobs.init")

    self.jobs_done = 0
    self.jobs_total = total
    self.jobs_at_once = at_once

    # dataset(s) of compA
    i = 0
    for x in compA_URL:
      DEMO_TRACE("demo_jobs.init: compA[" + str(i) + "]: URL: " + x)
      ds = scdc.dataset_open(x)
      self.compA_dataset += [ds]
      if compA_CONFIG: scdc.dataset_cmd(None, x + "CONFIG put " + compA_CONFIG, None, None)
#      scdc.dataset_cmd(None, x + "CONFIG get *", None, scdc.dataset_output_create("stream:autodestroy", sys.stdout))
      i += 1

    # dataset of compB
    i = 0
    for x in compB_URL:
      DEMO_TRACE("demo_jobs.init: compB[" + str(i) + "]: URL: " + x)
      ds = scdc.dataset_open(x)
      self.compB_dataset += [ds]
      if compB_CONFIG: scdc.dataset_cmd(None, x + "CONFIG put " + compB_CONFIG, None, None)
#      scdc.dataset_cmd(None, x + "CONFIG get *", None, scdc.dataset_output_create("stream:autodestroy", sys.stdout))
      i += 1

    # dataset of store
    if compS_URL != None:
      self.compS_dataset = scdc.dataset_open(compS_URL)
      if self.compS_dataset != None:
        scdc.dataset_cmd(self.compS_dataset, "cd ADMIN", None, None)
        scdc.dataset_cmd(self.compS_dataset, "rm " + compS_STORE, None, None)
        scdc.dataset_cmd(self.compS_dataset, "put " + compS_STORE, None, None)
        scdc.dataset_cmd(self.compS_dataset, "cd " + compS_STORE, None, None)
    else:
      self.compS_dataset = None

  def finished(self):
#    DEMO_TRACE("demo_jobs.loop_finished")
    return (self.jobs_done >= self.jobs_total)

  def execute(self):
    cur_job_ids = range(self.jobs_done, min(self.jobs_done + self.jobs_at_once, self.jobs_total))
    DEMO_TRACE("demo_jobs.execute: current jobs: " + str(cur_job_ids))

    cur_jobs = []

    for i in cur_job_ids:
      idA = "demoA_" + str(i)
      idB = "demoB_" + str(i)

      cA_ds = self.next_compA_dataset()

      DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: putting job '" + idA + "' to component A with input '" + str(compL_compA_input) + "'")
      scdc.dataset_cmd(cA_ds, "cd", None, None)
      scdc.dataset_cmd(cA_ds, "rm " + idA, None, None)
      input = scdc.dataset_input_create("fs:autodestroy", compL_case, compL_compA_input) if compL_compA_input else None
      if input != None and self.compS_dataset != None:
        DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: storing input for job '" + idA + "' in storage component '" + compS_URL + "/" + compS_STORE + "'")
        scdc.dataset_cmd(self.compS_dataset, "put " + idA + ".in", input, None)
        input = None
        get_cmdA = "scdc " + compS_URL + "/" + compS_STORE + " get " + idA + ".in;"
      else:
        get_cmdA = ""
      scdc.dataset_cmd(cA_ds, "put " + idA + " jobbegin;" + get_cmdA + "run " + compL_compA_runcmd + ";jobend", input, None)

      output = scdc.dataset_output_create("pybuf", 64)
      ret = scdc.dataset_cmd(cA_ds, "info", None, output)
      if ret:
        DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: info of compA: job '" + idA + "' state: '" + output.buf2str() + "'")
      else:
        DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: info of compA: FAILED!")
      scdc.dataset_output_destroy(output)

      cB_ds = self.next_compB_dataset()
      cB_compA_URL = self.next_compB_compA_URL_last()

      DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: putting job '" + idB + "' to component B with input '" + str(compL_compB_input) + "'")
      scdc.dataset_cmd(cB_ds, "cd", None, None)
      scdc.dataset_cmd(cB_ds, "rm " + idB, None, None)
      if compL_compB_get_compA and cB_compA_URL:
        DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: get input from compA with URL '" + cB_compA_URL + "'")
        get_cmdA = "scdc " + cB_compA_URL + " get " + idA + " compA.output compA.output.vtk;"
      else:
        get_cmdA = ""
      input = scdc.dataset_input_create("fs:autodestroy", compL_case, compL_compB_input) if compL_compB_input else None
      if input != None and self.compS_dataset != None:
        DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: storing input for job '" + idB + "' in storage component '" + compS_URL + "/" + compS_STORE + "'")
        scdc.dataset_cmd(self.compS_dataset, "put " + idB + ".in", input, None)
        input = None
        get_cmdB = "scdc " + compS_URL + "/" + compS_STORE + " get " + idB + ".in;"
      else:
        get_cmdB = ""
      scdc.dataset_cmd(cB_ds, "put " + idB + " jobbegin;" + get_cmdA + get_cmdB + "run " + compL_compB_runcmd + ";jobend", input, None)

      output = scdc.dataset_output_create("pybuf", 64)
      ret = scdc.dataset_cmd(cB_ds, "info", None, output)
      if ret:
        DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: info of compB: job '" + idB + "' state: '" + output.buf2str() + "'")
      else:
        DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: info of compB: FAILED!")
      scdc.dataset_output_destroy(output)

      cur_jobs += [(i, idA, cA_ds, idB, cB_ds)]

    i = 0
    for cA_ds in self.compA_dataset:
      scdc.dataset_cmd(cA_ds, "cd", None, None)
      output = scdc.dataset_output_create("pybuf", 64)
      ret = scdc.dataset_cmd(cA_ds, "info", None, output)
      if ret:
        x = output.buf2str().split(":")
        DEMO_TRACE("demo_jobs.execute: compA[" + str(i) + "]: jobs total: " + str(x[0]) + ", jobs waiting: " + str(x[1]) + ", jobs runing: " + str(x[2]))
      else:
        DEMO_TRACE("demo_jobs.execute: compA[" + str(i) + "]: info: FAILED!")
      scdc.dataset_output_destroy(output)
      i += 1

    i = 0
    for cB_ds in self.compB_dataset:
      scdc.dataset_cmd(cB_ds, "cd", None, None)
      output = scdc.dataset_output_create("pybuf", 64)
      ret = scdc.dataset_cmd(cB_ds, "info", None, output)
      if ret:
        x = output.buf2str().split(":")
        DEMO_TRACE("demo_jobs.execute: compB[" + str(i) + "]: jobs total: " + str(x[0]) + ", jobs waiting: " + str(x[1]) + ", jobs runing: " + str(x[2]))
      else:
        DEMO_TRACE("demo_jobs.execute: compB[" + str(i) + "]: info: FAILED!")
      scdc.dataset_output_destroy(output)

#    DEMO_TRACE("demo_jobs.execute: current jobs: " + str(cur_jobs))

    for j in cur_jobs:
      i = j[0]
      DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: finalizing")

      idA = j[1]
      cA_ds = j[2]

      idB = j[3]
      cB_ds = j[4]

      jobout = "demo_" + str(i) + ".output"

      DEMO_TRACE("demo_jobs.execute: job[" + str(i) + "]: getting job '" + idB + "' from component B")
      scdc.dataset_cmd(cB_ds, "cd", None, None)
      demo.ensure_empty_directory(jobout)
      output = scdc.dataset_output_create("fs:autodestroy", jobout)
#      ret = scdc.dataset_cmd(cB_ds, "get " + idB + " " + ("compA.output compA.output.vtk " if compL_compB_get_compA else "") + "compB.output compB.output.vtk", None, output)
      ret = scdc.dataset_cmd(cB_ds, "get " + idB + " " + ("compA.output " if compL_compB_get_compA else "") + "compB.output", None, output)

      if compL_clean_up:
        # clean up
        scdc.dataset_cmd(cA_ds, "rm " + idA, None, None)
        scdc.dataset_cmd(cB_ds, "rm " + idB, None, None)

      self.jobs_done += 1

  def release(self):
    DEMO_TRACE("demo_jobs.release:")
    for cA_ds in self.compA_dataset:
      scdc.dataset_close(cA_ds)
    for cB_ds in self.compB_dataset:
      scdc.dataset_close(cB_ds)


compL_WORKDIR = demo.DEMO_BASE_PATH + "compL/"

DEMO_TRACE("starting component L on host '" + platform.node() + "'")
DEMO_TRACE("  WORKDIR: '" + compL_WORKDIR + "'")
DEMO_TRACE("  case: '" + str(compL_case) + "'")
DEMO_TRACE("  jobs_total: '" + str(compL_jobs_total) + "'")
DEMO_TRACE("  jobs_at_once: '" + str(compL_jobs_at_once) + "'")
DEMO_TRACE("  compA_input: '" + str(compL_compA_input) + "'")
DEMO_TRACE("  compA_runcmd: '" + str(compL_compA_runcmd) + "'")
DEMO_TRACE("  compA_URL: '" + str(compA_URL) + "'")
DEMO_TRACE("  compA_CONFIG: '" + compA_CONFIG + "'")
DEMO_TRACE("  compB_input: '" + str(compL_compB_input) + "'")
DEMO_TRACE("  compB_runcmd: '" + str(compL_compB_runcmd) + "'")
DEMO_TRACE("  compB_get_compA: '" + str(compL_compB_get_compA) + "'")
DEMO_TRACE("  compB_URL: '" + str(compB_URL) + "'")
DEMO_TRACE("  compB_CONFIG: '" + compB_CONFIG + "'")
DEMO_TRACE("  compB_compA_URL: '" + str(compB_compA_URL) + "'")
DEMO_TRACE("")

scdc.init()

comps = demo.demo_comps()
comps.init()

DEMO_TRACE("")

jobs = demo_jobs()
jobs.init(compL_jobs_total, compL_jobs_at_once)

t = time.time()

while not jobs.finished():
  jobs.execute()

t = time.time() - t

jobs.release()

DEMO_TRACE("")

comps.release()

DEMO_TRACE("")

scdc.release()

DEMO_TRACE("total time: " + str(t))
DEMO_TRACE("per_job time: " + (str(t / compL_jobs_total) if compL_jobs_total > 0 else "0"))

DEMO_TRACE("finishing component L on host '" + platform.node() + "'")
