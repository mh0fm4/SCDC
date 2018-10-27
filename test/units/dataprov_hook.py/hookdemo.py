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


from z_pack import Z_LOG_PREFIX, Z_TRACE, Z_TRACE_F, __func__
import scdc


Z_LOG_PREFIX = "hookdemo: "


def hookdemo_open(conf, *args):
  Z_TRACE_F("conf: '" + str(conf) + "', args: '" + str(args) + "'")
  dp = 0xDEADBEEF
  Z_TRACE_F("return: dataprov: " + str(dp))
  return dp


def hookdemo_close(dataprov):
  Z_TRACE_F("dataprov: " + str(dataprov))
  ret = True
  Z_TRACE_F("return: " + str(ret))
  return ret


def hookdemo_config(dataprov, cmd, param, val):
  Z_TRACE_F("dataprov: " + str(dataprov) + ", cmd: '" + cmd + "', param: '" + param + "', val: '" + val + "'")
  if cmd == "put":
    ret = True
    res = None
  elif cmd == "get":
    ret = True
    res = param
  else:
    ret = False
    res = "configuration failed"
  Z_TRACE_F("return: " + str(ret) + ", result: " + str(res))
  return (ret, res) if res != None else ret


def hookdemo_dataset_open(dataprov, path):
  Z_TRACE_F("dataprov: " + str(dataprov) + ", path: '" + path + "'")
  ds = dataprov + 1 
  res = "result of " + __func__()
  Z_TRACE_F("return: dataset: " + str(ds) + ", result: " + res)
  return (ds, res)


def hookdemo_dataset_close(dataprov, dataset):
  Z_TRACE_F("dataprov: " + str(dataprov) + ", dataset: " + str(dataset))
  ret = True
  res = "result of " + __func__()
  Z_TRACE_F("return: " + str(ret) + ", result: " + res)
  return (ret, res)


def hookdemo_dataset_close_write_state(dataprov, dataset):
  Z_TRACE_F("dataprov: " + str(dataprov) + ", dataset: " + str(dataset))
  ret = str(dataset)
  res = "result of " + __func__()
  Z_TRACE_F("return: " + str(ret) + ", result: " + res)
  return (ret, res)


def hookdemo_dataset_open_read_state(dataprov, state):
  Z_TRACE_F("dataprov: " + str(dataprov) + ", state: '" + state + "'")
  ds = int(state)
  res = "result of " + __func__()
  Z_TRACE_F("return: dataset: " + str(ds) + ", result: " + res)
  return (ds, res)


def hookdemo_dataset_cmd(dataprov, dataset, cmd, params, input, output):
  Z_TRACE_F("dataset_cmd:")
  Z_TRACE_F(" dataprov: '" + str(dataprov) + "'")
  Z_TRACE_F(" dataset: '" + str(dataset) + "'")
  Z_TRACE_F(" cmd: '" + cmd + "'")
  Z_TRACE_F(" params: '" + str(params) + "'")
  Z_TRACE_F(" input: " + str(input))
  Z_TRACE_F(" output: " + str(output))

  output.buf = "OUT"
  output.buf_size = len(output.buf)
  output.current_size = output.buf_size
  output.total_size = output.current_size

  ret = True
  res = "result of " + __func__()

  Z_TRACE_F(" return: " + str((ret, res)))
  return (ret, res)


hook = scdc.dataprov_hook()
hook.open = hookdemo_open
hook.close = hookdemo_close
hook.config = hookdemo_config
hook.dataset_open = hookdemo_dataset_open
hook.dataset_close = hookdemo_dataset_close
hook.dataset_close_write_state = hookdemo_dataset_close_write_state
hook.dataset_open_read_state = hookdemo_dataset_open_read_state
hook.dataset_cmd = hookdemo_dataset_cmd
