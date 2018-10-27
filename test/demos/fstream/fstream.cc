/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
 *  Copyright (C) 2017 Eric Kunze
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


#include <iostream>
#include <string>

#include "scdc.h"
#include "fstream_scdc"


using namespace std;


void ifstream_test(const char *file)
{
  scdc::ifstream ifs;
  string line;

  ifs.open(file, ios_base::in);
  int i = 1;
  while (getline(ifs, line)) {
    cout << i << ": " << line << '\n';
    i++;
  }
  ifs.close();
}


int main(int argc, char** argv)
{
  scdc_init(SCDC_INIT_DEFAULT);

  scdc_dataprov_t dpFS = scdc_dataprov_open("storeFS", "fs", "store/");

  const char *uri = "scdc:/storeFS/test.in";

  ifstream_test(uri);

  scdc_dataprov_close(dpFS);

  scdc_release();

  return 0;
}
