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

#ifndef DTRANS_PNGIO_HPP
#define DTRANS_PNGIO_HPP

#include <png.h>
#include <stdexcept>


namespace dtrans {

  class DistanceTransform;
  

  /**
     Utility for reading and writing PNG files that encode distance
     transform information. Only 8-bit grayscale PNGs are currently
     supported.
  */
  class PNGIO
  {
  public:
    virtual ~PNGIO();
    
    /** Read a PNG file given as a path. If successful, this PNGIO
	instance can subsequently be used for createTransform() or
	mapSpeed(). An exception is thrown in case the file does not
	exist or is not an 8-bit grayscale PNG file.
    */
    void read(std::string const & filename) throw(std::runtime_error);
    
    /** Read a PNG file given as an open file pointer. If successful,
	this PNGIO instance can subsequently be used for
	createTransform() or mapSpeed(). An exception is thrown in
	case the file does not exist or is not an 8-bit grayscale PNG
	file.
    */
    void read(FILE * fp) throw(std::runtime_error);
    
    /** Write the data from DistanceTransform::getDist() as an 8-bit
	grayscale PNG file. Distances are scaled such that maxval gets
	encoded as 255. Throws an exception if something goes wrong,
	e.g. if the given filename could not be opened for writing. */
    static void write(DistanceTransform const & dt,
		      std::string const & filename, double maxval) throw(std::runtime_error);
    
    /** Write the data from DistanceTransform::getDist() as an 8-bit
	grayscale PNG file to an already open file pointer. Distances
	are scaled such that maxval gets encoded as 255. Throws an
	exception if something goes wrong. */
    static void write(DistanceTransform const & dt,
		      FILE * fp, double maxval) throw(std::runtime_error);
    
    /** \return The maximum value of the data, after a successful
	read().
     */
    png_byte maxVal() const;
    
    /** \return The minimum value of the data, after a successful
	read().
     */
    png_byte minVal() const;
    
    /** Create a DistanceTransform based on data previously
	read(). You can control how the input range (8-bit grayscale
	values, i.e. 0..255) gets translated to initial distances
	(passed to DistanceTransform::setDist()) by adjusting the
	thresh, scale, and invert parameters.
	
	\return A freshly allocated and initialized DistanceTransform
	object. Throws an exception if something goes wrong.
    */
    DistanceTransform *
    createTransform(/** The maximum grayscale value for which
			setDist() will be called. If invert=true, then
			thresh is the minimum such value. */
		    png_byte thresh,
		    /** The scale applied to the grayscale
			values. E.g. if scale=0.17 the a grayscale
			value of 1 will result in a call to
			setDist(..., 0.17). If invert=true, then the
			we use scale*(255-gray) instead. */
		    double scale,
		    /** Whether to invert the grayscale-to-distance
			scale. Use this if your input image uses white
			(gray=255) to denote the goal set. */
		    bool invert)
      const throw(std::runtime_error);
    
    /** Map previously read() data to
	DistanceTransform::setSpeed(). This allows you to read-in
	obstacle information from an 8-bit grayscale PNG file. The
	meaning of thresh, scale, and invert is the same as for
	createTransform(), but note that the result of the scaling
	gets clipped to the range 0..1 (where 0 means obstacle and 1
	means freespace, and intermediate values encode e.g. how risky
	it is to traverse a given cell).
	
	Throws an exception if something goes wrong, e.g. if the
	dimensions of the given DistanceTransform don't match the PNG
	file that was last read().
    */
    void mapSpeed(DistanceTransform & dt, png_byte thresh, double scale, bool invert)
      const throw(std::runtime_error);
    
  protected:
    png_structp read_;
    png_infop read_info_, read_info_end_;
    png_uint_32 width_, height_;
    png_bytepp row_p_;
    png_byte max_val_, min_val_;
    
    void init() throw(std::runtime_error);
    void fini();
  };
  
}

#endif // DTRANS_PNGIO_HPP
