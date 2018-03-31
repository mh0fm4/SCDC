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


#ifndef __DATAPROV_CONFIG_HH__
#define __DATAPROV_CONFIG_HH__


#include <string>
#include <sstream>


class scdc_config_result: public std::string
{
  public:
    scdc_config_result():std::string() { }

    template<int SEP>
    void add(const char *s) { if (size() > 0 && at(size() - 1) != SEP) (*this) += static_cast<char>(SEP); (*this) += s; }
    template<int SEP>
    void add(const std::string &s) { add<SEP>(s.c_str()); }

    template<int SEP>
    void set(const char *s) { clear(); add<SEP>(s); }
    template<int SEP>
    void set(const std::string &s) { set<SEP>(s.c_str()); }

    std::string &get() { return *this; }

    scdc_config_result &operator= (const std::string &s) { std::string::operator = (s); return *this; }
};


class scdc_dataprov_config
{
  public:
    static bool info(scdc_config_result &result, const char* msg) { result.add<'|'>(msg); return true; }
    static bool info(scdc_config_result &result, const std::string &param, const char* msg) { result.add<'|'>(msg); return true; }
    static bool info(scdc_config_result &result, const std::string &param, const std::string &msg) { return info(result, param, msg.c_str()); }

    static bool ls(scdc_config_result &result, const char *param) { result.add<','>(param); return true; }
    static bool ls(scdc_config_result &result, const std::string &param, const char* msg) { result.add<','>(msg); return true; }
    static bool ls(scdc_config_result &result, const std::string &param, const std::string &msg) { return ls(result, param, msg.c_str()); }

    static bool put(scdc_config_result &result, const char *param) { return true; }
    template<typename T>
    static bool put(scdc_config_result &result, const char *param, std::string &val, T &t) { std::stringstream ss(val); ss >> t; std::getline(ss, val); return put(result, param); }
    template<typename T>
    static bool put(scdc_config_result &result, const std::string &param, std::string &val, T &t) { return put<T>(result, param.c_str(), val, t); }

    static bool get(scdc_config_result &result, const char *param) { return true; }
    template<typename T>
    static bool get(scdc_config_result &result, const char *param, const T &t) { std::stringstream ss; ss << t; result.add<','>(ss.str()); return get(result, param); }
    template<typename T>
    static bool get(scdc_config_result &result, const std::string &param, const T &t) { return get<T>(result, param.c_str(), t); }

    static bool rm(scdc_config_result &result, const char *param) { return true; }

    static bool fail(scdc_config_result &result, const char *param, const char *msg) { result += "config with parameter '"; result += param; result += "' failed: "; result += msg; return false; }
    static bool fail(scdc_config_result &result, const char *param, const std::string &msg) { return fail(result, param, msg.c_str()); }
};


#endif /* __DATAPROV_CONFIG_HH__ */
