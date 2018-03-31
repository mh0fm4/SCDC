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


import scdc

class repoH_dataprov:
  def __init__(self, v):
    self.v = v
  def __str__(self):
    return "repoH_dataprov<" + str(self.v) + ">"
  def __repr__(self):
    return str(self)


class repoH_dataset:
  def __init__(self, dp, v):
    self.dp = dp
    self.v = v
  def __str__(self):
    return "repoH_dataset<" + str(self.dp) + ", " + str(self.v) + ">"
  def __repr__(self):
    return str(self)


def repoH_open(conf, *args):
  print("repoH_open: conf: '" + str(conf) + "', args: '" + str(args) + "'")
  return repoH_dataprov(args[0])


def repoH_close(dataprov):
  print("repoH_close: dataprov: '" + str(dataprov) + "'")
  return True


def repoH_config(dataprov, cmd, param, buf):
  if cmd == "ls":
    print("repoH_config: listing parameters")

    ret = "param0,param1,param2"

  elif cmd == "put":
    print("repoH_config: setting '" + param + "' to '" + buf + "'")

    ret = True

  elif cmd == "get":
    print("repoH_config: getting '" + param + "'")

    ret = param + "_VAL"

  else:
    return False

  return ret


def repoH_dataset_open(dataprov, path):
  print("repoH_dataset_open: dataprov: '" + str(dataprov) + "', path: '" + path + "'")
  return repoH_dataset(dataprov, dataprov.v + 1)


def repoH_dataset_close(dataprov, dataset):
  print("repoH_dataset_close: dataprov: " + str(dataprov) + ", dataset: " + str(dataset))
  return True


def repoH_dataset_open_read_state(dataprov, buf):
  print("repoH_dataset_open_read_state: dataprov: '" + str(dataprov) + "', buf: '" + buf + "'")
  return repoH_dataset(dataprov, int(buf))


def repoH_dataset_close_write_state(dataprov, dataset, buf_size):
  print("repoH_dataset_write_state: dataprov: '" + str(dataprov) + "', dataset: " + str(dataset) + ", buf_size: " + str(buf_size))
  return str(dataset.v)


DEFAULT_GET_SIZE = 13

def repoH_dataset_get_next(output):
  print("repoH_dataset_get_next: output: " + str(output))
  x = output.data
  output.current_size = min(output.buf_size, DEFAULT_GET_SIZE)
  output.buf = "x" * output.current_size
  x -= 1
  output.next = repoH_dataset_get_next if x > 0 else None
  output.data = x
  return True


def consume_input(prefix, input):
  while input:
    print(prefix + "input content: '" + str(input.buf) + "'")
    if input.next is None: break
    print(prefix + "next input")
    input.next(input)
    print(prefix + "input: " + str(input))


def clear_output(output):
  if output is None: return
  output.total_size = 0
  output.total_size_given = scdc.DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE
  output.current_size = 0
  output.next = None
  output.data = None


def repoH_dataset_cmd(dataprov, dataset, cmd, params, input, output):
  print("repoH_dataset_cmd: dataprov: " + str(dataprov) + ", dataset: " + str(dataset) + ", cmd: '" + cmd + "', params: '" + str(params) + "'")
  print("repoH_dataset_cmd: input: " + str(input))
  print("repoH_dataset_cmd: output: " + str(output))

  if (dataset == None): return repoH_config(cmd, params, input, output)

  clear_output(output)

  if cmd == "get":
    if params == "in2out":
      if input is not None:
        scdc.dataset_input_redirect(input, "to:output", output)
    elif params == "in4out":
      if input is not None:
        scdc.dataset_output_redirect(output, "from:input", input)
    elif params == "in5out":
      if input is not None:
        scdc.dataset_input_redirect(input, "to:file", "repoH_get.tmp")
        scdc.dataset_output_redirect(output, "from:file", "repoH_get.tmp")
    else:
      consume_input("repoH_dataset_cmd: ", input)

      if len(params) > 0 and params[0] == 'x':
        x = int(params[1:])
      else:
        x = 2

      output.format = "text"

      output.total_size = x * DEFAULT_GET_SIZE
      output.total_size_given = scdc.DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT

      output.data = x

      repoH_dataset_get_next(output)
  else:
    consume_input("repoH_dataset_cmd: ", input)

    output.format = "text"

    output.buf = "dataset: '" + str(dataset) + "', cmd: '" + cmd + "', params: '" + str(params) + "'"
    output.buf_size = len(output.buf)
    output.total_size = output.buf_size
    output.total_size_given = scdc.DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT
    output.current_size = output.buf_size

  print("repoH_dataset_cmd: output: '" + str(output))
  return True


repoH_hooks = scdc.dataprov_hook()
repoH_hooks.open = repoH_open
repoH_hooks.close = repoH_close
if True:
  repoH_hooks.config = repoH_config
repoH_hooks.dataset_open = repoH_dataset_open
repoH_hooks.dataset_close = repoH_dataset_close
if False:
  repoH_hooks.dataset_open_read_state = repoH_dataset_open_read_state
  repoH_hooks.dataset_close_write_state = repoH_dataset_close_write_state
repoH_hooks.dataset_cmd = repoH_dataset_cmd


def repoH_jobrun_handler(data, cmd, params, input, output):
  print("repoH_jobrun_handler: data: " + str(data) + ", cmd: '" + cmd + "', params: '" + str(params) + "'")
  print("repoH_jobrun_handler: input: " + str(input))
  print("repoH_jobrun_handler: output: " + str(output))
