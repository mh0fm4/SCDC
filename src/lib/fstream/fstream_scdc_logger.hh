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


#ifndef __fstream_scdc_logger_hh__
#define __fstream_scdc_logger_hh__

#include <iostream>
#include <string>

namespace FSTREAM_SCDC_NAMESPACE _GLIBCXX_VISIBILITY(default)
{
  class fstream_scdc_logger {
    public:
      fstream_scdc_logger();

      static void TRACE(const char *c, const char* f, std::string msg) {
        std::cout << c << "::" << f << ": " << msg << std::endl;
      }

      static void ERROR(const char *c, const char* f, std::string msg) {
        std::cerr << c << "::" << f << ": " << msg << std::endl;
      }

      static void TRACE(const char* f, std::string msg) {
        std::cout << f << ": " << msg << std::endl;
      }

      static void ERROR(const char* f, std::string msg) {
        std::cerr << f << ": " << msg << std::endl;
      }

    private:
  };

} // namespace FSTREAM_SCDC_NAMESPACE

#endif /* __fstream_scdc_logger_hh__ */
