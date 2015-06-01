/**
    Copyright (C) powturbo 2013-2015
    GPL v2 License
  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
//   bitutil.h - "Integer Compression" 
#include "conf.h" 
#include "bitutil.h"

#define DELTA(__p,__n, __inc, __start, __act) {\
  typeof(__p[0]) _x, *_p;\
  for(_p = __p; _p != __p+(__n&~(4-1)); ) {\
	_x = (*_p)-__start-__inc; __start = *_p++; __act;\
	_x = (*_p)-__start-__inc; __start = *_p++; __act;\
	_x = (*_p)-__start-__inc; __start = *_p++; __act;\
	_x = (*_p)-__start-__inc; __start = *_p++; __act;\
  }\
  while(_p < __p+__n) { \
    _x = *_p-__start-__inc; __start = *_p++; __act;\
  }\
}

#define UNDELTA(__p, __n, __start, __inc) { typeof(__p[0]) *_p;\
  for(_p = __p; _p != __p+(__n&~(4-1)); ) {\
    *_p = (__start += (*_p) + __inc); _p++;\
    *_p = (__start += (*_p) + __inc); _p++;\
    *_p = (__start += (*_p) + __inc); _p++;\
    *_p = (__start += (*_p) + __inc); _p++;\
  }\
  while(_p < __p+__n) { *_p = (__start += (*_p) + __inc); _p++; }\
}

unsigned bitdelta32(unsigned *in, unsigned n, unsigned *out, unsigned start, unsigned inc) {
    #ifdef __SSE2__
  unsigned *ip,b,*op = out; 
  __m128i bv = _mm_setzero_si128(), sv = _mm_set1_epi32(start), cv = _mm_set1_epi32(inc), dv;
  for(ip = in; ip != in+(n&~(4-1)); ip += 4) { 
    __m128i iv = _mm_loadu_si128((__m128i *)ip); 
	bv = _mm_or_si128(bv, dv = _mm_sub_epi32(DELTAV(iv,sv),cv)); 
	sv = iv; 
	_mm_storeu_si128((__m128i *)op, dv); 
	op += 4; 
  }
  start = (unsigned)_mm_cvtsi128_si32(_mm_srli_si128(sv,12));
  HOR(bv, b);
  while(ip < in+n) { unsigned x = *ip-start-inc; start = *ip++; b |= x; *op++ = x; }
    #else
  typeof(in[0]) b = 0,*op = out; DELTA(in, n, inc, start, b |= _x;*op++ = _x);
    #endif
  return bsr32(b);
}

unsigned bitd32(unsigned *in, unsigned n, unsigned start) {
    #ifdef __SSE2__
  unsigned *ip,b; __m128i bv = _mm_setzero_si128(), sv = _mm_set1_epi32(start);
  for(ip = in; ip != in+(n&~(4-1)); ip += 4) { 
    __m128i iv = _mm_loadu_si128((__m128i *)ip); 
	bv = _mm_or_si128(bv, DELTAV(iv,sv)); 
	sv = iv; 
  }
  
  start = (unsigned)_mm_cvtsi128_si32(_mm_srli_si128(sv,12));
  HOR(bv, b);
  while(ip < in+n) { 
    unsigned x = *ip-start; 
	start = *ip++; 
	b |= x; 
  }
    #else
  typeof(in[0]) b = 0; DELTA(in,n, 0, start, b |= _x);
    #endif
  return bsr32(b); 
}

unsigned bitd132(unsigned *in, unsigned n, unsigned start) {
    #ifdef __SSE2__
  unsigned *ip,b; __m128i bv = _mm_setzero_si128(), sv = _mm_set1_epi32(start), cv = _mm_set1_epi32(1);
  for(ip = in; ip != in+(n&~(4-1)); ip += 4) { 
    __m128i iv = _mm_loadu_si128((__m128i *)ip); 
	bv = _mm_or_si128(bv, _mm_sub_epi32(DELTAV(iv,sv),cv)); 
	sv = iv; 
  }
  
  start = (unsigned)_mm_cvtsi128_si32(_mm_srli_si128(sv,12));
  HOR(bv, b);
  while(ip < in+n) { 
    unsigned x = *ip-start-1; 
	start = *ip++; 
	b |= x; 
  }
    #else
  typeof(in[0]) b = 0; DELTA(in, n, 1, start, b |= _x);
	#endif
  return bsr32(b); 
}

void bitund32( unsigned *p, unsigned n, unsigned x) { UNDELTA(p, n, x, 0); }

void bitund132(unsigned *p, unsigned n, unsigned x) { 
    #ifdef __SSE2__
  __m128i sv = _mm_set1_epi32(x), cv = _mm_set_epi32(4,3,2,1);
  unsigned *ip;
  for(ip = p; ip != p+(n&~(4-1)); ) {
    __m128i v =  _mm_loadu_si128((__m128i *)ip); 
	VSCANI(v, sv, cv); 
	_mm_storeu_si128((__m128i *)ip, sv); 
	ip += 4;
  }
  x = (unsigned)_mm_cvtsi128_si32(_mm_srli_si128(sv,12));
  while(ip < p+n) { 
    *ip = (x += (*ip) + 1); 
	ip++; 
  }
    #else
  UNDELTA(p, n, x, 1); 
    #endif
}

void bitundx32(unsigned *p, unsigned n, unsigned x, unsigned inc) { UNDELTA(p, n, x, inc); }
