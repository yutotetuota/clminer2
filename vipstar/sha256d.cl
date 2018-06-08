OCLSTRINGIFY(
/* Elementary functions used by SHA256 */
\n#define Ch(x, y, z)     ((x & (y ^ z)) ^ z)\n
\n#define Maj(x, y, z)    ((x & (y | z)) | (y & z))\n
\n#define ROTR(x, n)      ((x >> n) | (x << (32 - n)))\n
\n#define S0(x)           (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))\n
\n#define S1(x)           (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))\n
\n#define s0(x)           (ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3))\n
\n#define s1(x)           (ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10))\n

/* SHA256 round function */
\n#define RND(a, b, c, d, e, f, g, h, k) \
	do { \
		t0 = h + S1(e) + Ch(e, f, g) + k; \
		t1 = S0(a) + Maj(a, b, c); \
		d += t0; \
		h  = t0 + t1; \
	} while (0)\n

\n#define swab32(x) ((((x) << 24) & 0xff000000u) | (((x) << 8) & 0x00ff0000u) \
                   | (((x) >> 8) & 0x0000ff00u) | (((x) >> 24) & 0x000000ffu))\n

\n#ifdef VECTOR_WIDTH_1\n
	\n#define vec_uint uint\n
	\n#define VECTOR_WIDTH 1\n
	\n#define IOTA (vec_uint)(0)\n
\n#elif VECTOR_WIDTH_2\n
	\n#define vec_uint uint2\n
	\n#define VECTOR_WIDTH 2\n
	\n#define IOTA (vec_uint)(0, 1)\n
\n#elif VECTOR_WIDTH_4\n
	\n#define vec_uint uint4\n
	\n#define VECTOR_WIDTH 4\n
	\n#define IOTA (vec_uint)(0, 1, 2, 3)\n
\n#elif VECTOR_WIDTH_8\n
	\n#define vec_uint uint8\n
	\n#define VECTOR_WIDTH 8\n
	\n#define IOTA (vec_uint)(0, 1, 2, 3, 4, 5, 6, 7)\n
\n#elif VECTOR_WIDTH_16\n
	\n#define vec_uint uint16\n
	\n#define VECTOR_WIDTH 16\n
	\n#define IOTA (vec_uint)(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)\n
\n#endif\n

\n#ifdef USE_CL_NV_COMPILER_OPTIONS\n
\n#pragma cl_nv_compiler_options\n
\n#endif\n

__kernel void sha256d_vips_ms_cl(__global uint *hash)\n
{\n
	__private uint threads = 1;\n
	
	for(int i = 0; i < get_work_dim(); i++)\n
		threads *= get_global_size(i);\n
	__private vec_uint W[128 + 8 + 8];
	__local vec_uint WS[128 + 8 + 8];
	hash[1] = UINT_MAX;

	__local vec_uint W16, W17, s0W17, dummy;
	W16 = hash[4];
	W17 = hash[5];
	s0W17 = hash[6];
	dummy = hash[7];

	#ifdef USE_UNROLL
	#pragma unroll
	#endif
	for(int i = 0; i < 128 + 8 + 8; i++)\n
		W[i] = hash[8 + i];\n


	#ifdef USE_UNROLL
	#pragma unroll
	#endif
	for(int i = 0; i < 128 + 8 + 8; i++)\n
		WS[i] = hash[8 + i];\n


	__local vec_uint *midstate = WS + 128;\n
	__local vec_uint *prehash = midstate + 8;\n
	__private vec_uint S[8];\n
	__local vec_uint *W2S;
	__private vec_uint t0, t1;\n
	__private uint idx = get_global_size(0) * get_global_id(1) + get_global_id(0);\n
	__local uint Htarg;
	 Htarg = hash[2];

	barrier(CLK_LOCAL_MEM_FENCE);
	
	//update nonce
	__private vec_uint nonce = WS[3] + idx * VECTOR_WIDTH;\n
	nonce += IOTA;

	#ifdef USE_UNROLL
	#pragma unroll
	#endif
	for(int j = 0; j < 32; j++){
		nonce += threads * VECTOR_WIDTH;

		//first round2
		W[18] = WS[18] +                      s0(nonce);\n
		W[19] = WS[19] +                                nonce;\n
		W[20] = WS[20] + s1(W[18]);\n
		W[21] = WS[21] + s1(W[19]);\n
		W[22] = WS[22] + s1(W[20]);\n
		W[23] = WS[23] + s1(W[21]);\n
		W[24] = WS[24] + s1(W[22]);\n
		W[25] = WS[25] + s1(W[23]) + W[18];
		W[26] = WS[26] + s1(W[24]) + W[19];
		W[27] = WS[27] + s1(W[25]) + W[20];
		W[28] = WS[28] + s1(W[26]) + W[21];
		W[29] = WS[29] + s1(W[27]) + W[22];
		W[30] = WS[30] + s1(W[28]) + W[23];
		W[31] = WS[31] + s1(W[29]) + W[24];

		W[32]       = s1(W[30]) + W[25] + s0W17 + W16;
		W[33] = s1(W[31]) + W[26] + s0(W[18]) + W17;
		
		#ifdef USE_UNROLL
		#pragma unroll
		#endif
		for (int i = 34; i < 64; i += 2) {
			W[i]       = s1(W[(i - 2)]) + W[(i - 7)] + s0(W[(i - 15)]) + W[(i - 16)];
			W[(i + 1)] = s1(W[(i - 1)]) + W[(i - 6)] + s0(W[(i - 14)]) + W[(i - 15)];
		}

		vec_uint a = prehash[0];
		vec_uint b = prehash[1];
		vec_uint c = prehash[2];
		vec_uint d = prehash[3];
		vec_uint e = prehash[4];
		vec_uint f = prehash[5];
		vec_uint g = prehash[6];
		vec_uint h = prehash[7];

		RND(f, g, h, a,	b, c, d, e, nonce + 0xe9b5dba5u);
		RND(e, f, g, h, a, b, c, d, WS[ 4]/* + 0x3956c25bu*/);
		RND(d, e, f, g, h, a, b, c, WS[ 5]/* + 0x59f111f1u*/);
		RND(c, d, e, f, g, h, a, b, WS[ 6]/* + 0x923f82a4u*/);
		RND(b, c, d, e, f, g, h, a, WS[ 7]/* + 0xab1c5ed5u*/);
		RND(a, b, c, d, e, f, g, h, WS[ 8]/* + 0xd807aa98u*/);
		RND(h, a, b, c, d, e, f, g, WS[ 9]/* + 0x12835b01u*/);
		RND(g, h, a, b, c, d, e, f, WS[10]/* + 0x243185beu*/);
		RND(f, g, h, a,	b, c, d, e, WS[11]/* + 0x550c7dc3u*/);
		RND(e, f, g, h, a, b, c, d, WS[12]/* + 0x72be5d74u*/);
		RND(d, e, f, g, h, a, b, c, WS[13]/* + 0x80deb1feu*/);
		RND(c, d, e, f, g, h, a, b, WS[14]/* + 0x9bdc06a7u*/);
		RND(b, c, d, e, f, g, h, a, WS[15]/* + 0xc19bf174u*/);
		RND(a, b, c, d, e, f, g, h, WS[16]/* + 0xe49b69c1u*/);
		RND(h, a, b, c, d, e, f, g, WS[17]/* + 0xefbe4786u*/);
		RND(g, h, a, b, c, d, e, f, W[18] + 0x0fc19dc6u);
		RND(f, g, h, a, b, c, d, e, W[19] + 0x240ca1ccu);
		RND(e, f, g, h, a, b, c, d, W[20] + 0x2de92c6fu);
		RND(d, e, f, g, h, a, b, c, W[21] + 0x4a7484aau);
		RND(c, d, e, f, g, h, a, b, W[22] + 0x5cb0a9dcu);
		RND(b, c, d, e, f, g, h, a, W[23] + 0x76f988dau);
		RND(a, b, c, d, e, f, g, h, W[24] + 0x983e5152u);
		RND(h, a, b, c, d, e, f, g, W[25] + 0xa831c66du);
		RND(g, h, a, b, c, d, e, f, W[26] + 0xb00327c8u);
		RND(f, g, h, a, b, c, d, e, W[27] + 0xbf597fc7u);
		RND(e, f, g, h, a, b, c, d, W[28] + 0xc6e00bf3u);
		RND(d, e, f, g, h, a, b, c, W[29] + 0xd5a79147u);
		RND(c, d, e, f, g, h, a, b, W[30] + 0x06ca6351u);
		RND(b, c, d, e, f, g, h, a, W[31] + 0x14292967u);
		RND(a, b, c, d, e, f, g, h, W[32] + 0x27b70a85u);
		RND(h, a, b, c, d, e, f, g, W[33] + 0x2e1b2138u);
		RND(g, h, a, b, c, d, e, f, W[34] + 0x4d2c6dfcu);
		RND(f, g, h, a, b, c, d, e, W[35] + 0x53380d13u);
		RND(e, f, g, h, a, b, c, d, W[36] + 0x650a7354u);
		RND(d, e, f, g, h, a, b, c, W[37] + 0x766a0abbu);
		RND(c, d, e, f, g, h, a, b, W[38] + 0x81c2c92eu);
		RND(b, c, d, e, f, g, h, a, W[39] + 0x92722c85u);
		RND(a, b, c, d, e, f, g, h, W[40] + 0xa2bfe8a1u);
		RND(h, a, b, c, d, e, f, g, W[41] + 0xa81a664bu);
		RND(g, h, a, b, c, d, e, f, W[42] + 0xc24b8b70u);
		RND(f, g, h, a, b, c, d, e, W[43] + 0xc76c51a3u);
		RND(e, f, g, h, a, b, c, d, W[44] + 0xd192e819u);
		RND(d, e, f, g, h, a, b, c, W[45] + 0xd6990624u);
		RND(c, d, e, f, g, h, a, b, W[46] + 0xf40e3585u);
		RND(b, c, d, e, f, g, h, a, W[47] + 0x106aa070u);
		RND(a, b, c, d, e, f, g, h, W[48] + 0x19a4c116u);
		RND(h, a, b, c, d, e, f, g, W[49] + 0x1e376c08u);
		RND(g, h, a, b, c, d, e, f, W[50] + 0x2748774cu);
		RND(f, g, h, a,	b, c, d, e, W[51] + 0x34b0bcb5u);
		RND(e, f, g, h, a, b, c, d, W[52] + 0x391c0cb3u);
		RND(d, e, f, g, h, a, b, c, W[53] + 0x4ed8aa4au);
		RND(c, d, e, f, g, h, a, b, W[54] + 0x5b9cca4fu);
		RND(b, c, d, e, f, g, h, a, W[55] + 0x682e6ff3u);
		RND(a, b, c, d, e, f, g, h, W[56] + 0x748f82eeu);
		RND(h, a, b, c, d, e, f, g, W[57] + 0x78a5636fu);
		RND(g, h, a, b, c, d, e, f, W[58] + 0x84c87814u);
		RND(f, g, h, a,	b, c, d, e, W[59] + 0x8cc70208u);
		RND(e, f, g, h, a, b, c, d, W[60] + 0x90befffau);
		RND(d, e, f, g, h, a, b, c, W[61] + 0xa4506cebu);
		RND(c, d, e, f, g, h, a, b, W[62] + 0xbef9a3f7u);
		RND(b, c, d, e, f, g, h, a, W[63] + 0xc67178f2u);


		S[0] = midstate[0] + a;
		S[1] = midstate[1] + b;
		S[2] = midstate[2] + c;
		S[3] = midstate[3] + d;
		S[4] = midstate[4] + e;
		S[5] = midstate[5] + f;
		S[6] = midstate[6] + g;
		S[7] = midstate[7] + h;
		

		W2S = WS + 64;


		a = S[0];
		b = S[1];
		c = S[2];
		d = S[3];
		e = S[4];
		f = S[5];
		g = S[6];
		h = S[7];

		RND(a, b, c, d, e, f, g, h, W2S[ 0]/* + 0x428a2f98u*/);
		RND(h, a, b, c, d, e, f, g, W2S[ 1]/* + 0x71374491u*/);
		RND(g, h, a, b, c, d, e, f, W2S[ 2]/* + 0xb5c0fbcfu*/);
		RND(f, g, h, a, b, c, d, e, W2S[ 3]/* + 0xe9b5dba5u*/);
		RND(e, f, g, h, a, b, c, d, W2S[ 4]/* + 0x3956c25bu*/);
		RND(d, e, f, g, h, a, b, c, W2S[ 5]/* + 0x59f111f1u*/);
		RND(c, d, e, f, g, h, a, b, W2S[ 6]/* + 0x923f82a4u*/);
		RND(b, c, d, e, f, g, h, a, W2S[ 7]/* + 0xab1c5ed5u*/);
		RND(a, b, c, d, e, f, g, h, W2S[ 8]/* + 0xd807aa98u*/);
		RND(h, a, b, c, d, e, f, g, W2S[ 9]/* + 0x12835b01u*/);
		RND(g, h, a, b, c, d, e, f, W2S[10]/* + 0x243185beu*/);
		RND(f, g, h, a,	b, c, d, e, W2S[11]/* + 0x550c7dc3u*/);
		RND(e, f, g, h, a, b, c, d, W2S[12]/* + 0x72be5d74u*/);
		RND(d, e, f, g, h, a, b, c, W2S[13]/* + 0x80deb1feu*/);
		RND(c, d, e, f, g, h, a, b, W2S[14]/* + 0x9bdc06a7u*/);
		RND(b, c, d, e, f, g, h, a, W2S[15]/* + 0xc19bf174u*/);
		RND(a, b, c, d, e, f, g, h, W2S[16]/* + 0xe49b69c1u*/);
		RND(h, a, b, c, d, e, f, g, W2S[17]/* + 0xefbe4786u*/);
		RND(g, h, a, b, c, d, e, f, W2S[18]/* + 0x0fc19dc6u*/);
		RND(f, g, h, a,	b, c, d, e, W2S[19]/* + 0x240ca1ccu*/);
		RND(e, f, g, h, a, b, c, d, W2S[20]/* + 0x2de92c6fu*/);
		RND(d, e, f, g, h, a, b, c, W2S[21]/* + 0x4a7484aau*/);
		RND(c, d, e, f, g, h, a, b, W2S[22]/* + 0x5cb0a9dcu*/);
		RND(b, c, d, e, f, g, h, a, W2S[23]/* + 0x76f988dau*/);
		RND(a, b, c, d, e, f, g, h, W2S[24]/* + 0x983e5152u*/);
		RND(h, a, b, c, d, e, f, g, W2S[25]/* + 0xa831c66du*/);
		RND(g, h, a, b, c, d, e, f, W2S[26]/* + 0xb00327c8u*/);
		RND(f, g, h, a,	b, c, d, e, W2S[27]/* + 0xbf597fc7u*/);
		RND(e, f, g, h, a, b, c, d, W2S[28]/* + 0xc6e00bf3u*/);
		RND(d, e, f, g, h, a, b, c, W2S[29]/* + 0xd5a79147u*/);
		RND(c, d, e, f, g, h, a, b, W2S[30]/* + 0x06ca6351u*/);
		RND(b, c, d, e, f, g, h, a, W2S[31]/* + 0x14292967u*/);
		RND(a, b, c, d, e, f, g, h, W2S[32]/* + 0x27b70a85u*/);
		RND(h, a, b, c, d, e, f, g, W2S[33]/* + 0x2e1b2138u*/);
		RND(g, h, a, b, c, d, e, f, W2S[34]/* + 0x4d2c6dfcu*/);
		RND(f, g, h, a,	b, c, d, e, W2S[35]/* + 0x53380d13u*/);
		RND(e, f, g, h, a, b, c, d, W2S[36]/* + 0x650a7354u*/);
		RND(d, e, f, g, h, a, b, c, W2S[37]/* + 0x766a0abbu*/);
		RND(c, d, e, f, g, h, a, b, W2S[38]/* + 0x81c2c92eu*/);
		RND(b, c, d, e, f, g, h, a, W2S[39]/* + 0x92722c85u*/);
		RND(a, b, c, d, e, f, g, h, W2S[40]/* + 0xa2bfe8a1u*/);
		RND(h, a, b, c, d, e, f, g, W2S[41]/* + 0xa81a664bu*/);
		RND(g, h, a, b, c, d, e, f, W2S[42]/* + 0xc24b8b70u*/);
		RND(f, g, h, a,	b, c, d, e, W2S[43]/* + 0xc76c51a3u*/);
		RND(e, f, g, h, a, b, c, d, W2S[44]/* + 0xd192e819u*/);
		RND(d, e, f, g, h, a, b, c, W2S[45]/* + 0xd6990624u*/);
		RND(c, d, e, f, g, h, a, b, W2S[46]/* + 0xf40e3585u*/);
		RND(b, c, d, e, f, g, h, a, W2S[47]/* + 0x106aa070u*/);
		RND(a, b, c, d, e, f, g, h, W2S[48]/* + 0x19a4c116u*/);
		RND(h, a, b, c, d, e, f, g, W2S[49]/* + 0x1e376c08u*/);
		RND(g, h, a, b, c, d, e, f, W2S[50]/* + 0x2748774cu*/);
		RND(f, g, h, a,	b, c, d, e, W2S[51]/* + 0x34b0bcb5u*/);
		RND(e, f, g, h, a, b, c, d, W2S[52]/* + 0x391c0cb3u*/);
		RND(d, e, f, g, h, a, b, c, W2S[53]/* + 0x4ed8aa4au*/);
		RND(c, d, e, f, g, h, a, b, W2S[54]/* + 0x5b9cca4fu*/);
		RND(b, c, d, e, f, g, h, a, W2S[55]/* + 0x682e6ff3u*/);
		RND(a, b, c, d, e, f, g, h, W2S[56]/* + 0x748f82eeu*/);
		RND(h, a, b, c, d, e, f, g, W2S[57]/* + 0x78a5636fu*/);
		RND(g, h, a, b, c, d, e, f, W2S[58]/* + 0x84c87814u*/);
		RND(f, g, h, a,	b, c, d, e, W2S[59]/* + 0x8cc70208u*/);
		RND(e, f, g, h, a, b, c, d, W2S[60]/* + 0x90befffau*/);
		RND(d, e, f, g, h, a, b, c, W2S[61]/* + 0xa4506cebu*/);
		RND(c, d, e, f, g, h, a, b, W2S[62]/* + 0xbef9a3f7u*/);
		RND(b, c, d, e, f, g, h, a, W2S[63]/* + 0xc67178f2u*/);


		W[0] = S[0] + a;
		W[1] = S[1] + b;
		W[2] = S[2] + c;
		W[3] = S[3] + d;
		W[4] = S[4] + e;
		W[5] = S[5] + f;
		W[6] = S[6] + g;
		W[7] = S[7] + h;


		//second
		W[16] =                     s0(W[ 1]) + W[ 0];
		W[17] = s1(0x00000100u)   + s0(W[ 2]) + W[ 1];
		W[18] = s1(W[16])         + s0(W[ 3]) + W[ 2];
		W[19] = s1(W[17])         + s0(W[ 4]) + W[ 3];
		W[20] = s1(W[18])         + s0(W[ 5]) + W[ 4];
		W[21] = s1(W[19])         + s0(W[ 6]) + W[ 5];
		W[22] = s1(W[20]) + 0x00000100u + s0(W[ 7]) + W[ 6];
		W[23] = s1(W[21]) + W[16] + s0(0x80000000u) + W[ 7];
		W[24] = s1(W[22]) + W[17]             + 0x80000000u;
		W[25] = s1(W[23]) + W[18];
		W[26] = s1(W[24]) + W[19];
		W[27] = s1(W[25]) + W[20];
		W[28] = s1(W[26]) + W[21];
		W[29] = s1(W[27]) + W[22];
		W[30] = s1(W[28]) + W[23] + s0(0x00000100u);
		W[31] = s1(W[29]) + W[24] + s0(W[16]) + 0x00000100u;
		
		#ifdef USE_UNROLL
		#pragma unroll
		#endif
		for (int i = 32; i < 60; i += 2) {
			W[i]   = s1(W[(i - 2)]) + W[(i - 7)] + s0(W[(i - 15)]) + W[(i - 16)];
			W[(i + 1)] = s1(W[(i - 1)]) + W[(i - 6)] + s0(W[(i - 14)]) + W[(i - 15)];
		}
		W[60] = s1(W[58]) + W[53] + s0(W[45]) + W[44];


		//hash init
		a = 0x6a09e667u;
		b = 0xbb67ae85u;
		c = 0x3c6ef372u;
		d = 0xa54ff53au;
		e = 0x510e527fu;
		f = 0x9b05688cu;
		g = 0x1f83d9abu;
		h = 0x5be0cd19u;


		RND(a, b, c, d, e, f, g, h, W[ 0] + 0x428a2f98u);
		RND(h, a, b, c, d, e, f, g, W[ 1] + 0x71374491u);
		RND(g, h, a, b, c, d, e, f, W[ 2] + 0xb5c0fbcfu);
		RND(f, g, h, a, b, c, d, e, W[ 3] + 0xe9b5dba5u);
		RND(e, f, g, h, a, b, c, d, W[ 4] + 0x3956c25bu);
		RND(d, e, f, g, h, a, b, c, W[ 5] + 0x59f111f1u);
		RND(c, d, e, f, g, h, a, b, W[ 6] + 0x923f82a4u);
		RND(b, c, d, e, f, g, h, a, W[ 7] + 0xab1c5ed5u);
		RND(a, b, c, d, e, f, g, h, 0x80000000u + 0xd807aa98u);
		RND(h, a, b, c, d, e, f, g, 0x00000000u + 0x12835b01u);
		RND(g, h, a, b, c, d, e, f, 0x00000000u + 0x243185beu);
		RND(f, g, h, a,	b, c, d, e, 0x00000000u + 0x550c7dc3u);
		RND(e, f, g, h, a, b, c, d, 0x00000000u + 0x72be5d74u);
		RND(d, e, f, g, h, a, b, c, 0x00000000u + 0x80deb1feu);
		RND(c, d, e, f, g, h, a, b, 0x00000000u + 0x9bdc06a7u);
		RND(b, c, d, e, f, g, h, a, 0x00000100u + 0xc19bf174u);
		RND(a, b, c, d, e, f, g, h, W[16] + 0xe49b69c1u);
		RND(h, a, b, c, d, e, f, g, W[17] + 0xefbe4786u);
		RND(g, h, a, b, c, d, e, f, W[18] + 0x0fc19dc6u);
		RND(f, g, h, a,	b, c, d, e, W[19] + 0x240ca1ccu);
		RND(e, f, g, h, a, b, c, d, W[20] + 0x2de92c6fu);
		RND(d, e, f, g, h, a, b, c, W[21] + 0x4a7484aau);
		RND(c, d, e, f, g, h, a, b, W[22] + 0x5cb0a9dcu);
		RND(b, c, d, e, f, g, h, a, W[23] + 0x76f988dau);
		RND(a, b, c, d, e, f, g, h, W[24] + 0x983e5152u);
		RND(h, a, b, c, d, e, f, g, W[25] + 0xa831c66du);
		RND(g, h, a, b, c, d, e, f, W[26] + 0xb00327c8u);
		RND(f, g, h, a,	b, c, d, e, W[27] + 0xbf597fc7u);
		RND(e, f, g, h, a, b, c, d, W[28] + 0xc6e00bf3u);
		RND(d, e, f, g, h, a, b, c, W[29] + 0xd5a79147u);
		RND(c, d, e, f, g, h, a, b, W[30] + 0x06ca6351u);
		RND(b, c, d, e, f, g, h, a, W[31] + 0x14292967u);
		RND(a, b, c, d, e, f, g, h, W[32] + 0x27b70a85u);
		RND(h, a, b, c, d, e, f, g, W[33] + 0x2e1b2138u);
		RND(g, h, a, b, c, d, e, f, W[34] + 0x4d2c6dfcu);
		RND(f, g, h, a,	b, c, d, e, W[35] + 0x53380d13u);
		RND(e, f, g, h, a, b, c, d, W[36] + 0x650a7354u);
		RND(d, e, f, g, h, a, b, c, W[37] + 0x766a0abbu);
		RND(c, d, e, f, g, h, a, b, W[38] + 0x81c2c92eu);
		RND(b, c, d, e, f, g, h, a, W[39] + 0x92722c85u);
		RND(a, b, c, d, e, f, g, h, W[40] + 0xa2bfe8a1u);
		RND(h, a, b, c, d, e, f, g, W[41] + 0xa81a664bu);
		RND(g, h, a, b, c, d, e, f, W[42] + 0xc24b8b70u);
		RND(f, g, h, a,	b, c, d, e, W[43] + 0xc76c51a3u);
		RND(e, f, g, h, a, b, c, d, W[44] + 0xd192e819u);
		RND(d, e, f, g, h, a, b, c, W[45] + 0xd6990624u);
		RND(c, d, e, f, g, h, a, b, W[46] + 0xf40e3585u);
		RND(b, c, d, e, f, g, h, a, W[47] + 0x106aa070u);
		RND(a, b, c, d, e, f, g, h, W[48] + 0x19a4c116u);
		RND(h, a, b, c, d, e, f, g, W[49] + 0x1e376c08u);
		RND(g, h, a, b, c, d, e, f, W[50] + 0x2748774cu);
		RND(f, g, h, a,	b, c, d, e, W[51] + 0x34b0bcb5u);
		RND(e, f, g, h, a, b, c, d, W[52] + 0x391c0cb3u);
		RND(d, e, f, g, h, a, b, c, W[53] + 0x4ed8aa4au);
		RND(c, d, e, f, g, h, a, b, W[54] + 0x5b9cca4fu);
		RND(b, c, d, e, f, g, h, a, W[55] + 0x682e6ff3u);
		RND(a, b, c, d, e, f, g, h, W[56] + 0x748f82eeu);


		c += g + S1(d) + Ch(d, e, f) + W[57] + 0x78a5636fu;
		b += f + S1(c) + Ch(c, d, e) + W[58] + 0x84c87814u;
		a += e + S1(b) + Ch(b, c, d) + W[59] + 0x8cc70208u;
		h += d + S1(a) + Ch(a, b, c) + W[60] + 0x90befffau + 0x5be0cd19u;

		#ifdef VECTOR_WIDTH_1
		if(swab32(h) <= Htarg){
			atomic_xchg(hash[1], h);
			atomic_xchg(hash[0], nonce);
		}
		#else
		__private uint *h_ptr = (uint*)&h;
		
		#ifdef USE_UNROLL
		#pragma unroll
		#endif
		for(int k = 0; k < VECTOR_WIDTH; k++){
			if(swab32(h_ptr[k]) <= Htarg){
				hash[1] = ((uint*)&h)[k];
				hash[0] = ((uint*)&nonce)[k];
			}
		}
		#endif
	}

	if(idx == 0)
		hash[11] += threads * 32 * VECTOR_WIDTH;
}
)
