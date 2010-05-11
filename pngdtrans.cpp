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
#include "pngio.hpp"
#include <limits>
#include <err.h>
#include <stdlib.h>

using namespace dtrans;
using namespace std;

static PNGIO pngio;
static DistanceTransform * dt(0);


static void cleanup()
{
  delete dt;
}


int main(int argc, char ** argv)
{
  if (0 != atexit(cleanup)) {
    errx(EXIT_FAILURE, "atexit() failed");
  }
  
  // parse options
  string infname("-");
  string outfname("-");
  int verbosity(0);
  int inthresh(0);
  float inscale(1.0/255);
  for (int iopt(1); iopt < argc; ++iopt) {
    string const opt(argv[iopt]);
    if ("-i" == opt) {
      ++iopt;
      if (iopt >= argc) {
	errx(EXIT_FAILURE, "-i requires an argument (use -h for some help)");
      }
      infname = argv[iopt];
    }
    else if ("-o" == opt) {
      ++iopt;
      if (iopt >= argc) {
	errx(EXIT_FAILURE, "-o requires an argument (use -h for some help)");
      }
      outfname = argv[iopt];
    }
    else if ("-t" == opt) {
      ++iopt;
      if (iopt >= argc) {
	errx(EXIT_FAILURE, "-t requires an argument (use -h for some help)");
      }
      if ((1 != sscanf(argv[iopt], "%d", &inthresh))
	  || (inthresh < 0) || (inthresh > 255)) {
	errx(EXIT_FAILURE, "error reading inthresh \"%s\"", argv[iopt]);
      }
    }
    else if ("-s" == opt) {
      ++iopt;
      if (iopt >= argc) {
	errx(EXIT_FAILURE, "-s requires an argument (use -h for some help)");
      }
      if ((1 != sscanf(argv[iopt], "%f", &inscale))
	  || (inthresh < 0) || (inthresh > 255)) {
	errx(EXIT_FAILURE, "error reading inthresh \"%s\"", argv[iopt]);
      }
    }
    else if ("-v" == opt) {
      ++verbosity;
    }
    else if ("-vv" == opt) {
      verbosity += 2;
    }
    else if ("-vvv" == opt) {
      verbosity += 3;
    }
    else if ("-h" == opt) {
      printf("usage [-i infile] [-o outfile] [-vh]\n"
	     "  -i  input file name   (\"-\" for stdin, which is the default)\n"
	     "  -o  output file name  (\"-\" for stdout, which is the default)\n"
	     "  -t  inthresh          threshold for distance initialization\n"
	     "  -s  inscale           scale for distance initialization (default %f which is 1/255)\n"
	     "  -v                    verbose mode (multiple times makes it more verbose)\n"
	     "  -h                    this message\n",
	     1.0/255);
      exit(EXIT_SUCCESS);
    }
    else {
      errx(EXIT_FAILURE, "invalid option \"%s\" (use -h for some help)", argv[iopt]);
    }
  }
  if ((verbosity > 0) && ("-" == outfname)) {
    errx(EXIT_FAILURE, "cannot use stdout in verbose mode, specify an output file using -o");
  }
  
  try {

    if (verbosity > 0) {
      printf("reading from file %s\n", infname.c_str());
    }
    if ("-" == infname) {
      pngio.read(stdin);
    }
    else {
      pngio.read(infname);
    }
    
    if (verbosity > 0) {
      printf("creating DistanceTransform\n");
    }
    DistanceTransform * dt(pngio.createTransform(inthresh, inscale, false));
    if (verbosity > 1) {
      printf("  distance transform input\n");
      dt->dump(stdout, "    ");
    }
    double minval, maxval, minkey, maxkey;
    dt->stat(minval, maxval, minkey, maxkey);
    if (minval > maxval) {
      errx(EXIT_FAILURE, "invalid input range, try adjusting the threshold with -t");
    }
    if (verbosity > 0) {
      printf("  input range %f to %f\n", minval, maxval);
    }
    
    if (verbosity > 0) {
      printf("propagating distance transform\n");
    }
    if (verbosity <= 2) {
      dt->compute();
    }
    else {
      int step(0);
      printf("step %d\n", step);
      dt->dumpQueue(stdout, "  ");
      while (dt->propagate()) {
	++step;
	printf("step %d\n", step);
	dt->dumpQueue(stdout, "  ");
      }
    }
    if (verbosity > 1) {
      printf("  distance transform output\n");
      dt->dump(stdout, "    ");
    }
    
    dt->stat(minval, maxval, minkey, maxkey);
    if (verbosity > 0) {
      printf("  output range %f to %f\n", minval, maxval);
      printf("writing result to %s\n", outfname.c_str());
    }
    
    if ("-" == outfname) {
      PNGIO::write(*dt, stdout, maxval);
    }
    else {
      PNGIO::write(*dt, outfname, maxval);
    }
    
  }
  catch (runtime_error const & ee) {
    errx(EXIT_FAILURE, "exception: %s", ee.what());
  }
}
