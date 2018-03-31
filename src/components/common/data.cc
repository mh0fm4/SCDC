/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
 *  
 *  This file is part of the Simulation Component and Data Coupling (SCDC) library.
 *  
 *  The SCDC library is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  The SCDC library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstring>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATA

#include "config.hh"
#include "log.hh"
#include "data.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "data: "


void scdc_data::compact()
{
  if (read_pos <= 0) return;

  SCDC_TRACE("compact: " << write_pos - read_pos << " bytes at offset " << read_pos);

  if (read_pos < write_pos) memmove(buf, buf + read_pos, write_pos - read_pos);

  write_pos -= read_pos;
  read_pos = 0;
}


void scdc_data::info(const string &prefix)
{
  SCDC_TRACE_N(prefix << "buf: " << buf_size << " at " << static_cast<void *>(buf) << ", alloc: " << alloc << ", write_pos: " << write_pos << ", read_pos: " << read_pos);
}


#undef SCDC_LOG_PREFIX
