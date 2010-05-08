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

#include "DistanceTransform.hpp"
#include <limits>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <math.h>

using namespace dtrans;


static png_structp png_ptr;
static png_infop info_ptr;
static png_infop end_info;


static void cleanup()
{
  if (end_info) {
    png_destroy_info_struct(png_ptr, &end_info);
  }
  if (info_ptr) {
    png_destroy_info_struct(png_ptr, &info_ptr);
  }
  if (png_ptr) {
    png_destroy_read_struct(&png_ptr, 0, 0);
  }
}


int main(int argc, char ** argv)
{
  if (0 != atexit(cleanup)) {
    errx(EXIT_FAILURE, "atexit() failed");
  }
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if ( ! png_ptr) {
    errx(EXIT_FAILURE, "png_create_read_struct() failed");
  }
  info_ptr = png_create_info_struct(png_ptr);
  if ( ! info_ptr) {
    errx(EXIT_FAILURE, "png_create_info_struct() failed");
  }
  end_info = png_create_info_struct(png_ptr);
  if ( ! end_info) {
    errx(EXIT_FAILURE, "png_create_info_struct() failed");
  }
  
  fprintf(stderr, "reading PNG from stdin\n");
  png_init_io(png_ptr, stdin);
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);
  
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type, compression_type, filter_type;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
	       &interlace_type, &compression_type, &filter_type);
  char const * color_type_str("unknown color type");
  switch (color_type) {
  case PNG_COLOR_TYPE_GRAY: color_type_str = "grayscale"; break;
  case PNG_COLOR_TYPE_GRAY_ALPHA: color_type_str = "grayscale with alpha"; break;
  case PNG_COLOR_TYPE_PALETTE: color_type_str = "palette"; break;
  case PNG_COLOR_TYPE_RGB: color_type_str = "rgb"; break;
  case PNG_COLOR_TYPE_RGB_ALPHA: color_type_str = "rgb with alpha"; break;
  }
  fprintf(stderr, "  %u by %u pixels with %d bits, %s\n",
	  (unsigned int) width, (unsigned int) height, bit_depth, color_type_str);
  
  if (PNG_COLOR_TYPE_GRAY != color_type) {
    errx(EXIT_FAILURE, "input is not grayscale");
  }
  
  png_bytepp row_pointers(png_get_rows(png_ptr, info_ptr));
  if ( ! row_pointers) {
    errx(EXIT_FAILURE, "png_get_rows() failed");
  }
  png_byte in_max(std::numeric_limits<png_byte>::min());
  png_byte in_min(std::numeric_limits<png_byte>::max());
  for (png_uint_32 irow(0); irow < height; ++irow) {
    png_bytep row(row_pointers[irow]);
    fprintf(stderr, "    ");
    for (png_uint_32 icol(0); icol < width; ++icol) {
      if (row[icol] > in_max) {
	in_max = row[icol];
      }
      if (row[icol] < in_min) {
	in_min = row[icol];
      }
      switch (row[icol] >> 6) {
      case 0: fprintf(stderr, "#"); break;
      case 1: fprintf(stderr, "*"); break;
      case 2: fprintf(stderr, "o"); break;
      case 3: fprintf(stderr, "."); break;
      }
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "  input range %u to %u\n", (unsigned int) in_min, (unsigned int) in_max);
  
  DistanceTransform dt(width, height, 1);
  for (png_uint_32 irow(0); irow < height; ++irow) {
    png_bytep row(row_pointers[irow]);
    for (png_uint_32 icol(0); icol < width; ++icol) {
      if (row[icol] == in_min) {
	dt.set(icol, height - irow - 1, 0);
      }
    }
  }
  fprintf(stderr, "  distance transform input\n");
  dt.dump(stderr, "    ");
  
  dt.compute();
  fprintf(stderr, "  distance transform output\n");
  dt.dump(stderr, "    ");
  
  double out_max(0);
  for (png_uint_32 irow(0); irow < height; ++irow) {
    png_bytep row(row_pointers[irow]);
    for (png_uint_32 icol(0); icol < width; ++icol) {
      double const val(dt.get(icol, irow));
      if (val > out_max) {
	out_max = val;
      }
    }
  }
  fprintf(stderr, "  output range 0 to %f\n", out_max);
  
  for (png_uint_32 irow(0); irow < height; ++irow) {
    png_bytep row(row_pointers[irow]);
    fprintf(stderr, "    ");
    for (png_uint_32 icol(0); icol < width; ++icol) {
      row[icol] = (png_uint_32) rint(255.0 * dt.get(icol, height - irow - 1) / out_max);
      switch (row[icol] >> 6) {
      case 0: fprintf(stderr, "#"); break;
      case 1: fprintf(stderr, "*"); break;
      case 2: fprintf(stderr, "o"); break;
      case 3: fprintf(stderr, "."); break;
      }
    }
    fprintf(stderr, "\n");
  }
  
  
  errx(EXIT_SUCCESS, "byebye");
}
