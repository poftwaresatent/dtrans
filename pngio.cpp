/*
 * Copyright (c) 2010 Roland Philippsen <roland DOT philippsen AT gmx DOT net>
 *
 * BSD license:
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of
 *    contributors to this software may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR THE CONTRIBUTORS TO THIS SOFTWARE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "pngio.hpp"
#include "DistanceTransform.hpp"
#include <limits>
#include <errno.h>
#include <string.h>
#include <math.h>

using namespace std;


namespace dtrans {

  PNGIO::
  ~PNGIO()
  {
    fini();
  }
  
  
  void PNGIO::
  read(std::string const & filename) throw(std::runtime_error)
  {
    FILE * fp(fopen(filename.c_str(), "rb"));
    if (0 == fp) {
      throw runtime_error("dtrans::PNGIO::read(" + filename + "): " + strerror(errno));
    }
    read(fp);
    fclose(fp);
  }
  
  
  void PNGIO::
  read(FILE * fp) throw(std::runtime_error)
  {
    init();
    png_init_io(read_, fp);
    png_read_png(read_, read_info_, PNG_TRANSFORM_IDENTITY, 0);
    int bit_depth, color_type, interlace_type, compression_type, filter_type;
    png_get_IHDR(read_, read_info_, &width_, &height_, &bit_depth, &color_type,
		 &interlace_type, &compression_type, &filter_type);
    if (PNG_COLOR_TYPE_GRAY != color_type) {
      throw runtime_error("dtrans::PNGIO::read(): input is not grayscale");
    }
    if (8 != bit_depth) {
      throw runtime_error("dtrans::PNGIO::read(): input is not 8-bit");
    }
    
    row_p_ = png_get_rows(read_, read_info_);
    if ( ! row_p_) {
      throw runtime_error("dtrans::PNGIO::read(): png_get_rows() failed");
    }
    
    max_val_ = std::numeric_limits<png_byte>::min();
    min_val_ = std::numeric_limits<png_byte>::max();
    for (png_uint_32 irow(0); irow < height_; ++irow) {
      png_bytep row(row_p_[irow]);
      for (png_uint_32 icol(0); icol < width_; ++icol) {
	if (row[icol] > max_val_) {
	  max_val_ = row[icol];
	}
	if (row[icol] < min_val_) {
	  min_val_ = row[icol];
	}
      }
    }
  }
  
  
  png_byte PNGIO::
  maxVal() const
  {
    return max_val_;
  }
  
  
  png_byte PNGIO::
  minVal() const
  {
    return min_val_;
  }
  
  
  DistanceTransform * PNGIO::
  createTransform(png_byte thresh, double scale, bool invert) const throw(std::runtime_error)
  {
    if ((0 == width_) || (0 == height_)) {
      throw runtime_error("dtrans::PNGIO::createTransform(): no data");
    }
    
    DistanceTransform * dt(new DistanceTransform(width_, height_, 1));
    
    for (png_uint_32 irow(0); irow < height_; ++irow) {
      png_bytep row(row_p_[irow]);
      for (png_uint_32 icol(0); icol < width_; ++icol) {
	if (invert) {
	  if (row[icol] >= thresh) {
	    dt->set(icol, height_ - irow - 1, (255 - row[icol]) * scale);
	  }
	}
	else {
	  if (row[icol] <= thresh) {
	    dt->set(icol, height_ - irow - 1, row[icol] * scale);
	  }
	}
      }
    }
    
    return dt;
  }
  
  
  void PNGIO::
  init() throw(std::runtime_error)
  {
    fini();
    
    width_ = 0;
    height_ = 0;
    row_p_ = 0;
    max_val_ = 0;
    min_val_ = 255;
    
    read_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if ( ! read_) {
      throw runtime_error("dtrans::PNGIO::init(): png_create_read_struct() failed");
    }
    read_info_ = png_create_info_struct(read_);
    if ( ! read_info_) {
      throw runtime_error("dtrans::PNGIO::init(): png_create_info_struct() failed");
    }
    read_info_end_ = png_create_info_struct(read_);
    if ( ! read_info_end_) {
      throw runtime_error("dtrans::PNGIO::init(): png_create_info_struct() failed");
    }
  }
  
  
  void PNGIO::
  fini()
  {
    if (read_) {
      png_destroy_read_struct(&read_, &read_info_, &read_info_end_);
      read_ = 0;
    }
    if (read_info_) {
      png_destroy_info_struct(read_, &read_info_);
      read_info_ = 0;
    }
    if (read_info_end_) {
      png_destroy_info_struct(read_, &read_info_end_);
      read_info_end_ = 0;
    }
  }
  
  
  void PNGIO::
  write(DistanceTransform const & dt,
	std::string const & filename, double maxval) throw(std::runtime_error)
  {
    FILE * fp(fopen(filename.c_str(), "wb"));
    if (0 == fp) {
      throw runtime_error("dtrans::PNGIO(" + filename + "): " + strerror(errno));
    }
    write(dt, fp, maxval);
    fclose(fp);
  }
  
  
  void PNGIO::
  write(DistanceTransform const & dt,
	FILE * fp, double maxval) throw(std::runtime_error)
  {
    png_structp write_ptr(0);
    png_infop write_info_ptr(0);
    std::vector<png_byte> data;
    std::vector<png_bytep> row_p;
    
    try {
      
      write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
      if ( ! write_ptr) {
	throw runtime_error("dtrans::PNGIO::write(): png_create_write_struct() failed");
      }
      write_info_ptr = png_create_info_struct(write_ptr);
      if ( ! write_info_ptr) {
	throw runtime_error("dtrans::PNGIO::write(): png_create_info_struct() failed");
      }
      png_init_io(write_ptr, fp);
      
      png_uint_32 width(dt.dimX());
      png_uint_32 height(dt.dimY());
      int const bit_depth(8);
      int const color_type(PNG_COLOR_TYPE_GRAY);
      int const interlace_type(PNG_INTERLACE_NONE);
      int const compression_type(PNG_COMPRESSION_TYPE_BASE);
      int const filter_type(PNG_FILTER_TYPE_BASE);
      png_set_IHDR(write_ptr, write_info_ptr, width, height,
		   bit_depth, color_type, interlace_type,
		   compression_type, filter_type);
      
      data.resize(width * height);
      row_p.resize(height);
      for (png_uint_32 irow(0); irow < height; ++irow) {
	png_bytep row(&data[irow * width]);
	row_p[irow] = row;
	for (png_uint_32 icol(0); icol < width; ++icol) {
	  double val_in(dt.get(icol, height - irow - 1));
	  png_byte val_out;
	  if (val_in >= maxval) {
	    val_out = 255;
	  }
	  else if (val_in <= 0) {
	    val_out = 0;
	  }
	  else {
	    val_out = static_cast<png_byte>(rint(255 * val_in / maxval));
	  }
	  row[icol] = val_out;
	}
      }
      
      png_set_rows(write_ptr, write_info_ptr, &row_p[0]);
      png_write_png(write_ptr, write_info_ptr, PNG_TRANSFORM_IDENTITY, 0);
      
      png_destroy_write_struct(&write_ptr, &write_info_ptr);
      png_destroy_info_struct(write_ptr, &write_info_ptr);
    }
    
    catch (runtime_error const & ee) {
      if (write_ptr) {
	png_destroy_write_struct(&write_ptr, &write_info_ptr);
      }
      if (write_info_ptr) {
	png_destroy_info_struct(write_ptr, &write_info_ptr);
      }
      throw ee;
    }
  }

}
