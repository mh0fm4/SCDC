/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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


#ifndef __NODEPORT_HH__
#define __NODEPORT_HH__


#include <string>

#include "args.hh"
#include "dataprov.hh"
#include "transport.hh"
#include "compcoup.hh"


class scdc_nodeport
{
  public:
    static bool init();
    static void release();

    static bool authority(const char *conf, scdc_args *args, std::string &auth);
    static bool supported(const char *uri, scdc_args *args);

    scdc_nodeport(const char *type_, scdcint_t supported_modes_);
    virtual ~scdc_nodeport();

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);
    bool open_config(std::string &conf, scdc_args *args);

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    virtual bool start(scdcint_t mode);
    virtual bool stop();
    virtual bool cancel(bool interrupt);
/*    virtual bool resume();*/

    void set_dataprovs(scdc_dataprov_pool *dataprovs) { if (compcoup) compcoup->set_dataprovs(dataprovs); };

    std::string get_type() { return type; }
    scdc_compcoup *get_compcoup() { return compcoup; }

  protected:
    const std::string type;
    const scdcint_t supported_modes;
    scdc_compcoup *compcoup;

    scdc_args *open_args;
    scdcint_t open_args_refcount;

    scdc_nodeport_cmd_handler_args_t cmd_handler_args;
    scdc_nodeport_loop_handler_args_t loop_handler_dummy_args;
    scdcint_t max_connections;
    std::string cfg_compression;

    scdc_args *open_args_init(scdc_args *args) { if (!open_args) open_args = new scdc_args(args); ++open_args_refcount; return open_args; }
    void open_args_clear() { open_args->clear_args_data(); }
    void open_args_release() { --open_args_refcount; if (open_args_refcount == 0) { delete open_args; open_args = 0; } }
};


#endif /* __NODEPORT_HH__ */
