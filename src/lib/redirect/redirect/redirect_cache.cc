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
#include <cstdlib>
#include <cstring>

#include "redirect_config.h"
#include "redirect_param.h"
#include "redirect_types.h"
#include "redirect_cache.h"
#include "common.h"


#define TRACE_PREFIX  "redirect_cache: "


#define TRACE_DATA  0

#ifndef HAVE_TRACE
# undef TRACE_DATA
#endif


void redirect_cache_create(redirect_cache_t *rch)
{
  TRACE_F(" ");

  rch->client = 1;

  redirect_cache_reset_params(rch);

  rch->nentries = 0;
}


void redirect_cache_destroy(redirect_cache_t *rch)
{
  TRACE_F(" ");

  redirect_cache_clear(rch);
}


void redirect_cache_register_param_dense(redirect_cache_t *rch, const char *id, int type, redirect_param_dense_t *param)
{
  TRACE_F("id: %s, type: %d, size: %" rdint_fmt, id, type, param->size);

  if (rch->nparams < REDIRECT_CACHE_PARAMS_MAX)
  {
    rch->params[rch->nparams].type = type;
    rch->params[rch->nparams].size = param->size;
    rch->params[rch->nparams].buf = param->buf;
    rch->params[rch->nparams].hash = 0;

    ++rch->nparams;
  }
}


void redirect_cache_check(redirect_cache_t *rch, scdc_dataset_t ds, const char *uri, char *state, rdint_t *n)
{
  TRACE_F("ds: %" scdc_dataset_fmt ", uri: %s", ds, uri);

  char cmd[REDIRECT_CALL_URI_MAX_SIZE + REDIRECT_CALL_OP_MAX_SIZE + REDIRECT_CALL_CACHE_REQUEST_MAX_SIZE];
  rdint_t cmd_size = 0;

  if (ds == SCDC_DATASET_NULL) cmd_size += sprintf(cmd, "%s ", uri);

  rdint_t i, j;

  cmd_size += sprintf(cmd + cmd_size, "cache_check %" rdint_fmt ":", rch->nparams);
  for (i = 0; i < rch->nparams; ++i) cmd_size += sprintf(cmd + cmd_size, "t%d:s%" rdint_fmt ":", rch->params[i].type, rch->params[i].size);

  TRACE_F("cmd: '%s'", cmd);

  if (scdc_dataset_cmd(ds, cmd, 0, 0) != SCDC_SUCCESS)
  {
    TRACE_F("failed");
    return;
  }

  const char *s = scdc_last_result();
  rdint_t state_size = 0;

  for (i = 0; i < rch->nparams; ++i)
  {
    rdint_t nentries = 0;
    sscanf(s, "%" rdint_fmt, &nentries);
    s = strchr(s, ':') + 1;

    TRACE_F("param %" rdint_fmt ": type: %d, size: %" rdint_fmt, i, rch->params[i].type, rch->params[i].size);

    rch->params[i].entry_index = -1;

    if (nentries > 0)
    {
      /* compute parameter hash */
      redirect_param_dense_t p;
      p.buf = rch->params[i].buf;
      p.size = rch->params[i].size;
      rch->params[i].hash = redirect_param_hash_dense(&p);

      TRACE_F("param %" rdint_fmt ": hash %" redirect_param_hash_fmt, i, rch->params[i].hash);

      /* check against server hashes */
      for (j = 0; j < nentries; ++j)
      {
        redirect_param_hash_t entry_hash;
        rdint_t entry_index = -1;
        sscanf(s, "%" redirect_param_hash_fmt_scanf ":%" rdint_fmt, &entry_hash, &entry_index);
        s = strchr(s, ':') + 1;
        s = strchr(s, ':') + 1;

        if (entry_hash == rch->params[i].hash)
        {
          rch->params[i].entry_index = entry_index;
        }
      }
    }

    if (rch->params[i].entry_index >= 0) TRACE_F("param %" rdint_fmt ": matching cache entry %" rdint_fmt, i, rch->params[i].entry_index);
    else TRACE_F("param %" rdint_fmt ": no matching cache entry", i);


    state_size += sprintf(state + state_size, "%" rdint_fmt ":", rch->params[i].entry_index);
  }

  *n = state_size;

  TRACE_F("return: state: '%.*s'", static_cast<int>(*n), state);
}


void redirect_cache_init(redirect_cache_t *rch)
{
  TRACE_F(" ");

  rch->client = 0;

  redirect_cache_reset_params(rch);

  rch->nentries = 0;
}


void redirect_cache_release(redirect_cache_t *rch)
{
  TRACE_F(" ");

  redirect_cache_clear(rch);
}


int redirect_cache_check_cmd(redirect_cache_t *rch, const char *cmd, const char *params, scdc_result_t *result)
{
  TRACE_F("cmd: '%s', params: '%s', result: %p", cmd, params, result);

  if (strcmp(cmd, "cache_check") != 0) return 0;

  redirect_cache_reset_params(rch);

  const char *s = params;
  char *r = SCDC_RESULT_STR(result);
  rdint_t r_size = 0;

  sscanf(s, "%" rdint_fmt ":", &rch->nparams);
  s = strchr(s, ':') + 1;
  rdint_t i, j;
  for (i = 0; i < rch->nparams; ++i)
  {
    sscanf(s, "t%d:s%" rdint_fmt ":", &rch->params[i].type, &rch->params[i].size);
    s = strchr(s, ':') + 1;
    s = strchr(s, ':') + 1;
    TRACE_F("param %" rdint_fmt ": type: %d, size: %" rdint_fmt, i, rch->params[i].type, rch->params[i].size);

    rch->params[i].buf = 0;
    rch->params[i].entry_index = 0;

    rdint_t entries[REDIRECT_CACHE_FIND_ENTRIES_MAX];
    rdint_t nentries = redirect_cache_find_entries(rch, rch->params[i].type, rch->params[i].size, entries);

    r_size += sprintf(r + r_size, "%" rdint_fmt ":", nentries);

    for (j = 0; j < nentries; ++j)
    {
      redirect_param_hash_t h = redirect_cache_get_entry_hash(rch, entries[j]);
      r_size += sprintf(r + r_size, "%" redirect_param_hash_fmt ":%" rdint_fmt ":", h, entries[j]);
    }
  }

  TRACE_F("return: %d, result: '%s'", 1, r);

  return 1;
}


void redirect_cache_check_state(redirect_cache_t *rch, const char *state, rdint_t state_size)
{
  TRACE_F("state: '%.*s'", static_cast<int>(state_size), state);

  const char *s = state;

  rdint_t i;
  for (i = 0; i < rch->nparams; ++i)
  {
    sscanf(s, "%" rdint_fmt ":", &rch->params[i].entry_index);
    s = strchr(s, ':') + 1;
  }
}


rdint_t redirect_cache_put_entry(redirect_cache_t *rch, int type, rdint_t size, const void *buf)
{
  rdint_t entry_index = redirect_cache_find_entry(rch, type, size, buf);

  if (entry_index >= 0)
  {
    TRACE_F("entry [%d,%" rdint_fmt ",%p] / %" rdint_fmt " already in cache", type, size, buf, entry_index);
    redirect_cache_update_entry(rch, entry_index);
    return entry_index;
  }

  if (rch->nentries >= REDIRECT_CACHE_ENTRIES_MAX)
  {
    TRACE_F("max. number of entries %" rdint_fmt " in cache", rch->nentries);
    // FIXME: cache replacement strategy: FIFO, LRU, LFU
    return -1;
  }

  entry_index = rch->nentries;

  rch->entries[entry_index].type = type;
  rch->entries[entry_index].size = size;
  rch->entries[entry_index].buf = const_cast<void *>(buf);
  rch->entries[entry_index].hash_set = 0;

  ++rch->nentries;

  return entry_index;
}


void redirect_cache_update_entry(redirect_cache_t *rch, rdint_t entry)
{
  rch->entries[entry].hash_set = 0;
}


rdint_t redirect_cache_find_entry(redirect_cache_t *rch, int type, rdint_t size, const void *buf)
{
  rdint_t i;
  for (i = 0; i < rch->nentries; ++i)
  {
    if (rch->entries[i].type != type || rch->entries[i].size != size || rch->entries[i].buf != buf) continue;

    return i;
  }
  
  return -1;
}


rdint_t redirect_cache_find_entries(redirect_cache_t *rch, int type, rdint_t size, rdint_t *entries)
{
  rdint_t nentries = 0;

  rdint_t i;
  for (i = 0; i < rch->nentries; ++i)
  {
    if (rch->entries[i].type != type || rch->entries[i].size != size) continue;

    entries[nentries] = i;
    ++nentries;

    if (nentries >= REDIRECT_CACHE_FIND_ENTRIES_MAX) break;
  }
  
  return nentries;
}


void *redirect_cache_get_entry_buf(redirect_cache_t *rch, rdint_t entry)
{
  return rch->entries[entry].buf;
}


redirect_param_hash_t redirect_cache_get_entry_hash(redirect_cache_t *rch, rdint_t entry)
{
  if (!rch->entries[entry].hash_set)
  {
    redirect_param_dense_t p;
    p.buf = rch->entries[entry].buf;
    p.size = rch->entries[entry].size;
    rch->entries[entry].hash = redirect_param_hash_dense(&p);
    rch->entries[entry].hash_set = 1;
  }

  return rch->entries[entry].hash;
}


void redirect_cache_print(redirect_cache_t *rch)
{
  TRACE_F(" ");
}


void redirect_cache_clear(redirect_cache_t *rch)
{
  TRACE_F(" ");

  rdint_t i;
  for (i = 0; i < rch->nentries; ++i) free(rch->entries[i].buf);

  rch->nentries = 0;
}


void redirect_cache_reset_params(redirect_cache_t *rch)
{
  rch->nparams = 0;
  rch->next_param = 0;
}


rdint_t redirect_cache_get_next_param_entry_index(redirect_cache_t *rch)
{
  if (rch->next_param >= rch->nparams) return -1;

  ++rch->next_param;

  return rch->params[rch->next_param - 1].entry_index;
}
