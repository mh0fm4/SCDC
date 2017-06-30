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


#define SCDC_TRACE_NOT  !SCDC_TRACE_NODEPORT

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "nodeport.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "nodeport: "


bool scdc_nodeport::init()
{
  return true;
}


void scdc_nodeport::release()
{
}


bool scdc_nodeport::authority(const char *conf, scdc_args *args, std::string &auth)
{
  auth.clear();

  return true;
}


bool scdc_nodeport::supported(const char *uri, scdc_args *args)
{
  return false;
}


scdc_nodeport::scdc_nodeport(const char *type_, scdcint_t supported_modes_)
  :type(type_), supported_modes(supported_modes_), compcoup(0), open_args(0), open_args_refcount(0)
{
  cmd_handler_args.handler = 0;
  cmd_handler_args.data = 0;

  loop_handler_dummy_args.handler = 0;
  loop_handler_dummy_args.data = 0;

  max_connections = -1;
}


scdc_nodeport::~scdc_nodeport()
{
}


bool scdc_nodeport::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  done = true;
  bool ret = true;

  if (conf == "cmd_handler")
  {
    if (args->get<scdc_nodeport_cmd_handler_args_t>(SCDC_ARGS_TYPE_NODEPORT_CMD_HANDLER, &cmd_handler_args, true) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting command handler");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: cmd_handler: " << reinterpret_cast<void *>(cmd_handler_args.handler) << " / " << static_cast<void *>(cmd_handler_args.data));
    }

  } else if (conf == "max_connections")
  {
    if (args->get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &max_connections) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting max. connections");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: max_connections: " << max_connections);
    }

  } else if (conf.compare(0, 4, "zlib") == 0)
  {
    SCDC_TRACE("open_config_conf: cfg_compression: '" << cfg_compression << "'");
    cfg_compression = conf;

  } else done = false;

  return ret;
}


bool scdc_nodeport::open_config(std::string &conf, scdc_args *args)
{
  bool done;
  stringlist cl(conf);

  while (cl.size() > 0)
  {
    const string &c = cl.front_pop();

    if (!open_config_conf(c, args, done))
    {
      SCDC_FAIL("open_config: processing configuration '" << c << "' failed");
      return false;
    }

    if (!done) SCDC_INFO("open_config: ignoring unknown configuration '" << c << "'");
  }

  return true;
}


bool scdc_nodeport::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  args = open_args_init(args);

  string confs(conf);

  if (conf && !open_config(confs, args))
  {
    SCDC_FAIL("open: processing configuration failed");
    ret = false;
    goto do_quit;
  }

  if (args->get<scdc_nodeport_loop_handler_args_t>(SCDC_ARGS_TYPE_NODEPORT_LOOP_HANDLER_DUMMY, &loop_handler_dummy_args, true) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("open: getting loop handler dummy");
    ret = false;
    goto do_quit;

  } else
  {
    SCDC_TRACE("open_config_conf: loop_handler_dummy: " << reinterpret_cast<void *>(loop_handler_dummy_args.handler) << " / " << static_cast<void *>(loop_handler_dummy_args.data));
  }

  open_args_clear();

do_quit:
  if (!ret) open_args_release();

  return ret;
}


void scdc_nodeport::close()
{
  SCDC_TRACE("close:");

  open_args_release();
}


bool scdc_nodeport::start(scdcint_t mode)
{
  if (!(supported_modes & mode))
  {
    SCDC_TRACE("start: mode '" << mode << "' not supported");
    return false;
  }

  if (compcoup) compcoup->set_compression(cfg_compression.c_str());

  SCDC_TRACE("cfg_compression: '" << cfg_compression << "'");

  return true;
}


bool scdc_nodeport::stop()
{
  return true;
}


bool scdc_nodeport::cancel(bool interrupt)
{
  return true;
}


/*bool scdc_nodeport::resume()
{
  return true;
}*/


#undef SCDC_LOG_PREFIX
