﻿/* 
   A C-program for MT19937, with initialization improved 2002/2/10.
   Coded by Takuji Nishimura and Makoto Matsumoto.
   This is a faster version by taking Shawn Cokus's optimization,
   Matthe Bellew's simplification, Isaku Wada's real version.

   Before using, initialize the state by using init_genrand(seed) 
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

	 1. Redistributions of source code must retain the above copyright
		notice, this list of conditions and the following disclaimer.

	 2. Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

	 3. The names of its contributors may not be used to endorse or promote 
		products derived from this software without specific prior written 
		permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#ifndef	MTRANDH
#define	MTRANDH

#include "globaldef.h"

typedef struct MersenneTwister {
	unsigned long state[624]; /* the array for the state vector  */
	int left;
//	int initf;
	unsigned long *next;
} MersenneTwister;

typedef struct MersenneTwister64 {
	std::uint64_t mt[312];
	int mti;
} MersenneTwister64;

/* initializes mt[N] with a seed */
void init_genrand(MersenneTwister &rs,unsigned long s);

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(MersenneTwister &rs,const unsigned long init_key[],const int key_length);

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(MersenneTwister &rs);

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(MersenneTwister &rs);

/* These real versions are due to Isaku Wada, 2002/01/09 added */
/* generates a random number on [0,1]-real-interval */
double genrand_real1(MersenneTwister &rs);

/* generates a random number on [0,1)-real-interval */
double genrand_real2(MersenneTwister &rs);

/* generates a random number on (0,1)-real-interval */
double genrand_real3(MersenneTwister &rs);

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(MersenneTwister &rs);

/* initializes mt[NN] with a seed */
void init_genrand64(MersenneTwister64& rs,std::uint64_t seed);

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
void init_by_array64(MersenneTwister64& rs,const std::uint64_t init_key[],
					 std::uint64_t key_length);

/* generates a random number on [0, 2^64-1]-interval */
std::uint64_t genrand64_int64(MersenneTwister64& rs);


/* generates a random number on [0, 2^63-1]-interval */
std::int64_t genrand64_int63(MersenneTwister64& rs);

/* generates a random number on [0,1]-real-interval */
double genrand64_real1(MersenneTwister64& rs);

/* generates a random number on [0,1)-real-interval */
double genrand64_real2(MersenneTwister64& rs);

/* generates a random number on (0,1)-real-interval */
double genrand64_real3(MersenneTwister64& rs);

#endif
