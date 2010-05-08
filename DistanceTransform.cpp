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
#include <math.h>

// cheap error messages, should use something else...
#include <iostream>


namespace dtrans {
  
  double const DistanceTransform::infinity(std::numeric_limits<double>::max());
  
  
  DistanceTransform::
  DistanceTransform(size_t dimx, size_t dimy, double scale)
    : m_dimx(dimx),
      m_dimy(dimy),
      m_ncells(dimx * dimy),
      m_toprow(m_ncells - dimx),
      m_rightcol(dimx - 1),
      m_scale(scale),
      m_scale2(pow(scale, 2)),
      m_value(m_ncells, infinity),
      m_rhs(m_ncells, infinity),
      m_key(m_ncells, -1)
  {
  }
  
  
  bool DistanceTransform::
  set(size_t ix, size_t iy, double dist)
  {
    if (dist < 0) {
      return false;
    }
    
    size_t const cell(index(ix, iy));
    if (cell >= m_ncells) {
      return false;
    }
    
    m_value[cell] = infinity;	// will be set in propagate()
    m_rhs[cell] = -dist;	// <=0 means "fixed"
    requeue(cell);
    
    return true;
  }
  
  
  double DistanceTransform::
  get(size_t ix, size_t iy) const
  {
    size_t const cell(index(ix, iy));
    if (cell >= m_ncells) {
      return infinity;
    }
    return m_value[cell];
  }
  
  
  void DistanceTransform::
  compute()
  {
    while ( ! m_queue.empty()) {
      propagate();
    }
  }
  
  
  void DistanceTransform::
  compute(FILE * dbg_fp, std::string const & dbg_prefix)
  {
    std::string prefix(dbg_prefix + "  ");
    for (size_t ii(0); ! m_queue.empty(); ++ii) {
      fprintf(stdout, "%siteration %zu\n", dbg_prefix.c_str(), ii);
      dump(stdout, prefix.c_str());
      propagate();
    }
    fprintf(stdout, "%sfinal\n", dbg_prefix.c_str());
    dump(stdout, prefix.c_str());
  }
  
  
  /** Don't call this unless you are (pretty) sure that the index is
      on the queue. */
  bool DistanceTransform::
  unqueue(size_t index)
  {
    queue_it iq(m_queue.lower_bound(m_key[index]));
    queue_it const endq(m_queue.end());
    for (/**/; iq != endq; ++iq) {
      if (index == iq->second) {
	m_queue.erase(iq);
	break;
      }
    }
    return endq != iq;
  }
  
  
  void DistanceTransform::
  requeue(size_t index)
  {
    double const rhs(fabs(m_rhs[index]));
    double const value(fabs(m_value[index]));
    
    // Special case of dtrans: we only propagate out once (unlike E*),
    // so we can just ignore when the rhs is bigger than the value. In
    // the full-blown implementation, the check below is for strict
    // equality instead. Beware that this is implicitly assumed in
    // other parts of this code.
    if (rhs >= value) {
      if (m_key[index] >= 0) {
	if ( ! unqueue(index)) {
	  std::cerr << "bug in requeue? could not unqueue consistent index\n";
	}
	m_key[index] = -1;
      }
      if (rhs != value) { // fix the data structures for the dtrans special case
	m_rhs[index] = m_value[index]; // re-read from m_value to get the same sign, in case of fixed cells
      }
      return;
    }
    
    // Otherwise, add or re-add the index, keeping track of its key
    // and handling the special negative distances. Avoid re-adding an
    // index under the same key.
    double const key(std::min(value, rhs));
    if (m_key[index] < 0) {
      m_key[index] = key;
      m_queue.insert(std::make_pair(key, index));
    }
    else if (key != m_key[index]) {
      if ( ! unqueue(index)) {
	std::cerr << "bug in requeue? could not unqueue inconsistent index\n";
      }
      m_key[index] = key;
      m_queue.insert(std::make_pair(key, index));
    }
  }
  
  
  void DistanceTransform::
  update(size_t index)
  {
    if (m_value[index] <= 0) {	// fixed cell, skip it
      return;
    }
    
    // Find all candidate propagators.
    queue_t props;
    if (index >= m_dimx) {	// try south
      size_t const nbor(index - m_dimx);
      double const nval(fabs(m_value[nbor]));
      if (nval < infinity) {
	props.insert(std::make_pair(nval, nbor));
      }
    }
    if (index < m_toprow) {	// try north
      size_t const nbor(index + m_dimx);
      double const nval(fabs(m_value[nbor]));
      if (nval < infinity) {
	props.insert(std::make_pair(nval, nbor));
      }
    }
    size_t const ix(index % m_dimx);
    if (ix > 0) {		// try west
      size_t const nbor(index - 1);
      double const nval(fabs(m_value[nbor]));
      if (nval < infinity) {
	props.insert(std::make_pair(nval, nbor));
      }
    }
    if (ix < m_rightcol) {	// try east
      size_t const nbor(index + 1);
      double const nval(fabs(m_value[nbor]));
      if (nval < infinity) {
	props.insert(std::make_pair(nval, nbor));
      }
    }
    
    // This probably never happens, at least in the dtrans special
    // case, because in order to arrive here we need to have expanded
    // one of our neighbors.
    if (props.empty()) {
      std::cerr << "bug in update? no valid propagators\n"
		<< "  index: " << index << " (" << (index % m_dimx) << ", " << (index / m_dimx) << ")\n"
		<< "  key:   " << m_key[index] << "\n"
		<< "  rhs:   " << m_rhs[index] << "\n"
		<< "  value: " << m_value[index] << "\n";
      m_rhs[index] = infinity;
      requeue(index);
      return;
    }
    
    queue_it ip(props.begin());
    queue_it endp(props.end());
    double const primary(ip->first);
    bool const northsouth(ix == (ip->second % m_dimx));
    
    // Try to find a valid secondary for the interpolation: it needs
    // to lie along a different axis than the primary, and it needs to
    // be closer than m_scale to it.
    for (++ip; endp != ip; ++ip) {
      if (northsouth && (ix != (ip->second % m_dimx))) {
	double const secondary(ip->first);
	if (m_scale > secondary - primary) {
	  // Found it!
	  double const bb(primary + secondary);
	  double const cc((pow(primary, 2)
			   + pow(secondary, 2)
			   - m_scale2) / 2.0);
	  double const root(pow(bb, 2) - 4.0 * cc);
	  m_rhs[index] = (bb + sqrt(root)) / 2.0;
	  requeue(index);
	  return;
	}
      }
    }
    
    m_rhs[index] = primary + m_scale;
    requeue(index);
  }
  
  
  size_t DistanceTransform::
  pop()
  {
    queue_it iq(m_queue.begin());
    size_t const index(iq->second);
    m_queue.erase(iq);
    m_key[index] = -1;
    return index;
  }
  
  
  void DistanceTransform::
  propagate()
  {
    size_t const index(pop());
    double const rhs(fabs(m_rhs[index]));
    
    if (m_value[index] > rhs) {
      m_value[index] = rhs;
      if (index >= m_dimx) {	// south
	update(index - m_dimx);
      }
      if (index < m_toprow) {	// north
	update(index + m_dimx);
      }
      size_t const ix(index % m_dimx);
      if (ix > 0) {		// west
	update(index - 1);
      }
      if (ix < m_rightcol) {	// east
	update(index + 1);
      }      
    }

    else {
      std::cerr << "bug in propagate? rhs >= value\n"
		<< "  index: " << index << " (" << (index % m_dimx) << ", " << (index / m_dimx) << ")\n"
		<< "  key:   " << m_key[index] << "\n"
		<< "  rhs:   " << rhs << "\n"
		<< "  value: " << m_value[index] << "\n";
      m_value[index] = infinity;
      // In this case, E-Star would propagate to all downwind
      // cells... but we do not track that information here in the
      // DistanceTransform. Propagate to all neighbors instead: waste
      // compute cycles that "never happens" though.
      if (index >= m_dimx) {	// south
	update(index - m_dimx);
      }
      if (index < m_toprow) {	// north
	update(index + m_dimx);
      }
      size_t const ix(index % m_dimx);
      if (ix > 0) {		// west
	update(index - 1);
      }
      if (ix < m_rightcol) {	// east
	update(index + 1);
      }      
    }
  }
  
  
  static void pval(FILE * fp, double val)
  {
    if (isinf(val)) {
      fprintf(fp, " inf    ");
    }
    else if (isnan(val)) {
      fprintf(fp, " nan    ");
    }
    else if (fabs(val) >= 1e4) {
      if (val > 0) {
	fprintf(fp, " huge   ");
      }
      else {
	fprintf(fp, " -huge  ");
      }
    }
    else if (fabs(fmod(val, 1)) < 1e-6) {
      fprintf(fp, "% 6d  ", static_cast<int>(rint(val)));
    }
    else {
      fprintf(fp, "% 6.4f ", val);
    }
  }
  
  
  void DistanceTransform::
  dump(FILE * fp, std::string const & prefix) const
  {
    fprintf(fp, "%skey\n", prefix.c_str());
    size_t iy(m_dimy);
    while (iy > 0) {
      --iy;
      fprintf(fp, "%s  ", prefix.c_str());
      for (size_t ix(0); ix < m_dimx; ++ix) {
	pval(fp, m_key[index(ix, iy)]);
      }
      fprintf(fp, "\n");
    }



    fprintf(fp, "%srhs\n", prefix.c_str());
    iy = m_dimy;
    while (iy > 0) {
      --iy;
      fprintf(fp, "%s  ", prefix.c_str());
      for (size_t ix(0); ix < m_dimx; ++ix) {
	pval(fp, m_rhs[index(ix, iy)]);
      }
      fprintf(fp, "\n");
    }



    fprintf(fp, "%svalue\n", prefix.c_str());
    iy = m_dimy;
    while (iy > 0) {
      --iy;
      fprintf(fp, "%s  ", prefix.c_str());
      for (size_t ix(0); ix < m_dimx; ++ix) {
	pval(fp, m_value[index(ix, iy)]);
      }
      fprintf(fp, "\n");
    }
  }
  
}
