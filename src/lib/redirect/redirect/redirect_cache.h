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


#ifndef __REDIRECT_CACHE_H__
#define __REDIRECT_CACHE_H__


#include "scdc.h"

#include "redirect_config.h"
#include "redirect_param.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _redirect_cache_param_t
{
  int type;
  rdint_t size;
  void *buf;
  redirect_param_hash_t hash;
  rdint_t entry_index;

} redirect_cache_param_t;


typedef struct _redirect_cache_entry_t
{
  int type;
  rdint_t size;
  void *buf;
  int hash_set;
  redirect_param_hash_t hash;

} redirect_cache_entry_t;


typedef struct _redirect_cache_t
{
  int client;

  rdint_t nparams, next_param;
  redirect_cache_param_t params[REDIRECT_CACHE_PARAMS_MAX];

  rdint_t nentries;
  redirect_cache_entry_t entries[REDIRECT_CACHE_ENTRIES_MAX];

} redirect_cache_t;


/* client side */
void redirect_cache_create(redirect_cache_t *rch);
void redirect_cache_destroy(redirect_cache_t *rch);

void redirect_cache_register_param_dense(redirect_cache_t *rch, const char *id, int type, redirect_param_dense_t *param);

void redirect_cache_check(redirect_cache_t *rch, scdc_dataset_t ds, const char *uri, char *state, rdint_t *n);

/* server side */
void redirect_cache_init(redirect_cache_t *rch);
void redirect_cache_release(redirect_cache_t *rch);

int redirect_cache_check_cmd(redirect_cache_t *rch, const char *cmd, const char *params, scdc_result_t *result);
void redirect_cache_check_state(redirect_cache_t *rch, const char *state, rdint_t n);

rdint_t redirect_cache_put_entry(redirect_cache_t *rch, int type, rdint_t size, const void *buf);
void redirect_cache_update_entry(redirect_cache_t *rch, rdint_t entry);
rdint_t redirect_cache_find_entry(redirect_cache_t *rch, int type, rdint_t size, const void *buf);
rdint_t redirect_cache_find_entries(redirect_cache_t *rch, int type, rdint_t size, rdint_t *entries);
void *redirect_cache_get_entry_buf(redirect_cache_t *rch, rdint_t entry);
redirect_param_hash_t redirect_cache_get_entry_hash(redirect_cache_t *rch, rdint_t entry);
void redirect_cache_print(redirect_cache_t *rch);
void redirect_cache_clear(redirect_cache_t *rch);

/* client and server side */
void redirect_cache_reset_params(redirect_cache_t *rch);

rdint_t redirect_cache_get_next_param_entry_index(redirect_cache_t *rch);


#ifdef __cplusplus
}
#endif


#endif /* __REDIRECT_CACHE_H__ */
