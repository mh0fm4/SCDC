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


#ifndef __REDIRECT_CALL_BYTES_TCC__
#define __REDIRECT_CALL_BYTES_TCC__


/* plain bytes */

void redirect_call_put_input_param_bytes(redirect_call_t *rc, const char *id, const void *b, rdint_t n)
{
  redirect_call_put_input_conf_rdint(rc, "b", n);
  redirect_call_put_input_param_dense_val<char>(rc, id, static_cast<const char *>(b), n);
}


void redirect_call_get_input_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n)
{
  redirect_call_get_input_conf_rdint(rc, "b", n);
  redirect_call_get_input_param_dense_val<char>(rc, id, reinterpret_cast<char **>(b), *n);
}


void redirect_call_put_output_param_bytes(redirect_call_t *rc, const char *id, const void *b, rdint_t n)
{
  redirect_call_put_output_conf_rdint(rc, "b", n);
  redirect_call_put_output_param_dense_val<char>(rc, id, static_cast<const char *>(b), n);
}


void redirect_call_get_output_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n)
{
  redirect_call_get_output_conf_rdint(rc, "b", n);
  redirect_call_get_output_param_dense_val<char>(rc, id, reinterpret_cast<char **>(b), *n);
}


void redirect_call_put_inout_param_bytes(redirect_call_t *rc, const char *id, const void *b, rdint_t n)
{
  redirect_call_put_input_conf_rdint(rc, "b", n);
  redirect_call_put_inout_param_dense_val<char>(rc, id, static_cast<const char *>(b), n);
}


void redirect_call_get_inout_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n)
{
  redirect_call_get_input_conf_rdint(rc, "b", n);
  redirect_call_get_inout_param_dense_val<char>(rc, id, reinterpret_cast<char **>(b), *n);
}


#endif /* __REDIRECT_CALL_BYTES_TCC__ */
