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


from z_pack import Z_LOG_PREFIX, Z_TRACE, Z_TRACE_F
import scdc
import hookdemo


Z_LOG_PREFIX = "hookdemo: "


Z_TRACE("start hook demo direct")
Z_TRACE()

scdc.init()

dp_hook = scdc.dataprov_open("hookdemo", "hook:id", hookdemo.hook, 0x2501)

input = scdc.dataset_input()
output = scdc.dataset_output()

# char format[SCDC_FORMAT_MAX_SIZE];
#  scdcint_t buf_size;
#  void *buf;
#  scdcint_t total_size, current_size;
#  char total_size_given;
#  scdc_dataset_inout_next_f *next;
#  void *data;

scdc.dataset_cmd(scdc.DATASET_NULL, "scdc:/hookdemo/CONFIG put param0 abc", None, None)
scdc.dataset_cmd(scdc.DATASET_NULL, "scdc:/hookdemo/CONFIG get param0", None, None)
scdc.dataset_cmd(scdc.DATASET_NULL, "scdc:/hookdemo/CONFIG fail", None, None)

input.buf = "IN"
input.buf_size = len(input.buf)
input.current_size = input.buf_size

scdc.dataset_cmd(scdc.DATASET_NULL, "scdc:/hookdemo cmd param1 param2 param3", input, output)

while True:
  Z_TRACE("output: " + str(output.buf.to_str(output.current_size)))
  if (output.next == None): break
  output.next(output)

scdc.dataprov_close(dp_hook)

scdc.release()

Z_TRACE("quit hook demo direct")
Z_TRACE()
