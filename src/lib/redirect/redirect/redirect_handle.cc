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

#include "common.h"
#include "redirect_handle.h"


#define TRACE_PREFIX  "redirect_handle: "


#define TRACE_DATA  0

#ifndef HAVE_TRACE
# undef TRACE_DATA
#endif


/* client side */

int redirect_handle_create_scdc(redirect_handle_t *rh, const char *id, const char *uri)
{
  rh->client = 1;

  strncpy(rh->id, id, sizeof(rh->id));
  strncpy(rh->uri, uri, sizeof(rh->uri));

  rh->dataset = scdc_dataset_open(rh->uri);

  if (rh->dataset == SCDC_DATASET_NULL)
  {
    TRACE_F("%s: failed", __func__);
    return 0;
  }

  rh->ptr = 0;

  return 1;
}


void redirect_handle_destroy_scdc(redirect_handle_t *rh)
{
  rh->id[0] = '\0';
  rh->uri[0] = '\0';

  if (rh->dataset != SCDC_DATASET_NULL)
  {
    scdc_dataset_close(rh->dataset);
    rh->dataset = SCDC_DATASET_NULL;
  }

  rh->ptr = 0;
}


/* server side */

void redirect_handle_set_ptr(redirect_handle_t *rh, const void *ptr)
{
  rh->ptr = const_cast<void *>(ptr);
}


void redirect_handle_get_ptr(redirect_handle_t *rh, void **ptr)
{
  *ptr = rh->ptr;
}


/* client and server side */

void redirect_handle_get_id(redirect_handle_t *rh, char *id, rdint_t size)
{
  strncpy(id, rh->id, size);
}
