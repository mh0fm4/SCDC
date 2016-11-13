/*
 *  Copyright (C) 2014, 2015, 2016 Michael Hofmann
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


#ifndef __SCDC_DATA_HH__
#define __SCDC_DATA_HH__


#include "scdc_defs.h"


class scdc_data;

typedef bool scdc_data_next_f(scdc_data *data);

class scdc_data
{
  public:
    scdc_data()
      :buf(0), buf_size(0), alloc(false), write_pos(0), read_pos(0), data(0), next(0) { }
    ~scdc_data() { if (alloc) delete[] buf; }

    void alloc_buf(scdcint_t buf_size_) { buf = new char[buf_size_]; buf_size = buf_size_; alloc = true; }
    void free_buf() { if (alloc) delete[] buf; buf = 0; buf_size = 0; alloc = false; }

    void set_buf(void *buf_, scdcint_t buf_size_) { free_buf(); buf = static_cast<char *>(buf_); buf_size = buf_size_; }
    char *get_buf() { return buf; }
    scdcint_t get_buf_size() { return buf_size; }

    bool ptr_inside(void *p) { return (buf <= p && p < buf + buf_size); }

    void set_write_pos(scdcint_t write_pos_) { write_pos = write_pos_; }
    void inc_write_pos(scdcint_t write_pos_) { write_pos += write_pos_; }
    scdcint_t get_write_pos() { return write_pos; }

    char *get_write_pos_buf() { return buf + write_pos; }
    scdcint_t get_write_pos_buf_size() { return buf_size - write_pos; }

    void set_read_pos(scdcint_t read_pos_) { read_pos = read_pos_; }
    void inc_read_pos(scdcint_t read_pos_) { read_pos += read_pos_; }
    scdcint_t get_read_pos() { return read_pos; }

    char *get_read_pos_buf() { return buf + read_pos; }
    scdcint_t get_read_pos_buf_size() { return write_pos - read_pos; }

    void set_data(void *data_) { data = data_; }
    void *get_data() { return data; }

    void set_next(scdc_data_next_f *next_) { next = next_; }
    scdc_data_next_f *get_next() { return next; }
    bool do_next() { if (!next) return false; return next(this); }

    void compact();

    virtual void info(const std::string &prefix = "");

  protected:
    char *buf;
    scdcint_t buf_size;
    bool alloc;

    scdcint_t write_pos, read_pos;

    void *data;
    scdc_data_next_f *next;
};


#endif /* __SCDC_DATA_HH__ */
