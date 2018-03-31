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


import os
import sys
import readline

import scdc
import repoH


#print("path: ", sys.path)

BASE_PATH = os.getenv('MERGE_SCDC_REPO_PATH', '')
MYSQL_CREDENTIALS = os.getenv('MERGE_SCDC_MYSQL_CREDENTIALS', '')

compX_WORKDIR = BASE_PATH + "demo/" + "compX/"


def manual_input_next(input):
  sys.stdout.write("INPUT #" + str(input.data) + " + <Enter>: ")
  b = sys.stdin.readline()
  if b == "":
    input.buf = None
    s = 0
  else:
    input.buf = b[:-1]
    s = len(input.buf)
  input.buf_size = s
  input.current_size = s
  input.total_size += input.current_size
  if s == 0:
    input.next = None
  input.data += 1
  return True

def manual_output_next(output):
  if output.format == "text" or output.format.split(":")[0] == "fslist":
    print "OUTPUT #" + str(output.data) + ": '" + output.buf2str() + "'"
  else:
    print "OUTPUT #" + str(output.data) + ": " + str(output.current_size) + " bytes"
  output.data = output.data + 1
  return True

def prepare_dataset_cmd(cmdline):
  print("cmdline: " + cmdline)
  input_pos = cmdline.find("<")
  output_pos = cmdline.find(">")

  cmdlen = len(cmdline)

  input_buf_size = 64
  output_buf_size = 256

  input_type = 0

  if input_pos >= 0:
    if input_pos < cmdlen:
      cmdlen = input_pos

    input_type = 1

    if output_pos > input_pos:
      input_str = cmdline[input_pos + 1:output_pos]
    else:
      input_str = cmdline[input_pos + 1:]
    print "input: '" + input_str + "'"

    name_pos = input_str.find(":")
    if name_pos >= 0:
      type_str = input_str[:name_pos]
      name_str = input_str[name_pos + 1:]
    else:
      type_str = None
      name_str = input_str

    if type_str == None:
      if os.path.isdir(name_str):
        type_str = "fs"
      elif os.path.isfile(name_str):
        type_str = "file"
      else:
        type_str = "text"

    if type_str == "fs":
      print "input fs: '" + name_str + "'"
      input = scdc.dataset_input_create("fs", name_str);
    elif type_str == "file":
      print "input file: '" + name_str + "'"
      input = scdc.dataset_input_create("file", name_str);
    elif type_str == "stream":
      print "input stream: '" + name_str + "'"
      input = scdc.dataset_input_create("stream", sys.stdin);
    elif type_str == "text":
      print "input text: '" + name_str + "'"
      input = scdc.dataset_input_create("buffer", name_str);
      input.format = "text"
    else:
      print "input manual:"
      input = scdc.dataset_input()
      input.format = "text"
      input.buf = "\x00" * input_buf_size
      input.buf_size = input_buf_size
      input.total_size = 0
      input.total_more = 1
      input.current_size = 0
      input.next = manual_input_next
      input.data = 0

  else:
    print "input none:"
    input = None


  output_type = 0

  if output_pos >= 0:
    if output_pos < cmdlen:
      cmdlen = output_pos

    output_type = 1

    if input_pos > output_pos:
      output_str = cmdline[output_pos + 1:input_pos]
    else:
      output_str = cmdline[output_pos + 1:]
    print "output: '" + output_str + "'"

    name_pos = output_str.find(":")
    if name_pos >= 0:
      type_str = output_str[:name_pos]
      name_str = output_str[name_pos + 1:]
    else:
      type_str = None
      name_str = output_str

    if type_str == None:
      type_str = "redirect"

    if type_str == "fs":
      print "output fs: '" + name_str + "'"
      output = scdc.dataset_output_create("fs", name_str);
    elif type_str == "file":
      print "output file: '" + name_str + "'"
      output = scdc.dataset_output_create("file", name_str);
    elif type_str == "stream":
      print "output stream: '" + name_str + "'"
      output = scdc.dataset_output_create("stream", sys.stdout);
    elif type_str == "consume":
      print "output consume: '" + name_str + "'"
      output = scdc.dataset_output_create("consume");
    elif type_str == "redirect":
      print "output redirect: '" + name_str + "'"
      output = scdc.dataset_output_create("alloc");
      output_type = 2
    else:
      print "output manual:"
      output = scdc.dataset_output()
      output.buf = "\x00" * output_buf_size
      output.buf_size = output_buf_size
      output.next = manual_output_next
      output.data = 0
      output_type = -1

  else:
    print "output loop manual:"
    output = scdc.dataset_output()
    output.buf = "\x00" * output_buf_size
    output.buf_size = output_buf_size

  return (cmdline[:cmdlen], input, input_type, output, output_type)


def unprepare_dataset_cmd(input, input_type, output, output_type):
  print "cmd input: " + str(input)

  if input_type > 0:
    scdc.dataset_input_destroy(input)
  else:
    pass

  print "cmd output: " + str(output)

  if output_type > 0:
    if output_type == 2:
      scdc.dataset_output_redirect(output, "stream", sys.stdout)
    scdc.dataset_output_destroy(output)
  else:
    if output_type == 0:
      i = 0
      while True:
        data = output.data
        output.data = i

        manual_output_next(output)

        i = output.data
        output.data = data

        if output.next is None:
          break

        if False:
          try:
            raw_input("Press <Enter>")
          except:
            pass

        output.next(output)


scdc.log_init("log_FILE", sys.stdout, sys.stderr)

scdc.init()

dataprovs = []

dataprovs.append(scdc.dataprov_open("repoA", "fs", BASE_PATH + "A"))
dataprovs.append(scdc.dataprov_open("repoB", "fs", BASE_PATH + "B"))
dataprovs.append(scdc.dataprov_open("repoC", "gen"))
#dataprovs.append(scdc.dataprov_open("repoD", "mysql", MYSQL_CREDENTIALS))
dataprovs.append(scdc.dataprov_open("repoH", "hook:id", repoH.repoH_hooks, 2501, 2502))
dataprovs.append(scdc.dataprov_open("repoJ", "jobrun", "uname -a; xterm -e sleep", BASE_PATH + "J"))
dataprovs.append(scdc.dataprov_open("repoJ1", "jobrun", "uname -a; echo \"J1\"; xterm -e sleep", BASE_PATH + "J"))
dataprovs.append(scdc.dataprov_open("repoJ2", "jobrun", "uname -a; echo \"J2\"; xterm -e sleep", BASE_PATH + "J"))
dataprovs.append(scdc.dataprov_open("repoJH", "jobrun:handler", repoH.repoH_jobrun_handler, True))
dataprovs.append(scdc.dataprov_open("repoRG", "register"))
dataprovs.append(scdc.dataprov_open("repoRL", "relay"))
dataprovs.append(scdc.dataprov_open("repoJRL", "jobrun_relay"))
dataprovs.append(scdc.dataprov_open("repoS", "fs_store", BASE_PATH + "S"))

#scdc.dataset_cmd(None, "scdc:///repoJRL/CONFIG/relay put sub scdc:///repoJ", None, None)
#scdc.dataset_cmd(None, "scdc:///repoJRL/CONFIG/relay put sub1 scdc:///repoJ1", None, None)
#scdc.dataset_cmd(None, "scdc:///repoJRL/CONFIG/relay put sub2 scdc:///repoJ2", None, None)

uri = None
cmdline = None

#cmdline = "scdc:/repoA/test_1k.in ls"

#cmdline = "\n\
#scdc:///repoJRL put test1 jobbegin;run 10;jobend\n\
#scdc:///repoJRL put test2 jobbegin;run 10;jobend\n\
#scdc:///repoJRL put test3 jobbegin;run 10;jobend\n\
#scdc:///repoJRL put test4 jobbegin;run 10;jobend\n\
#scdc:///repoJRL ls\n\
#"

#cmdline = "scdc:///repoJRL put test run 2\nscdc:///repoJRL ls\nscdc:///repoJRL rm test\nscdc:///repoJRL ls\nscdc:///repoJRL rm test"
#cmdline = "scdc:///repoJRL put test run 2\nscdc:///repoJRL/test rm\nscdc:///repoJRL ls"

#

#uri = "scdc:////CONFIG"
#uri = "scdc:///repoR"
#uri = "scdc+tcp://localhost/repoA"
#uri = "scdc+uds://repo_srv/repoA"
#uri = "scdc:/repoJ"

#cmdline = "put test_put_man.out<manual:"
#cmdline = "put test_put_text.out<text:ABCDEF"
#cmdline = "put test_put_file.out<file:test_2k.in"
#cmdline = "get test_2k.in"
#cmdline = "get test_2k.in>text:"
#cmdline = "get test_2k.in>file:test_put_file.out"

#cmdline = "ls"
#cmdline = "ls>file:TEST"
#cmdline = "get rand100M>file:TEST"
#cmdline = "put DEMO<file:TEST"
#cmdline = "get in2out<text:TEST"
#cmdline = "get in4out<text:TEST"
#cmdline = "get in5out<text:TEST"

#uri = "scdc:/repoC/ascii/zero"
#cmdline = "put TEST<file:test.in"
#cmdline = "get 70"
#cmdline = "get 70>probe:"

#uri = "scdc:///repoA/CONFIG/root"
#cmdline = "cd type\ninfo\ncd\nls"

#uri = "scdc:/repoH/"
#uri = "scdc+tcp://localhost/repoH/"
#cmdline = "ls"
#cmdline = "put test_put_man.out"
#cmdline = "put test_put_text.out<text:ABCDEF"
#cmdline = "put test_put_file.out<file:test_2k.in"
#cmdline = "get test_get"
#cmdline = "get test_get>text:"
#cmdline = "get test_get>file:test_get_file.out"

#cmdline = "scdc:/repoR put scdc:///repoA\nscdc:/repoR put scdc:///repoB\nscdc:/repoR info scdc:///repoA"
#cmdline = "scdc:/repoR put scdc:///repoA\nscdc:/repoR info scdc:///repoA"
#cmdline = "scdc:///repoH/CONFIG ls"

#cmdline = "get param2,*"

#cmdline = "scdc:/repoJ ls"

#cmdline = "scdc+uds://repo_srv/repoH get x20"
#cmdline = "scdc+uds://repo_srv/repoA ls\nscdc+uds://repo_srv/repoB ls"
#cmdline = "scdc:///repoA ls\nscdc:///repoB ls"

#cmdline = "scdc+tcp://localhost/repoA ls\nscdc+tcp:///repoB ls"

#cmdline = "scdc:///repoJ/CONFIG get type,max_parallel_jobs"
#cmdline = "scdc:///repoRG/CONFIG ls"
#cmdline = "scdc:///repoRG/CONFIG get type"
#cmdline = "scdc:///repoRG/CONFIG/type get"

#cmdline = "scdc:///repoJRL/CONFIG/relay put sub scdc:///repoA\nscdc:///repoJRL/CONFIG/relay put sub scdc:///repoJ\nscdc:///repoJRL/CONFIG/relay ls"

#cmdline = "scdc:///repoJRL/CONFIG/relay put sub scdc:///repoJ"

#cmdline = "scdc:///repoJ/CONFIG put nodes nodeA:8:1.5\n\
#scdc:///repoJ/CONFIG put nodes nodeB:4:2.5\n\
#scdc:///repoJ/CONFIG/nodes get\n\
#scdc:///repoJ/CONFIG/runjobs get"
#cmdline = "scdc:///repoJRL/CONFIG get relay"

uri = "scdc:///repoA"
#cmdline = "ls rand10M\ncd rand10M\nls\ncd\nls xxxxx"
#cmdline = "ls"


if uri:
  dataset = scdc.dataset_open(uri)
else:
  dataset = None
print("dataset: " + str(dataset))

if cmdline == False: cmdline = "quit"

if isinstance(cmdline, str):
  cmdlines = cmdline.split("\n")
  cmdlines_max = len(cmdlines)
else:
  cmdlines = None
  cmdlines_max = -1

n = 0
#while (dataset and cmdlines_max < 0) or (dataset != False and n < cmdlines_max):
while (cmdlines_max < 0) or (dataset != False and n < cmdlines_max):

  if cmdlines == None:
    try:
      c = raw_input("Enter command for dataset '" + str(dataset) + "': ")
    except:
      c = "quit"
  else:
    c = cmdlines[n]
    n += 1

  cs = c.split()

  if len(cs) == 0:
    continue

  if len(cs) == 1 and cs[0] == "quit":
    break

  if len(cs) == 2 and cs[0] == "open":
    if dataset: scdc.dataset_close(dataset)
    dataset = scdc.dataset_open(cs[1])
    continue

  if len(cs) == 1 and cs[0] == "close":
    scdc.dataset_close(dataset)
    dataset = None
    continue

  (cmd, input, input_type, output, output_type) = prepare_dataset_cmd(c)

  print("cmd: '" + cmd + "'");
  res = scdc.dataset_cmd(dataset, cmd, input, output)
  print("result: " + str(res));

  unprepare_dataset_cmd(input, input_type, output, output_type)

scdc.dataset_close(dataset)

while len(dataprovs) > 0:
  dp = dataprovs.pop()
  scdc.dataprov_close(dp)

scdc.release()

scdc.log_release()
