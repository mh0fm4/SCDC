#  
#  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
#  
#  This file is part of the Simulation Component and Data Coupling (SCDC) library.
#  
#  The SCDC library is free software: you can redistribute it and/or
#  modify it under the terms of the GNU Lesser Public License as published
#  by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  The SCDC library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser Public License for more details.
#  
#  You should have received a copy of the GNU Lesser Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#  


from distutils.core import setup, Extension
import os


pymod_macros = [(m.split('=')[0],m.split('=')[1]) for m in os.getenv('PYMOD_MACROS', '').split()]
pymod_incdirs = str.split(os.getenv('PYMOD_INCDIRS', ''))
pymod_libs = str.split(os.getenv('PYMOD_LIBS', ''))
pymod_libdirs = str.split(os.getenv('PYMOD_LIBDIRS', ''))
pymod_sources = str.split(os.getenv('PYMOD_SOURCES', ''))
pymod_depends = str.split(os.getenv('PYMOD_DEPENDS', ''))

#print("PYMOD_MACROS: ", pymod_macros)
#print("PYMOD_INCDIRS: ", pymod_incdirs)
#print("PYMOD_LIBS: ", pymod_libs)
#print("PYMOD_LIBDIRS: ", pymod_libdirs)
#print("PYMOD_SOURCES: ", pymod_sources)
#print("PYMOD_DEPENDS: ", pymod_depends)

scdcmodule = Extension('scdcmod',
  define_macros = [] + pymod_macros,
  include_dirs = [] + pymod_incdirs,
  libraries = [] + pymod_libs,
  library_dirs = [] + pymod_libdirs,
  sources = [] + pymod_sources,
  depends = [] + pymod_depends)

#print("define_macros: ", scdcmodule.define_macros)
#print("include_dirs: ", scdcmodule.include_dirs)
#print("libraries: ", scdcmodule.libraries)
#print("library_dirs: ", scdcmodule.library_dirs)
#print("sources: ", scdcmodule.sources)
#print("depends: ", scdcmodule.depends)

setup (name = 'SCDCMOD',
  version = '1.0',
  description = 'This is the SCDC package for Python.',
  ext_modules = [scdcmodule])
