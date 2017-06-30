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


import scdc


PREFIX = "hook_direct: "

def hookdemo_dataset_cmd(dataprov, dataset, cmd, params, input, output):
  print(PREFIX + "dataset_cmd:")
  print(PREFIX + " dataprov: '" + str(dataprov) + "'")
  print(PREFIX + " dataset: '" + str(dataset) + "'")
  print(PREFIX + " cmd: '" + cmd + "'")
  print(PREFIX + " params: '" + str(params) + "'")
  print(PREFIX + " input: " + str(input))
  print(PREFIX + " output: " + str(output))
  return True


hookdemo = scdc.dataprov_hook()
hookdemo.dataset_cmd = hookdemo_dataset_cmd


print(PREFIX + "start hook demo direct")
print(PREFIX)

scdc.init()

dp_hook = scdc.dataprov_open("hookdemo", "hook", hookdemo)

input = scdc.dataset_input()
output = scdc.dataset_output()

# char format[SCDC_FORMAT_MAX_SIZE];
#  scdcint_t buf_size;
#  void *buf;
#  scdcint_t total_size, current_size;
#  char total_size_given;
#  scdc_dataset_inout_next_f *next;
#  void *data;

scdc.dataset_cmd(scdc.DATASET_NULL, "scdc:/hookdemo CMD PARAM1 PARAM2 PARAM3", input, output)

scdc.dataprov_close(dp_hook)

scdc.release()

print(PREFIX + "quit hook demo direct")
print(PREFIX)
