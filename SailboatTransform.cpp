/*
 * Copyright (c) 2012 Roland Philippsen <roland DOT philippsen AT gmx DOT net>
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

#include "SailboatTransform.hpp"
#include <limits>
#include <math.h>

// cheap error messages, should use something else...
#include <iostream>


namespace dtrans {
  
  double const SailboatTransform::infinity(std::numeric_limits<double>::max());
  double const SailboatTransform::epsilon(1e-6);
  
  
  SailboatTransform::
  SailboatTransform(size_t dimx, size_t dimy, double scale)
    : m_dimx(dimx),
      m_dimy(dimy),
      m_ncells(dimx * dimy),
      m_toprow(m_ncells - dimx),
      m_rightcol(dimx - 1),
      m_scale(scale),
      m_value(m_ncells, infinity),
      m_key(m_ncells, -1.0),
      m_gx(m_ncells, 0.0),
      m_gy(m_ncells, 0.0),
      m_gn(m_ncells, -1)
  {
  }
  
  
  bool SailboatTransform::
  setTime(size_t ix, size_t iy, double time)
  {
    if (time < 0) {
      return false;
    }
    
    size_t const cell(index(ix, iy));
    if (cell >= m_ncells) {
      return false;
    }
    
    m_value[cell] = -time;	// <=0 means "fixed"
    requeue(cell);
    
    return true;
  }
  
  
  double SailboatTransform::
  getTime(size_t ix, size_t iy) const
  {
    size_t const cell(index(ix, iy));
    if (cell >= m_ncells) {
      return infinity;
    }
    return fabs(m_value[cell]);
  }
  
  
  void SailboatTransform::
  compute(double ceiling)
  {
    while ( ! m_queue.empty()) {
      if (m_queue.begin()->first > ceiling) {
	break;
      }
      propagate();
    }
  }
  
  
  void SailboatTransform::
  compute(double ceiling, FILE * dbg_fp, std::string const & dbg_prefix)
  {
    std::string prefix(dbg_prefix + "  ");
    for (size_t ii(0); ! m_queue.empty(); ++ii) {
      fprintf(stdout, "%siteration %zu\n", dbg_prefix.c_str(), ii);
      if (m_queue.begin()->first > ceiling) {
	fprintf(stdout, "%stop of queue %g is above ceiling %g\n",
		dbg_prefix.c_str(), m_queue.begin()->first, ceiling);
	break;
      }
      dump(stdout, prefix.c_str());
      propagate();
    }
    fprintf(stdout, "%sfinal\n", dbg_prefix.c_str());
    dump(stdout, prefix.c_str());
  }
  
  
  /** Don't call this unless you are (pretty) sure that the index is
      on the queue. */
  bool SailboatTransform::
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
  
  
  void SailboatTransform::
  requeue(size_t index)
  {
    if (m_key[index] >= 0) {
      if ( ! unqueue(index)) {
	std::cerr << "bug in requeue? could not unqueue\n";
      }
    }
    m_key[index] = fabs(m_value[index]);
    m_queue.insert(std::make_pair(m_key[index], index));
  }
  
  
  void SailboatTransform::
  update(size_t index)
  {
    if (m_value[index] <= 0) {	// fixed cell, skip it
      return;
    }
    
    double radius;
    if (m_model) {
      std::cerr << "\nMORE WORK TO DO!\nimplement sailboat transform update with model\nBYEBYE see you later!\n";
      abort();
      // will need to do the speed model within the search for a
      // secondary propagator, because the radius determines whether
      // we can use interpolation, and the upwind direction is
      // determined by the secondary propagator's value. bit of a
      // chicken and egg there, but should be solvable. can also
      // revert the gradient method back to its original form, or
      // better yet extract a common base class for DistanceTransform
      // and SailboatTransform.
    }
    else {
      radius = m_scale;	// it's h/F, or scale/speed ...
    }
    
    if (radius >= infinity) {
      // currently unreachable, but maybe later from a different
      // direction, so don't set it to -infinity, use +infinity
      m_value[index] = infinity;
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
		<< "  value: " << m_value[index] << "\n";
      m_value[index] = infinity;
      requeue(index);
      return;
    }
    
    queue_it ip(props.begin());
    queue_it endp(props.end());
    double const primary(ip->first);
    bool const northsouth(ix == (ip->second % m_dimx));
    
    // Try to find a valid secondary for the interpolation: it needs
    // to lie along a different axis than the primary, and it needs to
    // be closer than m_scale/speed to it.
    double const r2(pow(radius, 2.0));
    double const p2(pow(primary, 2));
    for (++ip; endp != ip; ++ip) {
      bool const valid(northsouth ^ (ix == (ip->second % m_dimx)));
      if (valid) {
	double const secondary(ip->first);
	if (radius > secondary - primary) {
	  // Found it!
	  double const bb(primary + secondary);
	  double const cc((p2 + pow(secondary, 2) - r2) / 2.0);
	  double const root(pow(bb, 2) - 4.0 * cc);
	  double const rhs((bb + sqrt(root)) / 2.0);
	  if (rhs < m_value[index]) {
	    m_value[index] = rhs;
	    requeue(index);
	    return;
	  }
	}
      }
    }
    
    double const rhs(primary + radius);
    if (rhs < m_value[index]) {
      m_value[index] = rhs;
      requeue(index);
    }
  }
  
  
  size_t SailboatTransform::
  pop()
  {
    queue_it iq(m_queue.begin());
    size_t const index(iq->second);
    m_queue.erase(iq);
    m_key[index] = -1;
    return index;
  }
  
  
  bool SailboatTransform::
  propagate()
  {
    if (m_queue.empty()) {
      return false;
    }
    
    size_t const index(pop());
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
    
    return true;
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
  
  
  void SailboatTransform::
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
  
  
  void SailboatTransform::
  dumpQueue(FILE * fp, std::string const & prefix) const
  {
    if (m_queue.empty()) {
      fprintf(fp, "%sempty queue\n", prefix.c_str());
      return;
    }
    
    fprintf(fp, "%squeue: [key index value]\n", prefix.c_str());
    for (queue_cit iq(m_queue.begin()); iq != m_queue.end(); ++iq) {
      fprintf(fp, "%s  ", prefix.c_str());
      pval(fp, m_key[iq->second]);
      fprintf(fp, "  (%zu, %zu)", iq->second % m_dimx, iq->second / m_dimx);
      fprintf(fp, "  ");
      pval(fp, m_value[iq->second]);
      if (fabs(m_key[iq->second]) != iq->first) {
	fprintf(fp, "  ERROR queue key (%g) mismatch", iq->first);
      }
      fprintf(fp, "\n");
    }
    
    fprintf(fp, "%swavefront:\n", prefix.c_str());
    double const front_key(m_queue.begin()->first);
    size_t iy(m_dimy);
    while (iy > 0) {
      --iy;
      fprintf(fp, "%s  ", prefix.c_str());
      for (size_t ix(0); ix < m_dimx; ++ix) {
	size_t const idx(index(ix, iy));
	char const * cc(0);
	bool const fixed(m_value[idx] <= 0);
	if (m_key[idx] < 0) {
	  if (fixed) {
	    cc = "x";
	  }
	  else {
	    cc = ".";
	  }
	}
	else if ((front_key >= 0) && (m_key[idx] == front_key)) {
	  if (fixed) {
	    cc = "#";
	  }
	  else {
	    cc = "*";
	  }
	}
	else {
	  if (fixed) {
	    cc = "+";
	  }
	  else {
	    cc = "o";
	  }
	}
	fprintf(fp, "%s", cc);
      }
      fprintf(fp, "\n");
    }
  }
  
  
  void SailboatTransform::
  stat(double & minval, double & maxval, double & minkey, double & maxkey) const
  {
    minval = infinity;
    maxval = - infinity;
    for (size_t ii(0); ii < m_ncells; ++ii) {
      double const val(fabs(m_value[ii]));
      if (val < infinity) {
	if (val > maxval) {
	  maxval = val;
	}
	if (val < minval) {
	  minval = val;
	}
      }
    }
    
    if (m_queue.empty()) {
      minkey = infinity;
      maxkey = - infinity;
    }
    else {
      minkey = m_queue.begin()->first;
      maxkey = m_queue.rbegin()->first;
    }
  }
  
  
  double SailboatTransform::
  getTopKey() const
  {
    if (m_queue.empty()) {
      return infinity;
    }
    return m_queue.begin()->first;
  }
  
  
  size_t SailboatTransform::
  computeGradient(size_t ix, size_t iy,
		  double & gx, double & gy,
		  bool use_cache) const
  {
    size_t const ixy(index(ix, iy));
    
    if (use_cache && m_gn[ixy] >= 0) {
      gx = m_gx[ixy];
      gy = m_gy[ixy];
      return m_gn[ixy];
    }
    
    double const height(fabs(m_value[ixy]));
    
    // Find all downwind neighbors.
    queue_t dwn;
    if (iy > 0) {		// try south
      size_t const nbor(ixy - m_dimx);
      double const nval(fabs(m_value[nbor]));
      if (nval < height) {
	dwn.insert(std::make_pair(nval, nbor));
      }
    }
    if (ixy < m_toprow) {	// try north
      size_t const nbor(ixy + m_dimx);
      double const nval(fabs(m_value[nbor]));
      if (nval < height) {
	dwn.insert(std::make_pair(nval, nbor));
      }
    }
    if (ix > 0) {		// try west
      size_t const nbor(ixy - 1);
      double const nval(fabs(m_value[nbor]));
      if (nval < height) {
	dwn.insert(std::make_pair(nval, nbor));
      }
    }
    if (ix < m_rightcol) {	// try east
      size_t const nbor(ixy + 1);
      double const nval(fabs(m_value[nbor]));
      if (nval < height) {
	dwn.insert(std::make_pair(nval, nbor));
      }
    }
    
    // Compute gradient based on 0, 1, or 2 downwind neighbors.
    
    if (dwn.empty()) {
      if (use_cache) {
	m_gx[ixy] = 0;
	m_gy[ixy] = 0;
	m_gn[ixy] = 0;
      }
      gx = 0;
      gy = 0;
      return 0;
    }
    
    // Beware of confusions between x and y as well as + and -
    // below. The idea is to use the height difference wrt the lowest
    // neighbor for either gx or gy, depending on whether it neighbors
    // the current cell along x or y. Then, if there is a second
    // lowest neighbor that lies along the other coordinate axis, use
    // that height difference to fill in the other gradient
    // coordinate. Height differences must be counted positive when we
    // go along positive x or y, and negative otherwise. By the checks
    // above we know that all candidate neighbors lie below the
    // current cell, so we can switch sign simply by choosing the
    // order of terms in the substractions.
    
    queue_it idwn(dwn.begin());
    queue_it endwn(dwn.end());
    double const nval0(idwn->first);
    size_t const nidx0(idwn->second);
    bool const lowest_nbor_along_y(ix == (nidx0 % m_dimx));
    
    // Fill in gx or gy based on the direction to the lowest neighbor.
    if (lowest_nbor_along_y) {
      if (ixy < nidx0) {	// lowest neighbor at (ix, iy+1) so gy < 0
	gy = nval0 - height;
      }
      else {			// else it's at (ix, iy-1) and thus gy > 0
	gy = height - nval0;
      }
    }
    else {
      if (ixy < nidx0) {	// lowest neighbor is at (ix+1, iy) so gx < 0
	gx = nval0 - height;
      }
      else {			// else it's at (ix-1, iy) and thus gx > 0
	gx = height - nval0;
      }
    }
    
    // Find a second lowest neighbor that lies along the other axis.
    for (++idwn; endwn != idwn; ++idwn) {
      size_t const nidx1(idwn->second);
      if (lowest_nbor_along_y ^ (ix == (nidx1 % m_dimx))) {
	double const nval1(idwn->first);
	// Fill in "the other" gx or gy based on the axis of the
	// second lowest neighbor.
	if (lowest_nbor_along_y) {
	  if (ixy < nidx1) { // SECOND lowest neighbor is at (ix+1, iy) so gx < 0
	    gx = nval1 - height;
	  }
	  else {	    // else it's at (ix-1, iy) and thus gx > 0
	    gx = height - nval1;
	  }
	}
	else {
	  if (ixy < nidx1) { // SECOND lowest neighbor is at (ix, iy+1) so gy < 0
	    gy = nval1 - height;
	  }
	  else {	    // else it's at (ix, iy-1) and thus gy > 0
	    gy = height - nval1;
	  }
	}
	if (use_cache) {
	  m_gx[ixy] = gx;
	  m_gy[ixy] = gy;
	  m_gn[ixy] = 2;
	}
	return 2;
      }
    }
    
    // We did not find a second_lowest neighbor that lies
    // perpendicular to the lowest, so we have to set that
    // contribution to zero.
    if (lowest_nbor_along_y) {
      gx = 0;
    }
    else {
      gy = 0;
    }
    if (use_cache) {
      m_gx[ixy] = gx;
      m_gy[ixy] = gy;
      m_gn[ixy] = 1;
    }
    return 1;
  }
  
  
  void SailboatTransform::
  resetTime()
  {
    m_value.assign(m_ncells, infinity);
    m_key.assign(m_ncells, -1.0);
    m_queue.clear();
    m_gx.assign(m_ncells, 0.0);
    m_gy.assign(m_ncells, 0.0);
    m_gn.assign(m_ncells, -1);
  }
  
}
