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


#ifndef __REDIRECT_HANDLE_H__
#define __REDIRECT_HANDLE_H__


#include "scdc.h"

#include "redirect_config.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _redirect_handle_t
{
  int client;
  char id[256], uri[256];

  scdc_dataset_t dataset;

  void *ptr;

} redirect_handle_t;


/* client side */
int redirect_handle_create_scdc(redirect_handle_t *rh, const char *id, const char *uri);
void redirect_handle_destroy_scdc(redirect_handle_t *rh);

/* server side */
void redirect_handle_set_ptr(redirect_handle_t *rh, const void *ptr);
void redirect_handle_get_ptr(redirect_handle_t *rh, void **ptr);

/* client and server side */
void redirect_handle_get_id(redirect_handle_t *rh, char *id, rdint_t size);


#ifdef __cplusplus
}
#endif


#endif /* __REDIRECT_HANDLE_H__ */
