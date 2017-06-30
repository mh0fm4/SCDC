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


#ifndef __NETLIB_COMMON_H__
#define __NETLIB_COMMON_H__


#define NETLIB_TRANS_IS_TRANS(_trans_)     ((_trans_) == 'T' || (_trans_) == 't')
#define NETLIB_TRANS_IS_NONTRANS(_trans_)  ((_trans_) == 'N' || (_trans_) == 'n')

#define NETLIB_TRANS_GET_NROWS(_trans_, _m_, _n_)  (NETLIB_TRANS_IS_NONTRANS(_trans_)?(_m_):(_n_))
#define NETLIB_TRANS_GET_NCOLS(_trans_, _m_, _n_)  (NETLIB_TRANS_IS_NONTRANS(_trans_)?(_n_):(_m_))

#define NETLIB_UPLO_IS_UPPER(_uplo_)  ((_uplo_) == 'U' || (_uplo_) == 'u')
#define NETLIB_UPLO_IS_LOWER(_uplo_)  ((_uplo_) == 'L' || (_uplo_) == 'l')

#define NETLIB_DIAG_IS_UNIT(_diag_)     ((_diag_) == 'U' || (_diag_) == 'u')
#define NETLIB_DIAG_IS_NONUNIT(_diag_)  ((_diag_) == 'N' || (_diag_) == 'n')

#define NETLIB_SIDE_IS_LEFT(_side_)   ((_side_) == 'L' || (_side_) == 'l')
#define NETLIB_SIDE_IS_RIGHT(_side_)  ((_side_) == 'R' || (_side_) == 'r')


#endif /* __NETLIB_COMMON_H__ */
