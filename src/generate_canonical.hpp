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
//     with the disctribution.
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

#if !(defined TRNG_GENERATE_CANONICAL_HPP)

#define TRNG_GENERATE_CANONICAL_HPP

#include <trng/cuda.hpp>
#include <trng/limits.hpp>
#include <trng/math.hpp>
#include <trng/utility.hpp>

namespace trng {
  
  template<typename result_type, typename R>
  TRNG_CUDA_ENABLE
  result_type generate_canonical(R &);

  namespace detail {

    template<typename result_type, typename R, typename category>
    result_type generate_canonical_impl(R &, result_type, category);
    
    class integer_tag { };
    class float_tag { };
    
    template<bool>
    struct integer_float_traits;
    
    template<>
    struct integer_float_traits<false> {
      typedef float_tag cat;
    };
    
    template<>
    struct integer_float_traits<true> {
      typedef integer_tag cat;
    };
    
    template<typename result_type, typename R>
    TRNG_CUDA_ENABLE
    inline result_type generate_canonical_impl(R &r, result_type, float_tag) {
      return utility::uniformoo<result_type>(r);
    }
  
    template<typename result_type, typename R>
    TRNG_CUDA_ENABLE
    inline result_type generate_canonical_impl(R &r, result_type, integer_tag) {
      return static_cast<result_type>(math::floor(utility::uniformco<double>(r)*(static_cast<double>(R::max)-static_cast<double>(R::min)+1.0)));
    }
    
  }
  
  template<typename result_type, typename R>
  TRNG_CUDA_ENABLE
  result_type generate_canonical(R &g) {
    return detail::generate_canonical_impl(g, result_type(), 
					   typename detail::integer_float_traits<math::numeric_limits<result_type>::is_integer>::cat());
  }
  
}

#endif
