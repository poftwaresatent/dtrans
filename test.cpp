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
#include <iostream>
#include <stdio.h>

using namespace dtrans;
using namespace std;

int main(int argc, char ** argv)
{
  DistanceTransform dt(4, 3, 0.1);
  bool ok(true);
  dt.dump(stdout, "init  ");
  
  if ( ! dt.set(0, 0, 1.0)) {
    ok = false;
    cout << "dt.set(0, 0, 1.0) failed\n";
  }
  dt.dump(stdout, "test1  ");
  
  if (dt.set(20, 10, 1.0)) {
    ok = false;
    cout << "dt.set(20, 10, 1.0) should have failed\n";
  }
  dt.dump(stdout, "test2  ");
  
  int step(0);
  while (dt.propagate()) {
    printf("step %d\n", step);
    dt.dumpQueue(stdout, "  ");
    ++step;
  }
  
  if (1.0 != dt.get(0, 0)) {
    ok = false;
    cout << "dt.get(0, 0) should have returned 1.0 instead of " << dt.get(0, 0) << "\n";
  }
  dt.dump(stdout, "test4  ");
  
  if (ok) {
    cout << "SUCCESS\n";
    return 0;
  }
  cout << "FAILURE\n";
  return 1;
}
