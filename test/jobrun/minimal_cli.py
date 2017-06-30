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

import scdc


def printmsg(s):
  print("minimal_cli: " + s)

printmsg("scdc init")
scdc.init()

url = "scdc+tcp://localhost/jobR"

njobs = 5

job_input_dir = "minimal.in"
job_input = "data*.in"

job_output_dir = "minimal.out"
job_output = "run.output data.out"

# open dataset handle for the target service
dataset = scdc.dataset_open(url)

# put njobs jobs to the target service
for i in range(0, njobs):

  # select an individual job name
  job_name = "job" + str(i)

  # create an input object for the job that contains all job_input in directory job_input_dir
  input = scdc.dataset_input_create("fs", job_input_dir, job_input)

  # build the command string for submitting a job with the following commands:
  # - every command between "jobbegin" and "jobend" is executed asynchronously
  # - the command "run <params>" executes the jobrun command of the target service with parameters <params> (here job_name and job_input)
  cmd = "put " + job_name + " jobbegin;run " + job_name + " " + job_input + ";jobend"

  printmsg("putting job '" + job_name + "' with command '" + cmd + "'")

  # execute the command string with the input
  scdc.dataset_cmd(dataset, cmd, input, None)

  # destroy the input object
  scdc.dataset_input_destroy(input)


# list the current jobs on the target service
# create an output object for storing the result string
output = scdc.dataset_output_create("pybuf")

# execute the listing command
scdc.dataset_cmd(dataset, "ls", None, output)

printmsg("listing jobs: " + output.buf2str());

# destroy the output object
scdc.dataset_output_destroy(output)


# get the output of the jobs and delete them on the target service
for i in range(0, njobs):

  # select an individual job name and output directory
  job_name = "job" + str(i)
  outdir = job_output_dir + "/" + job_name

  # create output directory for the job
  if not os.path.exists(outdir): os.mkdir(outdir)

  # create an output object for storing the output in the output directory of the job 
  output = scdc.dataset_output_create("fs", outdir)

  # build the command string for getting the job output
  cmd = "get " + job_name + " " + job_output

  printmsg("getting output of job '" + job_name + "' with command '" + cmd + "'")

  # execute the command string with the input
  scdc.dataset_cmd(dataset, cmd, None, output)

  # destroy the output object
  scdc.dataset_output_destroy(output)

  # build the command string for deleting the job
  cmd = "rm " + job_name

  printmsg("deleting job '" + job_name + "' with command '" + cmd + "'")

  # execute the command string
  scdc.dataset_cmd(dataset, cmd, None, None)


# close the dataset handle
scdc.dataset_close(dataset)

printmsg("scdc release")
scdc.release()
