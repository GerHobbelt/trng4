// Copyright (c) 2000-2019, Heiko Bauke
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//
//   * Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#if !(defined TRNG_BETA_DIST_HPP)

#define TRNG_BETA_DIST_HPP

#include <trng/cuda.hpp>
#include <trng/constants.hpp>
#include <trng/limits.hpp>
#include <trng/utility.hpp>
#include <trng/math.hpp>
#include <trng/special_functions.hpp>
#include <ostream>
#include <istream>
#include <iomanip>
#include <limits>

namespace trng {

  // uniform random number generator class
  template<typename float_t=double>
  class beta_dist {
  public:
    typedef float_t result_type;
    class param_type;
    
    class param_type {
    private:
      result_type alpha_, beta_, norm_;
    public:
      TRNG_CUDA_ENABLE
      result_type alpha() const { return alpha_; }
      TRNG_CUDA_ENABLE
      result_type beta() const { return beta_; }
    private:
      TRNG_CUDA_ENABLE
      result_type norm() const { return norm_; }
    public:
      TRNG_CUDA_ENABLE
      void alpha(result_type alpha_new) { alpha_=alpha_new; norm_=math::Beta(alpha_, beta_); }
      TRNG_CUDA_ENABLE
      void beta(result_type beta_new) { beta_=beta_new; norm_=math::Beta(alpha_, beta_); }
      param_type() : 
	alpha_(1), beta_(1), norm_(math::Beta(alpha_, beta_)) {
      }
      TRNG_CUDA_ENABLE
      param_type(result_type alpha, result_type beta) : 
	alpha_(alpha), beta_(beta), norm_(math::Beta(alpha, beta)) {
      }
      
      friend class beta_dist;
      
      // Streamable concept
      template<typename char_t, typename traits_t>
      friend std::basic_ostream<char_t, traits_t> &
      operator<<(std::basic_ostream<char_t, traits_t> &out,
		 const param_type &p) {
	std::ios_base::fmtflags flags(out.flags());
	out.flags(std::ios_base::dec | std::ios_base::fixed |
		  std::ios_base::left);
	out << '('
            << std::setprecision(math::numeric_limits<float_t>::digits10+1) 
	    << p.alpha() << ' ' << p.beta()
	    << ')';
	out.flags(flags);
	return out;
      }
  
      template<typename char_t, typename traits_t>
      friend std::basic_istream<char_t, traits_t> &
      operator>>(std::basic_istream<char_t, traits_t> &in,
		 param_type &p) {
	float_t alpha, beta;
	std::ios_base::fmtflags flags(in.flags());
	in.flags(std::ios_base::dec | std::ios_base::fixed |
		 std::ios_base::left);
	in >> utility::delim('(')
	   >> alpha >> utility::delim(' ')
	   >> beta >> utility::delim(')');
	if (in)
	  p=param_type(alpha, beta);
	in.flags(flags);
	return in;
      }

    };
    
  private:
    param_type p;
   
  public:
    // constructor
    TRNG_CUDA_ENABLE
    beta_dist(result_type alpha, result_type beta) : p(alpha, beta) {
    }
    TRNG_CUDA_ENABLE
    explicit beta_dist(const param_type &p) : p(p) {
    }
    // reset internal state
    TRNG_CUDA_ENABLE
    void reset() { }
    // random numbers
    template<typename R>
    TRNG_CUDA_ENABLE
    result_type operator()(R &r) {
      return math::inv_Beta_I(utility::uniformoo<result_type>(r), p.alpha(), p.beta(), p.norm());
    }
    template<typename R>
    TRNG_CUDA_ENABLE
    result_type operator()(R &r, const param_type &p) {
      beta_dist g(p);
      return g(r);
    }
    // property methods
    TRNG_CUDA_ENABLE
    result_type min() const { return result_type(0); }
    TRNG_CUDA_ENABLE
    result_type max() const { return result_type(1); }
    TRNG_CUDA_ENABLE
    param_type param() const { return p; }
    TRNG_CUDA_ENABLE
    void param(const param_type &p_new) { p=p_new; }
    TRNG_CUDA_ENABLE
    result_type alpha() const { return p.thata(); }
    // probability density function  
    TRNG_CUDA_ENABLE
    result_type pdf(result_type x) const {
      if (x<0 or x>1)
	return 0;
      if ((x==0 and p.alpha()-1<0) or (x==1 and p.beta()-1<0) ) {
#if !(defined __CUDA_ARCH__)
	errno=EDOM;
#endif
	return math::numeric_limits<result_type>::quiet_NaN();
      }
      return 1/p.norm()*math::pow(x, p.alpha()-1)*math::pow(1-x, p.beta()-1);
    }
    // cumulative density function 
    TRNG_CUDA_ENABLE
    result_type cdf(result_type x) const {
      if (x<=0)
	return 0;
      if (x>=1)
	return 1;
      return math::Beta_I(x, p.alpha(), p.beta(), p.norm());
    }
    // inverse cumulative density function 
    TRNG_CUDA_ENABLE
    result_type icdf(result_type x) const {
      if (x<0 or x>1) {
#if !(defined __CUDA_ARCH__)
	errno=EDOM;
#endif
	return math::numeric_limits<result_type>::quiet_NaN();
      }
      if (x==0)
        return 0;
      if (x==1)
        return 1;
      return math::inv_Beta_I(x, p.alpha(), p.beta(), p.norm());
    }
  };
    
  // -------------------------------------------------------------------

  // EqualityComparable concept
  template<typename float_t>
  TRNG_CUDA_ENABLE
  inline bool operator==(const typename beta_dist<float_t>::param_type &p1, 
			 const typename beta_dist<float_t>::param_type &p2) {
    return p1.alpha()==p2.alpha();
  }

  template<typename float_t>
  TRNG_CUDA_ENABLE
  inline bool operator!=(const typename beta_dist<float_t>::param_type &p1, 
			 const typename beta_dist<float_t>::param_type &p2) {
    return not (p1==p2);
  }  
  
  // -------------------------------------------------------------------

  // EqualityComparable concept
  template<typename float_t>
  TRNG_CUDA_ENABLE
  inline bool operator==(const beta_dist<float_t> &g1, 
			 const beta_dist<float_t> &g2) {
    return g1.param()==g2.param();
  }

  template<typename float_t>
  TRNG_CUDA_ENABLE
  inline bool operator!=(const beta_dist<float_t> &g1, 
			 const beta_dist<float_t> &g2) {
    return g1.param()!=g2.param();
  }
  
  // Streamable concept
  template<typename char_t, typename traits_t, typename float_t>
  std::basic_ostream<char_t, traits_t> &
  operator<<(std::basic_ostream<char_t, traits_t> &out,
	     const beta_dist<float_t> &g) {
    std::ios_base::fmtflags flags(out.flags());
    out.flags(std::ios_base::dec | std::ios_base::fixed |
	      std::ios_base::left);
    out << "[beta " << g.param() << ']';
    out.flags(flags);
    return out;
  }
  
  template<typename char_t, typename traits_t, typename float_t>
  std::basic_istream<char_t, traits_t> &
  operator>>(std::basic_istream<char_t, traits_t> &in,
	     beta_dist<float_t> &g) {
    typename beta_dist<float_t>::param_type p;
    std::ios_base::fmtflags flags(in.flags());
    in.flags(std::ios_base::dec | std::ios_base::fixed |
	     std::ios_base::left);
    in >> utility::ignore_spaces()
       >> utility::delim("[beta ") >> p >> utility::delim(']');
    if (in)
      g.param(p);
    in.flags(flags);
    return in;
  }
  
}

#endif
