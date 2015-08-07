#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>

#include <miner.h>

#include "crypto/mshabal.h"

#ifdef __AVX2__
#define __8WAY__
#endif

typedef mshabal_u32 hash_t[8];

#if defined(__x86_64__) && !defined(__8WAY__)
static void axiomhash4way(mshabal_context* ctx_org, void* memspace, const void *input1, void *result1,
  const void *input2, void *result2, const void *input3, void *result3, const void *input4, void *result4)
{
	mshabal_context ctx_shabal;
	int i, b;
	hash_t *hash1, *hash2, *hash3, *hash4;

	hash1 = memspace;
	memset(hash1, 0, 65536 * 32 * 4);
	hash2 = &hash1[65536 * 1];
	hash3 = &hash1[65536 * 2];
	hash4 = &hash1[65536 * 3];

	memcpy(&ctx_shabal, ctx_org, sizeof(mshabal_context));
	mshabal(&ctx_shabal, input1, input2, input3, input4, 80);
	mshabal_close(&ctx_shabal, hash1[0], hash2[0], hash3[0], hash4[0]);

	for (i = 1; i < 65536; i++)
	{
		memcpy(&ctx_shabal, ctx_org, sizeof(mshabal_context));
		mshabal(&ctx_shabal, hash1[i - 1], hash2[i - 1], hash3[i - 1], hash4[i - 1], 32);
		mshabal_close(&ctx_shabal, hash1[i], hash2[i], hash3[i], hash4[i]);
	}

	for (b = 0; b < 65536; b++)
	{
		int p = b > 0 ? b - 1 : 0xffff;
		int q1 = hash1[p][0] % 0xffff;
		int j1 = (b + q1) % 65536;
		int q2 = hash2[p][0] % 0xffff;
		int j2 = (b + q2) % 65536;
		int q3 = hash3[p][0] % 0xffff;
		int j3 = (b + q3) % 65536;
		int q4 = hash4[p][0] % 0xffff;
		int j4 = (b + q4) % 65536;

		uint8_t _hash1[64];
		uint8_t _hash2[64];
		uint8_t _hash3[64];
		uint8_t _hash4[64];

		memcpy(_hash1, hash1[p], 32);
		memcpy(&_hash1[32], hash1[j1], 32);
		memcpy(_hash2, hash2[p], 32);
		memcpy(&_hash2[32], hash2[j2], 32);
		memcpy(_hash3, hash3[p], 32);
		memcpy(&_hash3[32], hash3[j3], 32);
		memcpy(_hash4, hash4[p], 32);
		memcpy(&_hash4[32], hash4[j4], 32);

		memcpy(&ctx_shabal, ctx_org, sizeof(mshabal_context));
		mshabal(&ctx_shabal, _hash1, _hash2, _hash3, _hash4, 64);
		mshabal_close(&ctx_shabal, hash1[b], hash2[b], hash3[b], hash4[b]);
	}

	memcpy(result1, hash1[0xffff], 32);
	memcpy(result2, hash2[0xffff], 32);
	memcpy(result3, hash3[0xffff], 32);
	memcpy(result4, hash4[0xffff], 32);
}
#endif

#ifdef __8WAY__
static void axiomhash8way(mshabal8_context* ctx_org, void* memspace,
	const void *input1, void *result1,
	const void *input2, void *result2,
	const void *input3, void *result3,
	const void *input4, void *result4,
	const void *input5, void *result5,
	const void *input6, void *result6,
	const void *input7, void *result7,
	const void *input8, void *result8)
{
	mshabal8_context ctx_shabal;
	int i, b;
	hash_t *hash1, *hash2, *hash3, *hash4, *hash5, *hash6, *hash7, *hash8;

	hash1 = memspace;
	memset(hash1, 0, 65536 * 32 * 8);
	hash2 = &hash1[65536 * 1];
	hash3 = &hash1[65536 * 2];
	hash4 = &hash1[65536 * 3];
	hash5 = &hash1[65536 * 4];
	hash6 = &hash1[65536 * 5];
	hash7 = &hash1[65536 * 6];
	hash8 = &hash1[65536 * 7];

	memcpy(&ctx_shabal, ctx_org, sizeof(mshabal8_context));
	mshabal8(&ctx_shabal, input1, input2, input3, input4, input5, input6, input7, input8, 80);
	mshabal8_close(&ctx_shabal, hash1[0], hash2[0], hash3[0], hash4[0], hash5[0], hash6[0], hash7[0], hash8[0]);

	for (i = 1; i < 65536; i++)
	{
		memcpy(&ctx_shabal, ctx_org, sizeof(mshabal8_context));
		mshabal8(&ctx_shabal, hash1[i - 1], hash2[i - 1], hash3[i - 1], hash4[i - 1], hash5[i - 1], hash6[i - 1], hash7[i - 1], hash8[i - 1], 32);
		mshabal8_close(&ctx_shabal, hash1[i], hash2[i], hash3[i], hash4[i], hash5[i], hash6[i], hash7[i], hash8[i]);
	}

	for (b = 0; b < 65536; b++)
	{
		int p = b > 0 ? b - 1 : 0xffff;
		int q1 = hash1[p][0] % 0xffff;
		int j1 = (b + q1) % 65536;
		int q2 = hash2[p][0] % 0xffff;
		int j2 = (b + q2) % 65536;
		int q3 = hash3[p][0] % 0xffff;
		int j3 = (b + q3) % 65536;
		int q4 = hash4[p][0] % 0xffff;
		int j4 = (b + q4) % 65536;

		int q5 = hash5[p][0] % 0xffff;
		int j5 = (b + q5) % 65536;
		int q6 = hash6[p][0] % 0xffff;
		int j6 = (b + q6) % 65536;
		int q7 = hash7[p][0] % 0xffff;
		int j7 = (b + q7) % 65536;
		int q8 = hash8[p][0] % 0xffff;
		int j8 = (b + q8) % 65536;

		uint8_t _hash1[64];
		uint8_t _hash2[64];
		uint8_t _hash3[64];
		uint8_t _hash4[64];
		uint8_t _hash5[64];
		uint8_t _hash6[64];
		uint8_t _hash7[64];
		uint8_t _hash8[64];

		memcpy(_hash1, hash1[p], 32);
		memcpy(&_hash1[32], hash1[j1], 32);
		memcpy(_hash2, hash2[p], 32);
		memcpy(&_hash2[32], hash2[j2], 32);
		memcpy(_hash3, hash3[p], 32);
		memcpy(&_hash3[32], hash3[j3], 32);
		memcpy(_hash4, hash4[p], 32);
		memcpy(&_hash4[32], hash4[j4], 32);

		memcpy(_hash5, hash5[p], 32);
		memcpy(&_hash5[32], hash5[j5], 32);
		memcpy(_hash6, hash6[p], 32);
		memcpy(&_hash6[32], hash6[j6], 32);
		memcpy(_hash7, hash7[p], 32);
		memcpy(&_hash7[32], hash7[j7], 32);
		memcpy(_hash8, hash8[p], 32);
		memcpy(&_hash8[32], hash8[j8], 32);

		memcpy(&ctx_shabal, ctx_org, sizeof(mshabal8_context));
		mshabal8(&ctx_shabal, _hash1, _hash2, _hash3, _hash4, _hash5, _hash6, _hash7, _hash8, 64);
		mshabal8_close(&ctx_shabal, hash1[b], hash2[b], hash3[b], hash4[b], hash5[b], hash6[b], hash7[b], hash8[b]);
	}

	memcpy(result1, hash1[0xffff], 32);
	memcpy(result2, hash2[0xffff], 32);
	memcpy(result3, hash3[0xffff], 32);
	memcpy(result4, hash4[0xffff], 32);
	memcpy(result5, hash5[0xffff], 32);
	memcpy(result6, hash6[0xffff], 32);
	memcpy(result7, hash7[0xffff], 32);
	memcpy(result8, hash8[0xffff], 32);
}
#endif

#ifdef __x86_64__
int scanhash_axiom(int thr_id, uint32_t *pdata, const uint32_t *ptarget,
	uint32_t max_nonce, uint64_t *hashes_done, uint32_t *nonces, int *nonces_len)
{
#ifdef __8WAY__
#define HASHES 8
#else
#define HASHES 4
#endif

	uint32_t _ALIGN(128) hash64_1[8], hash64_2[8], hash64_3[8], hash64_4[8]
#ifdef __8WAY__
		, hash64_5[8], hash64_6[8], hash64_7[8], hash64_8[8]
#endif
		;

	uint32_t _ALIGN(128) endiandata_1[20], endiandata_2[20], endiandata_3[20], endiandata_4[20]
#ifdef __8WAY__
		, endiandata_5[20], endiandata_6[20], endiandata_7[20], endiandata_8[20]
#endif
		;

#ifdef __8WAY__
	mshabal8_context ctx_org;
#else
	mshabal_context ctx_org;
#endif
	void* memspace;

	const uint32_t Htarg = ptarget[7];
	const uint32_t first_nonce = pdata[19];

	uint32_t n = first_nonce;

	*nonces_len = 0;

	// to avoid small chance of generating duplicate shares
	max_nonce = (max_nonce / HASHES) * HASHES;

	for (int i = 0; i < 19; i++) {
		be32enc(&endiandata_1[i], pdata[i]);
	}

	memcpy(endiandata_2, endiandata_1, sizeof(endiandata_1));
	memcpy(endiandata_3, endiandata_1, sizeof(endiandata_1));
	memcpy(endiandata_4, endiandata_1, sizeof(endiandata_1));
#ifdef __8WAY__
	memcpy(endiandata_5, endiandata_1, sizeof(endiandata_1));
	memcpy(endiandata_6, endiandata_1, sizeof(endiandata_1));
	memcpy(endiandata_7, endiandata_1, sizeof(endiandata_1));
	memcpy(endiandata_8, endiandata_1, sizeof(endiandata_1));
#endif

#ifdef __8WAY__
	mshabal8_init(&ctx_org, 256);
#else
	mshabal_init(&ctx_org, 256);
#endif
	memspace = malloc(65536 * 32 * HASHES);

	do {
		be32enc(&endiandata_1[19], n);
		be32enc(&endiandata_2[19], n + 1);
		be32enc(&endiandata_3[19], n + 2);
		be32enc(&endiandata_4[19], n + 3);
#ifdef __8WAY__
		be32enc(&endiandata_5[19], n + 4);
		be32enc(&endiandata_6[19], n + 5);
		be32enc(&endiandata_7[19], n + 6);
		be32enc(&endiandata_8[19], n + 7);
#endif

#ifdef __8WAY__
		axiomhash8way(&ctx_org, memspace, endiandata_1, hash64_1, endiandata_2, hash64_2, endiandata_3, hash64_3, endiandata_4, hash64_4,
			endiandata_5, hash64_5, endiandata_6, hash64_6, endiandata_7, hash64_7, endiandata_8, hash64_8);
#else
		axiomhash4way(&ctx_org, memspace, endiandata_1, hash64_1, endiandata_2, hash64_2, endiandata_3, hash64_3, endiandata_4, hash64_4);
#endif

		if (hash64_1[7] < Htarg && fulltest(hash64_1, ptarget)) {
			nonces[(*nonces_len)++] = n;
		}
		if (hash64_2[7] < Htarg && fulltest(hash64_2, ptarget)) {
			nonces[(*nonces_len)++] = n + 1;
		}
		if (hash64_3[7] < Htarg && fulltest(hash64_3, ptarget)) {
			nonces[(*nonces_len)++] = n + 2;
		}
		if (hash64_4[7] < Htarg && fulltest(hash64_4, ptarget)) {
			nonces[(*nonces_len)++] = n + 3;
		}

#ifdef __8WAY__
		if (hash64_5[7] < Htarg && fulltest(hash64_5, ptarget)) {
			nonces[(*nonces_len)++] = n + 4;
		}
		if (hash64_6[7] < Htarg && fulltest(hash64_6, ptarget)) {
			nonces[(*nonces_len)++] = n + 5;
		}
		if (hash64_7[7] < Htarg && fulltest(hash64_7, ptarget)) {
			nonces[(*nonces_len)++] = n + 6;
		}
		if (hash64_8[7] < Htarg && fulltest(hash64_8, ptarget)) {
			nonces[(*nonces_len)++] = n + 7;
		}
#endif

		n += HASHES;

		if ((*nonces_len) > 0)
		{
			*hashes_done = n - first_nonce;
			pdata[19] = nonces[0];
			free(memspace);
			return (*nonces_len);
		}

	} while (n < max_nonce && !work_restart[thr_id].restart);

	*hashes_done = n - first_nonce;
	pdata[19] = n;
	free(memspace);
	return (*nonces_len);
}

#endif /* __i386__ (intel only) */

