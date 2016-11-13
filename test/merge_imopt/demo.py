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


import os
import sys
import scdc
import shutil


HAVE_DEMO_TRACE = True
HAVE_DEMO_TRACE_TUPLE = True
HAVE_DEMO_TRACE_CLASS = True

HAVE_DEMO_ERROR = True

DEMO_LOG_PREFIX = ""

if HAVE_DEMO_TRACE:
  DEMO_TRACE_PREFIX = "DEMO_TRACE: "

  def DEMO_TRACE_N(x):
    if sys.version_info[0] < 3:
      sys.stdout.write(DEMO_TRACE_PREFIX + DEMO_LOG_PREFIX + x)
    else:
#      print(DEMO_TRACE_PREFIX + x, end = "")
      sys.stdout.write(DEMO_TRACE_PREFIX + DEMO_LOG_PREFIX + x)

  def DEMO_TRACE(x):
    DEMO_TRACE_N(x)
    print("")

  if HAVE_DEMO_TRACE_TUPLE:
    def DEMO_TRACE_TUPLE(t, x):
      DEMO_TRACE(x + str(t))
  else:
    def DEMO_TRACE_TUPLE(t, x):
      pass

  if HAVE_DEMO_TRACE_CLASS:
    def DEMO_TRACE_CLASS(c, x):
      DEMO_TRACE(x + str(c))
  else:
    def DEMO_TRACE_CLASS(c, x):
      pass
else:
  def DEMO_TRACE(x):
    pass
  def DEMO_TRACE_TUPLE(t, x):
    pass
  def DEMO_TRACE_CLASS(c, x):
    pass


if HAVE_DEMO_ERROR:
  DEMO_ERROR_PREFIX = "DEMO_ERROR: "

  def DEMO_ERROR_N(x):
    if sys.version_info[0] < 3:
      sys.stderr.write(DEMO_ERROR_PREFIX + DEMO_LOG_PREFIX + x)
    else:
#      print(DEMO_ERROR_PREFIX + x, end = "")
      sys.stderr.write(DEMO_ERROR_PREFIX + DEMO_LOG_PREFIX + x)

  def DEMO_ERROR(x):
    DEMO_ERROR_N(x)
    print("")
else:
  def DEMO_ERROR(x):
    pass
  


DEMO_BASE_PATH = os.getenv('MERGE_SCDC_REPO_PATH', '') + "demo/"
DEMO_MYSQL_CREDENTIALS = os.getenv('MERGE_SCDC_MYSQL_CREDENTIALS', '')

if not os.path.isdir(DEMO_BASE_PATH):
  DEMO_ERROR("directory '" + DEMO_BASE_PATH + "' does not exist")
  sys.exit()


def ensure_empty_directory(d):
  shutil.rmtree(d, True)
  os.makedirs(d)


class demo_comps:
  def __init__(self):
    pass


  # main
  def init(self):
    DEMO_TRACE("DEMO_BASE_PATH: '" + DEMO_BASE_PATH + "'")
    self.compA_init()
    self.compB_init()
    self.storeFS_init()
#    self.storeDB_init()
    return True

  def release(self):
#    self.storeDB_release()
    self.storeFS_release()
    self.compB_release()
    self.compA_release()

  def info(self):
    self.compA_info()
    self.compB_info()
    self.storeFS_info()
#    self.storeDB_info()


  # compA
  compA_path = "compA"
  compA_WORKDIR = DEMO_BASE_PATH + "compA/"
  compA_CMD = "uname -a; echo compA; " + os.getcwd() + "/run.sh"
  compA_dataprov = None

  def compA_init(self):
    DEMO_TRACE("compA: init")
    DEMO_TRACE("  WORKDIR: '" + self.compA_WORKDIR + "'")
    DEMO_TRACE("  CMD: '" + self.compA_CMD + "'")
    ensure_empty_directory(self.compA_WORKDIR)
    self.compA_dataprov = scdc.dataprov_open(self.compA_path, "jobrun", self.compA_CMD, self.compA_WORKDIR)
    DEMO_TRACE("  dataprov: '" + str(self.compA_dataprov) + "'")

  def compA_release(self):
    DEMO_TRACE("compA: release")
    scdc.dataprov_close(self.compA_dataprov)

  def compA_info(self):
    DEMO_TRACE("DEMO_INFO")
    output = scdc.dataset_output_create("pybuf", 64)
    scdc.dataset_cmd(None, "scdc:///" + self.compA_path + " info", None, output)
    x = output.buf2str().split(":")
    scdc.dataset_output_destroy(output)
    DEMO_TRACE("compA: jobs total: " + str(x[0]) + ", jobs waiting: " + str(x[1]) + ", jobs running: " + str(x[2]))


  # compB
  compB_path = "compB"
  compB_WORKDIR = DEMO_BASE_PATH + "compB/"
  compB_CMD = "uname -a; echo compB; " + os.getcwd() + "/run.sh"
  compB_dataprov = None

  def compB_init(self):
    DEMO_TRACE("compB: init")
    DEMO_TRACE("  WORKDIR: '" + self.compB_WORKDIR + "'")
    DEMO_TRACE("  CMD: '" + self.compB_CMD + "'")
    ensure_empty_directory(self.compB_WORKDIR)
    self.compB_dataprov = scdc.dataprov_open(self.compB_path, "jobrun", self.compB_CMD, self.compB_WORKDIR)
    DEMO_TRACE("  dataprov: '" + str(self.compB_dataprov) + "'")

  def compB_release(self):
    DEMO_TRACE("compB: release")
    scdc.dataprov_close(self.compB_dataprov)

  def compB_info(self):
    output = scdc.dataset_output_create("pybuf", 64)
    scdc.dataset_cmd(None, "scdc:///" + self.compB_path + " info", None, output)
    x = output.buf2str().split(":")
    scdc.dataset_output_destroy(output)
    DEMO_TRACE("compB: jobs total: " + str(x[0]) + ", jobs waiting: " + str(x[1]) + ", jobs running: " + str(x[2]))


  # storeFS
  storeFS_WORKDIR = DEMO_BASE_PATH + "storeFS/"
  storeFS_dataprov = None

  def storeFS_init(self):
    DEMO_TRACE("storeFS: init")
    DEMO_TRACE("  WORKDIR: '" + self.storeFS_WORKDIR + "'")
    self.storeFS_dataprov = scdc.dataprov_open("storeFS", "fs:store", self.storeFS_WORKDIR)
    DEMO_TRACE("  dataprov: '" + str(self.storeFS_dataprov) + "'")

  def storeFS_release(self):
    DEMO_TRACE("storeFS: release")
    if self.storeFS_dataprov != False: scdc.dataprov_close(self.storeFS_dataprov)

  def storeFS_info(self):
    DEMO_TRACE("storeFS: info")
    

  # storeDB
  storeDB_dataprov = None

  def storeDB_init(self):
    DEMO_TRACE("storeDB: init")
    DEMO_TRACE("  DEMO_MYSQL_CREDENTIALS: '" + DEMO_MYSQL_CREDENTIALS + "'")
    if DEMO_MYSQL_CREDENTIALS != "":
      self.storeDB_dataprov = scdc.dataprov_open("storeDB", "mysql", DEMO_MYSQL_CREDENTIALS)
    else:
      self.storeDB_dataprov = None
    DEMO_TRACE("  dataprov: '" + str(self.storeDB_dataprov) + "'")

  def storeDB_release(self):
    DEMO_TRACE("storeDB: release")
    if self.storeDB_dataprov != False: scdc.dataprov_close(self.storeDB_dataprov)

  def storeDB_info(self):
    DEMO_TRACE("storeDB: info")
