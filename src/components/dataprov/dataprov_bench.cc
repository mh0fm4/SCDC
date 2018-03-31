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


#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>

#include "config.hh"
#include "log.hh"
#include "common.hh"
#include "dataprov_bench.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-bench: "


typedef struct _dataset_output_next_data_t
{
  scdcint_t total_size_left;

  string format, content;
  FILE *file;

} dataset_output_next_data_t;


static scdcint_t dataset_output_next(scdc_dataset_output_t *output)
{
  dataset_output_next_data_t *data = static_cast<dataset_output_next_data_t *>(output->data);

  SCDC_TRACE_DATASET_OUTPUT(output, "dataset_output_next: ");

  /* min of data left and buffer size */
  scdcint_t size = min(data->total_size_left, SCDC_DATASET_INOUT_BUF_SIZE(output));

  char *buf = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(output));

  strncpy(output->format, (data->format == "ascii")?"text":"data", SCDC_FORMAT_MAX_SIZE);

  SCDC_TRACE("dataset_output_next: get data of size = " << size);

  if (data->file)
  {
    size = fread(buf, 1, size, data->file);

    if (data->format == "ascii")
    {
      if (data->content == "random")
      {
        for (scdcint_t i = 0; i < size; ++i) buf[i] = 32 + buf[i] % (126 - 32 + 1);

      } else if (data->content == "zero")
      {
        for (scdcint_t i = 0; i < size; ++i) buf[i] += '0';
      }
    }

  } else
  {
    if (data->format == "binary")
    {
      if (data->content == "zero")
      {
        memset(buf, 0, size);

      } else if (data->content == "random")
      {
        for (scdcint_t i = 0; i < size; ++i) buf[i] = random() & 0xFF;
      }

    } else if (data->format == "ascii")
    {
      if (data->content == "zero")
      {
        memset(buf, '0', size);

      } else if (data->content == "random")
      {
        for (scdcint_t i = 0; i < size; ++i) buf[i] = 32 + random() % (126 - 32 + 1);
      }
    }
  }

  SCDC_DATASET_INOUT_BUF_CURRENT(output) = size;

  data->total_size_left -= size;

  if (data->total_size_left <= 0)
  {
    SCDC_TRACE("dataset_output_next: done");

    output->next = 0;
    output->data = 0;

    if (data->file) fclose(data->file);
    data->file = 0;

    delete data;
  }

  return SCDC_SUCCESS;
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-bench: "


class scdc_dataset_bench: public scdc_dataset
{
  public:
    string format, content, mode;

    scdc_dataset_bench(scdc_dataprov *dataprov_)
      :scdc_dataset(dataprov_), format("binary"), content("zero") { }


    bool do_cmd_info(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("dataset_bench: do_cmd_info");

      return scdc_dataset::do_cmd_info(params, input, output);
    }


    bool do_cmd_cd(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("dataset_bench: do_cmd_cd: '" << params << "'");

      stringlist pl('/', params);

      string p;
      
      while (pl.front_pop(p))
      {
        if (p == "binary") format = "binary";
        else if (p == "ascii") format = "ascii";
        else if (p == "zero") content = "zero";
        else if (p == "random") content = "random";
        else if (p == "memory") mode = "memory";
        else if (p == "file") mode = "file";
        else
        {
          SCDC_FAIL("invalid path '" << p << "'");
          SCDC_DATASET_OUTPUT_PRINTF(output, "error: invalid path '%s'", p.c_str());
          return false;
        }
      }

      return scdc_dataset::do_cmd_cd((format + "/" + content + "/" + mode).c_str(), input, output);
    }


    bool do_cmd_ls(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("dataset_bench: do_cmd_ls: params: '" << params << "'");

      SCDC_DATASET_OUTPUT_PRINTF(output, "binary|d|ascii|d|zero|d|random|d|memory|d|file|d|");

      return true;
    }


    bool do_cmd_put(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("dataset_bench: do_cmd_put: '" << params << "'");

      string p = trim(params);

      scdcint_t total_size = (p.size() > 0)?atoll(p.c_str()):-1;

      FILE *file = 0;

      if (mode == "file") file = fopen("/dev/null", "w");

      bool ret = true;

      while (ret)
      {
        /* min of data left and current size */
        scdcint_t size = min(total_size, SCDC_DATASET_INOUT_BUF_CURRENT(input));

        SCDC_TRACE("do_cmd_put: put data of size = " << size);

        if (file) size = fwrite(SCDC_DATASET_INOUT_BUF_PTR(input), 1, size, file);

        total_size -= size;

        if (!input->next || total_size == 0) break;

        ret = input->next(input);
      }

      if (file) fclose(file);

      return true;
    }


    bool do_cmd_get(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("dataset_bench: do_cmd_get: '" << params << "'");

      scdcint_t total_size;

      stringstream(params) >> total_size;

      SCDC_TRACE("dataset_bench: do_cmd_get: start get of size = " << total_size);

      output->total_size = total_size;
      output->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

      dataset_output_next_data_t *data = new dataset_output_next_data_t();

      data->total_size_left = total_size;
      data->format = format;
      data->content = content;

      if (mode == "file")
      {
        if (content == "zero") data->file = fopen("/dev/zero", "r");
        else if (content == "random") data->file = fopen("/dev/urandom", "r");
      }

      output->next = dataset_output_next;
      output->data = data;

      /* get first chunk of data */
      output->next(output);

      return true;
    }
};


scdc_dataprov_bench::scdc_dataprov_bench()
  :scdc_dataprov("bench")
{
}


bool scdc_dataprov_bench::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  if (!scdc_dataprov::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_bench::do_cmd_pwd));
    dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_bench::do_cmd_info));
    dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_bench::do_cmd_cd));
    dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_bench::do_cmd_ls));
    dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_bench::do_cmd_put));
    dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_bench::do_cmd_get));
  }

  return ret;
}


void scdc_dataprov_bench::close()
{
  SCDC_TRACE("close:");

  scdc_dataprov::close();
}


scdc_dataset *scdc_dataprov_bench::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

  scdc_dataset *dataset = 0;
  
  if (config_open(path, path_size, output, &dataset)) return dataset;

  scdc_dataset_bench *dataset_bench = new scdc_dataset_bench(this);

  if (path && !dataset_bench->do_cmd_cd(string(path, path_size).c_str(), NULL, output))
  {
    SCDC_FAIL("dataset_open: do_cmd_cd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
    delete dataset_bench;
    return 0;
  }

  SCDC_TRACE("dataset_open: return: '" << dataset_bench << "'");

  return dataset_bench;
}


void scdc_dataprov_bench::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close: '" << dataset << "'");

  if (config_close(dataset, output)) return;

  delete dataset;

  SCDC_TRACE("dataset_close: return");
}


#undef SCDC_LOG_PREFIX
