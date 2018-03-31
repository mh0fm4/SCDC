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


#include <cstdio>
#include <cstdarg>
#include <string>

#include <mysql/mysql.h>

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_mysql.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-mysql: "


struct scdc_dataprov_mysql_data_t
{
  scdc_dataprov_mysql_data_t(MYSQL *mysql_):mysql(mysql_) { }

  MYSQL *mysql;
  pthread_mutex_t lock;
};


static int my_mysql_query(MYSQL *mysql, const char *query)
{
  SCDC_TRACE("mysql_query: '" << query << "'");

  int err = mysql_query(mysql, query);

  if (err) SCDC_FAIL("mysql_query: error: '" << mysql_error(mysql) << "'");

  return err;
}


static int my_mysql_query_printf(MYSQL *mysql, const char *query_format, ...)
{
  const int query_size = 512;
  char query[query_size];

  va_list argp;
  va_start(argp, query_format);
  vsnprintf(query, query_size, query_format, argp);
  va_end(argp);

  SCDC_TRACE("mysql_query: '" << query << "'");

  int err = mysql_query(mysql, query);

  if (err) SCDC_FAIL("mysql_query: error: '" << mysql_error(mysql) << "'");

  return err;
}


static int my_mysql_real_query(MYSQL *mysql, const char *query, unsigned long length)
{
  SCDC_TRACE("mysql_query: '" << string(query, min(length, 64UL)) << "...' (size: " << length << ")");

  int err = mysql_real_query(mysql, query, length);

  if (err) SCDC_FAIL("mysql_query: error: '" << mysql_error(mysql) << "'");

  return err;
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataset-mysql-store: "


#define TABLE_STORE_PREFIX  "store_v1_"
#define BLOB_PART_MAX_SIZE  2048


typedef struct _dataset_output_next_data_t
{
  scdc_dataprov_mysql_store *dataprov_mysql;

  string pwd, params;
  scdcint_t total_pos, total_size_left;

} dataset_output_next_data_t;


static scdcint_t dataset_output_next(scdc_dataset_output_t *output)
{
  dataset_output_next_data_t *data = static_cast<dataset_output_next_data_t *>(output->data);

  SCDC_TRACE_DATASET_OUTPUT(output, "dataset_output_next: ");

  output->current_size = 0;

  char *buf = static_cast<char *>(output->buf);
  scdcint_t buf_size = output->buf_size;

  scdc_dataprov_mysql_data_t *mysql_data = data->dataprov_mysql->aquire_access();

  while (data->total_size_left > 0 && buf_size > 0)
  {
    SCDC_TRACE("dataset_output_next: total_size_left: " << data->total_size_left << ", buf_size: " << buf_size);

    scdcint_t size = min(static_cast<scdcint_t>(BLOB_PART_MAX_SIZE), min(data->total_size_left, buf_size));

    if (my_mysql_query_printf(mysql_data->mysql, "SELECT SUBSTRING(data FROM %" scdcint_fmt " FOR %" scdcint_fmt ") FROM " TABLE_STORE_PREFIX "%s WHERE name='%s'", data->total_pos + 1, size, data->pwd.c_str(), data->params.c_str()))
    {
      SCDC_FAIL("dataset_output_next: query failed: getting data failed");
      data->dataprov_mysql->release_access(mysql_data);
      return SCDC_FAILURE;
    }

    MYSQL_RES *result = mysql_store_result(mysql_data->mysql);

    if (!result)
    {
      SCDC_FAIL("dataset_output_next: store result failed: '" << mysql_error(mysql_data->mysql) << "'");
      data->dataprov_mysql->release_access(mysql_data);
      return SCDC_FAILURE;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    unsigned long *lengths = mysql_fetch_lengths(result);

    size = lengths[0];

    SCDC_TRACE("dataset_output_next: get data of size " << size);

    memcpy(buf, row[0], size);

    mysql_free_result(result);

    buf += size;
    buf_size -= size;

    data->total_pos += size;
    data->total_size_left -= size;

    output->current_size += size;
  }

  data->dataprov_mysql->release_access(mysql_data);

  if (data->total_size_left <= 0)
  {
    SCDC_TRACE("dataset_output_next: done");

    output->next = 0;
    output->data = 0;

    delete data;
  }

  return SCDC_SUCCESS;
}


class scdc_dataset_mysql_store: public scdc_dataset
{
  public:
    scdc_dataset_mysql_store(scdc_dataprov *dataprov_)
      :scdc_dataset(dataprov_), admin(false) { };


    bool do_cmd_info(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_info: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      scdc_dataprov_mysql_store *dataprov_mysql = static_cast<scdc_dataprov_mysql_store *>(dataprov);

      scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

      if (my_mysql_query(mysql_data->mysql, "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'if_pimerge'"))
      {
        SCDC_FAIL("do_cmd_info: query failed: counting tables failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "counting tables failed");
        dataprov_mysql->release_access(mysql_data);
        return false;
      }

      MYSQL_RES *result = mysql_store_result(mysql_data->mysql);

      if (!result)
      {
        SCDC_FAIL("do_cmd_info: store result failed: '" << mysql_error(mysql_data->mysql) << "'");
        dataprov_mysql->release_access(mysql_data);
        return false;
      }

      dataprov_mysql->release_access(mysql_data);

      MYSQL_ROW row = mysql_fetch_row(result);

      SCDC_TRACE("do_cmd_info: tables: " << row[0]);

      SCDC_DATASET_OUTPUT_PRINTF(output, "admin: %s, tables: %s", (admin?"yes":"no"), row[0]);

      mysql_free_result(result);

      return true;
    }


    bool do_cmd_cd(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_cd: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      if (params == "ADMIN")
      {
        admin = true;
        return scdc_dataset::do_cmd_cd(params, input, output);
      }

      scdc_dataprov_mysql_store *dataprov_mysql = static_cast<scdc_dataprov_mysql_store *>(dataprov);

      if (!params.empty())
      {
        scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

        if (my_mysql_query_printf(mysql_data->mysql, "SHOW TABLES LIKE '" TABLE_STORE_PREFIX "%s'", params.c_str()))
        {
          SCDC_FAIL("do_cmd_cd: query failed: listing tables failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "listing tables failed");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        MYSQL_RES *result = mysql_store_result(mysql_data->mysql);

        if (!result)
        {
          SCDC_FAIL("do_cmd_cd: store result failed: '" << mysql_error(mysql_data->mysql) << "'");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        dataprov_mysql->release_access(mysql_data);

        if (mysql_num_rows(result) < 1)
        {
          mysql_free_result(result);

          SCDC_FAIL("do_cmd_cd: table '" << params << "' does not exist");
          SCDC_DATASET_OUTPUT_PRINTF(output, "table does not exist");
          return false;
        }

        mysql_free_result(result);
      }

      admin = false;
      return scdc_dataset::do_cmd_cd(params, input, output);
    }


    bool do_cmd_ls(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_ls: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      scdc_dataprov_mysql_store *dataprov_mysql = static_cast<scdc_dataprov_mysql_store *>(dataprov);

      if (admin || pwd.size() == 0)
      {
        SCDC_TRACE("do_cmd_ls: listing tables");

        scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

        if (my_mysql_query_printf(mysql_data->mysql, "SHOW TABLES LIKE '" TABLE_STORE_PREFIX "%%'"))
        {
          SCDC_FAIL("do_cmd_ls: query failed: listing tables failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "listing tables failed");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        MYSQL_RES *result = mysql_store_result(mysql_data->mysql);

        if (!result)
        {
          SCDC_FAIL("do_cmd_ls: store result failed: '" << mysql_error(mysql_data->mysql) << "'");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        dataprov_mysql->release_access(mysql_data);

        SCDC_DATASET_OUTPUT_PRINTF(output, "%llu|", mysql_num_rows(result));

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)))
        {
          SCDC_TRACE("table: '" << row[0] << "'");
          SCDC_DATASET_OUTPUT_PRINTF_APPEND(output, "%s|", row[0] + strlen(TABLE_STORE_PREFIX));
        }

        mysql_free_result(result);

      } else
      {
        SCDC_TRACE("do_cmd_put: listing entries of table '" << pwd << "'");

        scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

        if (my_mysql_query_printf(mysql_data->mysql, "SELECT name, format, size FROM " TABLE_STORE_PREFIX "%s", pwd.c_str()))
        {
          SCDC_FAIL("do_cmd_ls: query failed: listing entries failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "listing entries failed");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        MYSQL_RES *result = mysql_store_result(mysql_data->mysql);

        if (!result)
        {
          SCDC_FAIL("do_cmd_put: store result failed: '" << mysql_error(mysql_data->mysql) << "'");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        dataprov_mysql->release_access(mysql_data);

        SCDC_DATASET_OUTPUT_PRINTF(output, "%llu|", mysql_num_rows(result));

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)))
        {
          SCDC_TRACE("entry: name: '" << row[0] << "', format: '" << row[1] << "', size: " << row[2]);
          SCDC_DATASET_OUTPUT_PRINTF_APPEND(output, "%s:%s:%s|", row[0], row[1], row[2]);
        }

        mysql_free_result(result);
      }

      return true;
    }


    bool do_cmd_put(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_put: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      scdc_dataprov_mysql_store *dataprov_mysql = static_cast<scdc_dataprov_mysql_store *>(dataprov);

      if (params.empty())
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "empty path");
        return false;

      } else if (admin)
      {
        SCDC_TRACE("do_cmd_put: creating table '" << params << "'");

        scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

        if (my_mysql_query_printf(mysql_data->mysql, "CREATE TABLE " TABLE_STORE_PREFIX "%s("
          "name CHAR(64) NOT NULL UNIQUE, "
          "format CHAR(%d), "
          "size INT, "
          "data LONGBLOB, "
          "PRIMARY KEY (name))", params.c_str(), static_cast<int>(SCDC_FORMAT_MAX_SIZE)))
        {
          SCDC_FAIL("do_cmd_put: query failed: creating table failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "creating table failed");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        dataprov_mysql->release_access(mysql_data);

      } else if (pwd.size() == 0)
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "no table selected");
        return false;

      } else
      {
        SCDC_TRACE("do_cmd_put: creating entry '" << params << "'");

        scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

        if (my_mysql_query_printf(mysql_data->mysql, "INSERT INTO " TABLE_STORE_PREFIX "%s VALUES("
          "'%s', "
          "'%s', "
          "0, "
          "''"
          ")", pwd.c_str(), params.c_str(), input->format))
        {
          SCDC_FAIL("do_cmd_put: query failed: creating entry failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "creating entry failed");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        scdcint_t total_pos = 0;

        const scdcint_t max_stmt_size = 128;
        char query[max_stmt_size + 2 * BLOB_PART_MAX_SIZE + 1], *p; /* x*2+1 required for mysql_real_escape_string */

        while (1)
        {
          char *buf = static_cast<char *>(input->buf);
          scdcint_t current_pos = 0;

          while (current_pos < input->current_size)
          {
            scdcint_t size = min(static_cast<scdcint_t>(BLOB_PART_MAX_SIZE), input->current_size - current_pos);

            SCDC_TRACE("do_cmd_put: total: " << input->total_size << " at " << total_pos << ", current: " << size << " at " << current_pos);

            p = query;
            p += sprintf(p, "UPDATE " TABLE_STORE_PREFIX "%s SET size = %" scdcint_fmt ", data = CONCAT(data, '", pwd.c_str(), total_pos + size);
            p += mysql_real_escape_string(mysql_data->mysql, p, buf + current_pos, size);
            p += sprintf(p, "') WHERE name='%s'", params.c_str());

            if (my_mysql_real_query(mysql_data->mysql, query, p - query))
            {
              SCDC_FAIL("do_cmd_put: query failed: adding data failed");
              SCDC_DATASET_OUTPUT_PRINTF(output, "adding data failed");
              dataprov_mysql->release_access(mysql_data);
              return false;
            }

            current_pos += size;
            total_pos += size;
          }

          if (!input->next) break;

          SCDC_TRACE("do_cmd_put: next input");

          input->next(input);
        }

        dataprov_mysql->release_access(mysql_data);
      }

      return true;
    }


    bool do_cmd_get(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_get: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      scdc_dataprov_mysql_store *dataprov_mysql = static_cast<scdc_dataprov_mysql_store *>(dataprov);

      if (params.empty())
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "empty path");
        return false;

      } else if (pwd.size() == 0)
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "no table selected");
        return false;

      } else
      {
        SCDC_TRACE("do_cmd_get: getting entry '" << params << "'");

        scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

        if (my_mysql_query_printf(mysql_data->mysql, "SELECT format, size, OCTET_LENGTH(data) FROM " TABLE_STORE_PREFIX "%s WHERE name='%s'", pwd.c_str(), params.c_str()))
        {
          SCDC_FAIL("query failed: getting entry failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "getting entry failed");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        MYSQL_RES *result = mysql_store_result(mysql_data->mysql);

        if (!result)
        {
          SCDC_FAIL("do_cmd_get: store result failed: '" << mysql_error(mysql_data->mysql) << "'");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        dataprov_mysql->release_access(mysql_data);

        if (mysql_num_rows(result) != 1)
        {
          mysql_free_result(result);

          SCDC_FAIL("do_cmd_get: found " << mysql_num_rows(result) << " entries");
          SCDC_DATASET_OUTPUT_PRINTF(output, "found %llu entries", mysql_num_rows(result));
          return false;
        }

        MYSQL_ROW row = mysql_fetch_row(result);

        strncpy(output->format, row[0], SCDC_FORMAT_MAX_SIZE);

        sscanf(row[2], "%" scdcint_fmt, &output->total_size);

        output->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

        mysql_free_result(result);

        dataset_output_next_data_t *data = new dataset_output_next_data_t();

        data->dataprov_mysql = dataprov_mysql;
        data->pwd = pwd;
        data->params = params;
        data->total_pos = 0;
        data->total_size_left = output->total_size;

        output->next = dataset_output_next;
        output->data = data;

        /* get first chunk of data */
        output->next(output);
      }

      return true;
    }


    bool do_cmd_rm(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_rm: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      scdc_dataprov_mysql_store *dataprov_mysql = static_cast<scdc_dataprov_mysql_store *>(dataprov);

      if (params.empty())
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "empty path");
        return false;

      } else if (admin)
      {
        SCDC_TRACE("do_cmd_rm: deleting table '" << params << "'");

        scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

        if (my_mysql_query_printf(mysql_data->mysql, "DROP TABLE " TABLE_STORE_PREFIX "%s", params.c_str()))
        {
          SCDC_FAIL("do_cmd_rm: query failed: deleting table failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "deleting table failed");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        dataprov_mysql->release_access(mysql_data);

      } else if (pwd.size() == 0)
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "no table selected");
        return false;

      } else
      {
        SCDC_TRACE("do_cmd_rm: deleting entry '" << params << "'");

        scdc_dataprov_mysql_data_t *mysql_data = dataprov_mysql->aquire_access();

        if (my_mysql_query_printf(mysql_data->mysql, "DELETE FROM " TABLE_STORE_PREFIX "%s WHERE name='%s'", pwd.c_str(), params.c_str()))
        {
          SCDC_FAIL("do_cmd_rm: query failed: deleting entry failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "deleting entry failed");
          dataprov_mysql->release_access(mysql_data);
          return false;
        }

        dataprov_mysql->release_access(mysql_data);
      }

      return true;
    }

  private:
    bool admin;
};


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-mysql: "


scdc_dataprov_mysql_store::scdc_dataprov_mysql_store()
  :scdc_dataprov("mysql"), mysql_data(0)
{
}


bool scdc_dataprov_mysql_store::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  const char *mysql_conf;
  if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &mysql_conf) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("open: getting MySQL DB credentials");
    ret = false;
    goto do_quit;
  }

  SCDC_TRACE("open: mysql_conf: '" << mysql_conf << "'");

  if (!scdc_dataprov::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    stringlist confs(' ', mysql_conf);
    string host, user, password, database;

    confs.front_pop(host);
    confs.front_pop(user);
    confs.front_pop(password);
    confs.front_pop(database);

    SCDC_TRACE("mysql_confs: host: '" << host << "', user: '" << user << "', password: '" << password << "', database: '" << database << "'");

    MYSQL *mysql = mysql_init(NULL);

    if (mysql == NULL)
    {
      SCDC_ERROR("open: initialize MySQL DB");
      ret = false;
      goto do_close;
    }

    if (mysql_real_connect(mysql, host.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, NULL, 0) == NULL)
    {
      SCDC_ERROR("open: connecting to MySQL DB");
      ret = false;
      goto do_close;
    }

    mysql_data = new scdc_dataprov_mysql_data_t(mysql);

    pthread_mutex_init(&mysql_data->lock, NULL);

    dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_mysql_store::do_cmd_pwd));
    dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_mysql_store::do_cmd_info));
    dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_mysql_store::do_cmd_cd));
    dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_mysql_store::do_cmd_ls));
    dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_mysql_store::do_cmd_put));
    dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_mysql_store::do_cmd_get));
    dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_mysql_store::do_cmd_rm));

do_close:
    if (!ret && mysql == NULL) mysql_close(mysql);

    if (!ret) scdc_dataprov::close();
  }

do_quit:
  return ret;
}


void scdc_dataprov_mysql_store::close()
{
  SCDC_TRACE("close:");

  pthread_mutex_lock(&mysql_data->lock);

  mysql_close(mysql_data->mysql);

  pthread_mutex_unlock(&mysql_data->lock);

  pthread_mutex_destroy(&mysql_data->lock);

  delete mysql_data; mysql_data = 0;

  scdc_dataprov::close();
}


scdc_dataset *scdc_dataprov_mysql_store::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

  scdc_dataset *dataset = 0;
  
  if (config_open(path, path_size, output, &dataset)) return dataset;

  scdc_dataset_mysql_store *dataset_mysql = new scdc_dataset_mysql_store(this);

  if (path && !dataset_mysql->do_cmd_cd(string(path, path_size).c_str(), NULL, output))
  {
    SCDC_FAIL("dataset_open: do_cmd_cd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
    delete dataset_mysql;
    return 0;
  }

  SCDC_TRACE("dataset_open: return: '" << dataset_mysql << "'");

  return dataset_mysql;
}


void scdc_dataprov_mysql_store::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close: '" << dataset << "'");

  if (config_close(dataset, output)) return;

  delete dataset;

  SCDC_TRACE("dataset_close: return");
}


scdc_dataprov_mysql_data_t *scdc_dataprov_mysql_store::aquire_access()
{
  pthread_mutex_lock(&mysql_data->lock);

  return mysql_data;
}

void scdc_dataprov_mysql_store::release_access(scdc_dataprov_mysql_data_t *mysql_data)
{
  pthread_mutex_unlock(&mysql_data->lock);
}


#undef SCDC_LOG_PREFIX
