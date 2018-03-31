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


#ifndef __SCDC_COMPRESSION_HH__
#define __SCDC_COMPRESSION_HH__


#if HAVE_ZLIB_H
# include <zlib.h>
#endif


class scdc_compression
{
  public:
    scdc_compression() { };
    virtual ~scdc_compression() { };

    virtual void init(bool compress) = 0;
    virtual void release() = 0;

    virtual void set_level(scdcint_t level) { };

    virtual bool perform(scdcint_t &in_size, const void *in_buf, scdcint_t &out_size, void *out_buf, bool finish = true) = 0;
};


#if HAVE_ZLIB_H

class scdc_compression_zlib: public scdc_compression
{
  public:
    scdc_compression_zlib()
      :scdc_compression(), level(Z_DEFAULT_COMPRESSION) { }

    void init(bool compress_)
    {
      compress = compress_;
      initialized = true;

      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;
      strm.avail_in = 0;
      strm.next_in = Z_NULL;

      if (compress) deflateInit(&strm, level);
      else inflateInit(&strm);
    }

    void release()
    {
      if (!initialized) return;

      if (compress) deflateEnd(&strm);
      else inflateEnd(&strm);

      initialized = false;
    }    

    void set_level(scdcint_t level_)
    {
      if (level_ == -1) level = Z_DEFAULT_COMPRESSION;
      else if (level_ == 0) level = Z_NO_COMPRESSION;
      else if (level_ == 1) level = Z_BEST_SPEED;
      else if (level_ == 2) level = Z_BEST_COMPRESSION;
    }

    bool perform(scdcint_t &in_size, const void *in_buf, scdcint_t &out_size, void *out_buf, bool finish)
    {
      strm.next_in = static_cast<Bytef *>(const_cast<void *>(in_buf));
      strm.avail_in = in_size;

      strm.next_out = static_cast<Bytef *>(out_buf);
      strm.avail_out = out_size;

/*      std::cout << "avail_in: " << strm.avail_in << ", avail_out: " << strm.avail_out << ", finish: " << finish << std::endl;*/

      int ret;

      if (_compress) ret = deflate(&strm, finish?Z_FINISH:Z_NO_FLUSH);
      else ret = inflate(&strm, Z_NO_FLUSH);

/*      std::cout << "avail_in: " << strm.avail_in << ", avail_out: " << strm.avail_out << ", ret: " << ret << std::endl;*/

      in_size -= strm.avail_in;

      out_size -= strm.avail_out;
      
      return (ret >= 0);
    }

  private:
    bool compress, initialized;
    int level;
    z_stream strm;
};

#endif /* HAVE_ZLIB_H */


#endif /* __SCDC_COMPRESSION_HH__ */
