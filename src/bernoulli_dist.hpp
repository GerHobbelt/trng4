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

#if !(defined TRNG_BERNOULLI_DIST_HPP)

#define TRNG_BERNOULLI_DIST_HPP

#include <trng/cuda.hpp>
#include <trng/limits.hpp>
#include <trng/utility.hpp>
#include <ostream>
#include <istream>

namespace trng {

  // non-uniform random number generator class
  template<typename T>
  class bernoulli_dist {
  public:
    typedef T result_type;
    class param_type;
    
    class param_type {
    private:
      double p_;
      T head_, tail_;
    public:
      TRNG_CUDA_ENABLE
      double p() const { return p_; }
      TRNG_CUDA_ENABLE
      void p(double p_new) { p_=p_new; }
      TRNG_CUDA_ENABLE
      T head() const { return head_; }
      TRNG_CUDA_ENABLE
      void head(const T &head_new) { head_=head_new; }
      TRNG_CUDA_ENABLE
      T tail() const { return tail_; }
      TRNG_CUDA_ENABLE
      void tail(const T &tail_new) { tail_=tail_new; }
      TRNG_CUDA_ENABLE
      param_type(double p, const T &head, const T &tail) :
	p_(p), head_(head), tail_(tail) {
      }
      friend class bernoulli_dist;
    };
    
  private:
    param_type P;
    
  public:
    // constructor
    TRNG_CUDA_ENABLE
    bernoulli_dist(double p, const T &head, const T &tail) : 
      P(p, head, tail) {
    }
    TRNG_CUDA_ENABLE
    explicit bernoulli_dist(const param_type &P) : P(P) {
    }
    // reset internal state
    TRNG_CUDA_ENABLE
    void reset() { }
    // random numbers
    template<typename R>
    TRNG_CUDA_ENABLE
    T operator()(R &r) {
      return utility::uniformco<double>(r)<P.p() ? P.head() : P.tail();
    }
    template<typename R>
    TRNG_CUDA_ENABLE
    T operator()(R &r, const param_type &P) {
      bernoulli_dist g(P);
      return g(r);
    }
    // property methods
    TRNG_CUDA_ENABLE
    T min() const { return P.head(); }
    TRNG_CUDA_ENABLE
    T max() const { return P.tail(); }
    TRNG_CUDA_ENABLE
    param_type param() const { return P; }
    TRNG_CUDA_ENABLE
    void param(const param_type &P_new) { P=P_new; }
    TRNG_CUDA_ENABLE
    double p() const { return P.p(); }
    TRNG_CUDA_ENABLE
    void p(double p_new) { P.p(p_new); }
    TRNG_CUDA_ENABLE
    T head() const { return P.head(); }
    TRNG_CUDA_ENABLE
    void head(const T & head_new) { P.head(head_new); }
    TRNG_CUDA_ENABLE
    T tail() const { return P.tail(); }
    TRNG_CUDA_ENABLE
    void tail(const T & tail_new) { P.tail(tail_new); }
    // probability density function  
    TRNG_CUDA_ENABLE
    double pdf(const T &x) const {
      if (x==P.head())
	return P.p();
      else if (x==P.tail())
	return 1.0-P.p();
      return 0.0;
    }
    // cumulative density function 
    TRNG_CUDA_ENABLE
    double cdf(const T &x) const {
      if (x==P.head())
	return P.p();
      else if (x==P.tail())
	return 1.0;
      return 0.0;
    }
    
  };

  // -------------------------------------------------------------------

  // EqualityComparable concept
  template<typename T>
  TRNG_CUDA_ENABLE
  inline bool operator==(const typename bernoulli_dist<T>::param_type &p1, 
			 const typename bernoulli_dist<T>::param_type &p2) {
    return p1.p()==p2.p() and p1.head()==p2.head() and p1.tail()==p2.tail();
  }
  template<typename T>
  TRNG_CUDA_ENABLE
  inline bool operator!=(const typename bernoulli_dist<T>::param_type &p1, 
			 const typename bernoulli_dist<T>::param_type &p2) {
    return !(p1==p2);
  }
  
  // Streamable concept
  template<typename char_t, typename traits_t, typename T>
  std::basic_ostream<char_t, traits_t> &
  operator<<(std::basic_ostream<char_t, traits_t> &out,
	     const typename bernoulli_dist<T>::param_type &P) {
    std::ios_base::fmtflags flags(out.flags());
    out.flags(std::ios_base::dec | std::ios_base::fixed |
	      std::ios_base::left);
    out << '('
	<< P.p() << ' ' << P.head() << ' ' << P.tail()
	<< ')';
    out.flags(flags);
    return out;
  }
  
  template<typename char_t, typename traits_t, typename T>
  std::basic_istream<char_t, traits_t> &
  operator>>(std::basic_istream<char_t, traits_t> &in,
	     typename bernoulli_dist<T>::param_type &P) {
    double p;
    T head, tail;
    std::ios_base::fmtflags flags(in.flags());
    in.flags(std::ios_base::dec | std::ios_base::fixed |
	     std::ios_base::left);
    in >> utility::delim('(')
       >> p >> utility::delim(' ')
       >> head >> utility::delim(' ')
       >> tail >> utility::delim(')');
    if (in)
      P=typename bernoulli_dist<T>::param_type(p, head, tail);
    in.flags(flags);
    return in;
  }
  
  // -------------------------------------------------------------------
  
  // EqualityComparable concept
  template<typename T>
  TRNG_CUDA_ENABLE
  inline bool operator==(const bernoulli_dist<T> &g1, 
			 const bernoulli_dist<T> &g2) {
    return g1.param()==g2.param();
  }
  template<typename T>
  TRNG_CUDA_ENABLE
  inline bool operator!=(const bernoulli_dist<T> &g1, 
			 const bernoulli_dist<T> &g2) {
    return g1.param()!=g2.param();
  }
  
  // Streamable concept
  template<typename char_t, typename traits_t, typename T>
  std::basic_ostream<char_t, traits_t> &
  operator<<(std::basic_ostream<char_t, traits_t> &out,
	     const bernoulli_dist<T> &g) {
    std::ios_base::fmtflags flags(out.flags());
    out.flags(std::ios_base::dec | std::ios_base::fixed |
	      std::ios_base::left);
    out << "[bernoulli " << g.param() << ']';
    out.flags(flags);
    return out;
  }
  
  template<typename char_t, typename traits_t, typename T>
  std::basic_istream<char_t, traits_t> &
  operator>>(std::basic_istream<char_t, traits_t> &in,
	     bernoulli_dist<T> &g) {
    typename bernoulli_dist<T>::param_type p;
    std::ios_base::fmtflags flags(in.flags());
    in.flags(std::ios_base::dec | std::ios_base::fixed |
	     std::ios_base::left);
    in >> utility::ignore_spaces()
       >> utility::delim("[bernoulli ") >> p >> utility::delim(']');
    if (in)
      g.param(p);
    in.flags(flags);
    return in;
  }
  
}

#endif

