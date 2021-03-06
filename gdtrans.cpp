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
#include <boost/shared_ptr.hpp>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <limits>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

using namespace dtrans;
using namespace boost;
using namespace std;

static size_t dimx(300);
static size_t dimy(300);
static double scale(1);
static shared_ptr<DistanceTransform> dt;


static int gui();
static void cleanup();


int main(int argc, char ** argv)
{
  if (0 != atexit(cleanup)) {
    errx(EXIT_FAILURE, "atexit() failed");
  }
  
  // parse options
  int verbosity(0);
  for (int iopt(1); iopt < argc; ++iopt) {
    string const opt(argv[iopt]);if ("-v" == opt) {
      ++verbosity;
    }
    else if ("-vv" == opt) {
      verbosity += 2;
    }
    else if ("-vvv" == opt) {
      verbosity += 3;
    }
    else if ("-h" == opt) {
      printf("Distance transform from estar.sf.net -- Copyright (c) 2010 Roland Philippsen.\n"
	     "Redistribution, use, and modification permitted under the new BSD license.\n"
	     "\n"
	     "usage [-vh]\n"
	     "\n"
	     "  -v                    verbose mode (multiple times makes it more verbose)\n"
	     "  -h                    this message\n");
      exit(EXIT_SUCCESS);
    }
    else {
      errx(EXIT_FAILURE, "invalid option \"%s\" (use -h for some help)", argv[iopt]);
    }
  }
  
  if (verbosity > 0) {
    printf("Distance transform from estar.sf.net -- Copyright (c) 2010 Roland Philippsen.\n"
	   "Redistribution, use, and modification permitted under the new BSD license.\n");
  }
  
  if (verbosity > 0) {
    printf("creating DistanceTransform of size %zux%zu (scale %g)\n", dimx, dimy, scale);
  }
  dt.reset(new DistanceTransform(dimx, dimy, scale));
  
  dt->setDist(dimx/10, dimy/10, 0);
  dt->compute(DistanceTransform::infinity);
  
  if (verbosity > 1) {
    printf("  distance transform output\n");
    dt->dump(stdout, "    ");
  }
  
  return gui();
}


void cleanup()
{
  dt.reset();
}


class ValueImage : public Fl_Widget {
public:
  ValueImage(int xx, int yy, int width, int height, const char * label = 0)
    : Fl_Widget(xx, yy, width, height, label),
      value(0)
  {
  }
  
protected:
  virtual void draw()
  {
    // lazy init
    if (value.empty()) {
      dimx = dt->dimX();
      dimy = dt->dimY();
      value.resize(dt->nCells());
      double minval, maxval, minkey, maxkey;
      dt->stat(minval, maxval, minkey, maxkey);
      // warnx("ValueImage::draw():  minval %g  maxval %g  minkey %g  maxkey %g",
      // 	    minval, maxval, minkey, maxkey);
      if (maxval <= minval) {
	memset(&value[0], 0, value.size());
      }
      else {
	vector<double>::const_iterator in(dt->valueArray().begin());
	vector<unsigned char>::iterator out(value.begin());
	vector<unsigned char>::iterator end(value.end());
	for (/**/; out != end; ++in, ++out) {
	  if (*in >= maxval) {
	    *out = 255;
	  }
	  else if (*in <= 0) {
	    *out = 0;
	  }
	  else {
	    *out = static_cast<unsigned char>(rint(255 * (*in) / maxval));
	  }
	}
      }
    }
    
    // actually draw it
    fl_draw_image_mono(&value[0], x(), y(), dimx, dimy);
  }

private:
  size_t dimx, dimy;
  vector<unsigned char> value;
};


class GradientImage : public Fl_Widget {
public:
  GradientImage(int xx, int yy, int width, int height, const char * label = 0)
    : Fl_Widget(xx, yy, width, height, label),
      gx(0),
      gy(0),
      count(0)
  {
  }
  
protected:
  virtual void draw()
  {
    static int const skip(20);
    
    // lazy init
    if (gx.empty()) {
      dimx = dt->dimX();
      dimy = dt->dimY();
      gx.resize(dt->nCells());
      gy.resize(dt->nCells());
      count.resize(dt->nCells());
      for (size_t ix(0); ix < dimx; ++ix) {
	for (size_t iy(0); iy < dimy; ++iy) {
	  size_t ixy(dt->index(ix, iy));
	  double dgx, dgy;
	  count[ixy] = dt->computeGradient(ix, iy, dgx, dgy);
	  if (count[ixy] > 0) {
	    double const len(sqrt(pow(dgx, 2) + pow(dgy, 2)));
	    gx[ixy] = static_cast<int>(rint(dgx * skip / len));
	    gy[ixy] = static_cast<int>(rint(dgy * skip / len));
	  }
	  else {
	    gx[ixy] = 0;
	    gy[ixy] = 0;
	  }
	}
      }
    }
    
    // actually draw it
    fl_color(FL_BLACK);
    fl_rectf(x(), y(), dimx-1, dimy-1);
    for (int ix(0); ix < dimx; ix += skip) {
      for (int iy(0); iy < dimy; iy += skip) {
	size_t ixy(dt->index(ix, iy)); // beware of mixing cached sizes with possible new dt
	if (count[ixy] > 0) {
	  if (count[ixy] > 1) {
	    fl_color(FL_WHITE);
	  }
	  else {
	    fl_color(FL_YELLOW);
	  }
	  fl_line(x() + ix, y() + iy, x() + ix + gx[ixy], y() + iy + gy[ixy]);
	}
	else {
	  fl_color(FL_RED);
	  fl_point(x() + ix, y() + iy);
	}
      }
    }
  }

private:
  size_t dimx, dimy;
  vector<int> gx, gy;
  vector<size_t> count;
};


class Window : public Fl_Window {
public:
  Window(int width, int height, const char * title)
    : Fl_Window(width, height, title)
  {
    begin();
    value_image = new ValueImage(0, 0, dimx, dimy);
    gradient_image = new GradientImage(dimx + 50, 0, dimx, dimy);
    quit = new Fl_Button(width / 2 - 50, dimy + 50, 100, 30, "&Quit");
    quit->callback(cb_quit, this);
    end();
    resizable(this);
    show();
  }
  
  ValueImage * value_image;
  GradientImage * gradient_image;
  Fl_Button * quit;
  
private:
  static void cb_quit(Fl_Widget * widget, void * param)
  {
    reinterpret_cast<Window*>(param)->hide();
  }
};


int gui()
{
  Window win(2 * dimx + 100, dimy + 100, "toto");
  return Fl::run();
}
