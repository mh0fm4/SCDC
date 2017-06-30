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


import sys
import inspect

scdc_parent_frame = inspect.stack()[1][0]
if hasattr(scdc_parent_frame, "f_globals") and scdc_parent_frame.f_globals.has_key("SCDC_THREADING"): SCDCMOD_THREADING = scdc_parent_frame.f_globals["SCDC_THREADING"]
import scdcmod

(
  HAVE_PYSCDC_INFO,
  HAVE_PYSCDC_TRACE,
  HAVE_PYSCDC_FAIL,
  HAVE_PYSCDC_ERROR,
  HAVE_PYSCDC_ASSERT,
  HAVE_PYSCDC_DEBUG
) = scdcmod.CONFIG()

HAVE_PYSCDC_TRACE_CLASS = True

PYSCDC_TRACE_NOT = True

(
  DATASET_NULL,
  DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT,
  DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST,
  DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_MOST,
  DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE,
  NODEPORT_START_NONE,
  NODEPORT_START_LOOP_UNTIL_CANCEL,
  NODEPORT_START_LOOP_UNTIL_IDLE,
  NODEPORT_START_ASYNC_UNTIL_CANCEL,
  NODEPORT_START_ASYNC_UNTIL_IDLE,
  PYBUF_DEFAULT_SIZE
) = scdcmod.CONSTANTS()


if HAVE_PYSCDC_TRACE and not PYSCDC_TRACE_NOT:
  PYSCDC_TRACE_PREFIX = "PYSCDC-TRACE: "

  def PYSCDC_TRACE(x):
    if scdcmod.log_cout_write(PYSCDC_TRACE_PREFIX + x + "\n") == False: sys.stdout.write(PYSCDC_TRACE_PREFIX + x)

  def PYSCDC_TRACE_N(x):
    if scdcmod.log_cout_write(PYSCDC_TRACE_PREFIX + x) == False: sys.stdout.write(PYSCDC_TRACE_PREFIX + x)

  if HAVE_PYSCDC_TRACE_CLASS:
    def PYSCDC_TRACE_CLASS(c, x):
      PYSCDC_TRACE(x + str(c))
  else:
    def PYSCDC_TRACE_CLASS(c, x):
      pass
else:
  def PYSCDC_TRACE(x):
    pass
  def PYSCDC_TRACE_CLASS(c, x):
    pass


class dataprov_hook:
  def __init__(self):
    self.open = None
    self.close = None
    self.config = None
    self.dataset_open = None
    self.dataset_close = None
    self.dataset_open_read_state = None
    self.dataset_close_write_state = None
    self.dataset_cmd = None
  def init_from_prefix(self, prefix):
    hooks = ["open", "close", "dataset_open", "dataset_close", "dataset_open_read_state", "dataset_close_write_state", "dataset_cmd" ]
    for h in hooks:
      eval("self." + h + " = " + prefix + h)


class cptr:
  def __init__(self, v, *args):
    self.set_c(v)
    if len(args) > 0:
      self.type = args[0]
    else:
      self.type = "cptr"
#    print str(self)
  def set_c(self, v):
    self.v = v
  def get_c(self):
    return self.v
  def __str__(self):
    return self.type + "<0x%x>" % (self.v)
#    return "CPTR_STR"
#  def __repr__(self):
#    return  "CPTR_REPR"
#    return str(self)

class dataprov(cptr):
  def __init__(self, v):
    cptr.__init__(self, v, "dataprov")

class nodeport(cptr):
  def __init__(self, v):
    cptr.__init__(self, v, "nodeport")

class dataset(cptr):
  def __init__(self, v):
    cptr.__init__(self, v, "dataset")

class cbuf(cptr):
  def __init__(self, v):
    cptr.__init__(self, v, "cbuf")
  def to_str(self, *args):
    s = scdcmod.cptr2py(self.v, "z", args[0]) if len(args) > 0 else scdcmod.cptr2py(self.v, "z")
    return "" if s is None else s
  def to_bytes(self, *args):
    return scdcmod.cptr2py(self.v, "y", args[0]) if len(args) > 0 else scdcmod.cptr2py(self.v, "y")
  def from_str(self, x):
    self.v = scdcmod.py2cptr(x, "z")
  def from_bytes(self, x):
    self.v = scdcmod.py2cptr(x, "y")


class dataset_inout:
  def __init__(self):
    self.unset()
  def __str__(self):
    s = ""
    s += "format: '" + str(self.format) + "', "
    if isinstance(self.buf, str):
      s += "buf: '" + self.buf[:16] + "...' = " + str(len(self.buf)) + ", "
    else:
      s += "buf: '" + str(self.buf) + "', "
    s += "buf_size: " + str(self.buf_size) + ", "
    s += "total_size: " + str(self.total_size) + ", "
    s += "total_size_given: '" + str(self.total_size_given) + "', "
    s += "current_size: "  +str(self.current_size) + ", "
    s += "next: " + str(self.next) + ", "
    s += "data: " + str(self.data) + ", "
    s += "intern: " + str(self.intern)
    return s
  def unset(self):
    self.format = None
    self.buf = None
    self.buf_size = 0
    self.total_size = 0
    self.total_size_given = DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE
    self.current_size = 0
    self.next = None
    self.data = None
    self.intern = 0
  def set_all(self, vals):
#    print("vals: " + str(vals));
    (self.format, self.buf, self.buf_size, self.total_size, self.total_size_given, self.current_size, self.next, self.data, self.intern) = vals
  def get_all(self):
    return (self.format, self.buf, self.buf_size, self.total_size, self.total_size_given, self.current_size, self.next, self.data, self.intern)
  def buf2str(self):
    return self.buf.to_str(self.current_size) if self.buf is not None else ""


class dataset_input(dataset_inout):
  def __str__(self):
    return "input<" + dataset_inout.__str__(self) + ">"
  def get_copy(self):
    c = dataset_input()
    c.set_all(self.get_all())
    return c

class dataset_output(dataset_inout):
  def __str__(self):
    return "output<" + dataset_inout.__str__(self) + ">"
  def get_copy(self):
    c = dataset_output()
    c.set_all(self.get_all())
    return c


def pybuf(*args):
  PYSCDC_TRACE("pybuf: " + str(args))
  return scdcmod.pybuf(*args)


# this loop handler is called inside transport loops (e.g., tcp, uds, stream) to trigger the Python signal handling
def loop_handler_dummy(data, i):
#  print("loop_handler: data: " + str(data) + ", i: " + str(i))
  return True


def init(*args):
  PYSCDC_TRACE("init: " + str(args))
  return scdcmod.init(*args if args else (None, ))


def release(*args):
  PYSCDC_TRACE("release: " + str(args))
  return scdcmod.release(*args)


def log_init(*args):
  PYSCDC_TRACE("log_init: " + str(args))
  return scdcmod.log_init(*args)


def log_release(*args):
  PYSCDC_TRACE("log_release: " + str(args))
  return scdcmod.log_release(*args)


def dataprov_open(*args):
  PYSCDC_TRACE("dataprov_open: " + str(args))
  return scdcmod.dataprov_open(*args)


def dataprov_close(*args):
  PYSCDC_TRACE("dataprov_close: " + str(args))
  return scdcmod.dataprov_close(*args)


def nodeport_open(*args):
  PYSCDC_TRACE("nodeport_open: " + str(args))
  return scdcmod.nodeport_open(*args)


def nodeport_close(*args):
  PYSCDC_TRACE("nodeport_close: " + str(args))
  return scdcmod.nodeport_close(*args)


def nodeport_start(*args):
  PYSCDC_TRACE("nodeport_start: " + str(args))
  return scdcmod.nodeport_start(*args)


def nodeport_stop(*args):
  PYSCDC_TRACE("nodeport_stop: " + str(args))
  return scdcmod.nodeport_stop(*args)


def nodeport_cancel(*args):
  PYSCDC_TRACE("nodeport_cancel: " + str(args))
  return scdcmod.nodeport_cancel(*args)


def nodeport_authority(*args):
  PYSCDC_TRACE("nodeport_authority: " + str(args))
  return scdcmod.nodeport_authority(*args)


def nodeport_supported(*args):
  PYSCDC_TRACE("nodeport_supported: " + str(args))
  return scdcmod.nodeport_supported(*args)


def dataset_open(*args):
  PYSCDC_TRACE("dataset_open: " + str(args))
  return scdcmod.dataset_open(*args)


def dataset_close(*args):
  PYSCDC_TRACE("dataset_close: " + str(args))
  return scdcmod.dataset_close(*args)


def dataset_cmd(*args):
  PYSCDC_TRACE("dataset_cmd: " + str(args))
  return scdcmod.dataset_cmd(*args)


def dataset_input_create(conf, *args):
  PYSCDC_TRACE("dataset_input_create: conf: '" + conf + "', args: " + str(len(args)))
  confs = conf.split(":")
  if (confs[0] == "pybuf"):
    input = dataset_input()
    input.buf_size = int(confs[1]) if len(confs) > 1 else PYBUF_DEFAULT_SIZE
    input.buf = pybuf(input.buf_size)
  else:
    input = scdcmod.dataset_input_create(conf, *args)
  PYSCDC_TRACE_CLASS(input, "dataset_input_create: input: ")
  return input


def dataset_input_destroy(*args):
  PYSCDC_TRACE("dataset_input_destroy: " + str(args))
  return scdcmod.dataset_input_destroy(*args)


def dataset_output_create(conf, *args):
  PYSCDC_TRACE("dataset_output_create: conf: '" + conf + "', args: '" + str(args) + "'")
  confs = conf.split(":")
  if (confs[0] == "pybuf"):
    output = dataset_output()
    output.buf_size = int(confs[1]) if len(confs) > 1 else PYBUF_DEFAULT_SIZE
    output.buf = pybuf(output.buf_size)
  else:
    output = scdcmod.dataset_output_create(conf, *args)
  PYSCDC_TRACE_CLASS(output, "dataset_output_create: output: ")
  return output


def dataset_output_destroy(*args):
  PYSCDC_TRACE("dataset_output_destroy: " + str(args))
  return scdcmod.dataset_output_destroy(*args)


def dataset_input_redirect(*args):
  PYSCDC_TRACE("dataset_input_redirect: " + str(args))
  return scdcmod.dataset_input_redirect(*args)


def dataset_output_redirect(*args):
  PYSCDC_TRACE("dataset_output_redirect: " + str(args))
  return scdcmod.dataset_output_redirect(*args)
