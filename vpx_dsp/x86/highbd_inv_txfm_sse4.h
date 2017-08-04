/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_X86_HIGHBD_INV_TXFM_SSE4_H_
#define VPX_DSP_X86_HIGHBD_INV_TXFM_SSE4_H_

#include <smmintrin.h>  // SSE4.1

#include "./vpx_config.h"
#include "vpx_dsp/x86/highbd_inv_txfm_sse2.h"

static INLINE __m128i multiplication_round_shift_sse4_1(
    const __m128i *const in /*in[2]*/, const int c) {
  const __m128i pair_c = pair_set_epi32(c * 4, 0);
  __m128i t0, t1;

  t0 = _mm_mul_epi32(in[0], pair_c);
  t1 = _mm_mul_epi32(in[1], pair_c);
  t0 = dct_const_round_shift_64bit(t0);
  t1 = dct_const_round_shift_64bit(t1);

  return pack_4(t0, t1);
}

static INLINE void highbd_butterfly_sse4_1(const __m128i in0, const __m128i in1,
                                           const int c0, const int c1,
                                           __m128i *const out0,
                                           __m128i *const out1) {
  const __m128i pair_c0 = pair_set_epi32(4 * c0, 0);
  const __m128i pair_c1 = pair_set_epi32(4 * c1, 0);
  __m128i temp1[4], temp2[4];

  extend_64bit(in0, temp1);
  extend_64bit(in1, temp2);
  temp1[2] = _mm_mul_epi32(temp1[0], pair_c1);
  temp1[3] = _mm_mul_epi32(temp1[1], pair_c1);
  temp1[0] = _mm_mul_epi32(temp1[0], pair_c0);
  temp1[1] = _mm_mul_epi32(temp1[1], pair_c0);
  temp2[2] = _mm_mul_epi32(temp2[0], pair_c0);
  temp2[3] = _mm_mul_epi32(temp2[1], pair_c0);
  temp2[0] = _mm_mul_epi32(temp2[0], pair_c1);
  temp2[1] = _mm_mul_epi32(temp2[1], pair_c1);
  temp1[0] = _mm_sub_epi64(temp1[0], temp2[0]);
  temp1[1] = _mm_sub_epi64(temp1[1], temp2[1]);
  temp2[0] = _mm_add_epi64(temp1[2], temp2[2]);
  temp2[1] = _mm_add_epi64(temp1[3], temp2[3]);
  temp1[0] = dct_const_round_shift_64bit(temp1[0]);
  temp1[1] = dct_const_round_shift_64bit(temp1[1]);
  temp2[0] = dct_const_round_shift_64bit(temp2[0]);
  temp2[1] = dct_const_round_shift_64bit(temp2[1]);
  *out0 = pack_4(temp1[0], temp1[1]);
  *out1 = pack_4(temp2[0], temp2[1]);
}

static INLINE void highbd_multiplication_sse4_1(const __m128i in, const int c0,
                                                const int c1,
                                                __m128i *const out0,
                                                __m128i *const out1) {
  __m128i temp[2];

  extend_64bit(in, temp);
  *out0 = multiplication_round_shift_sse4_1(temp, c0);
  *out1 = multiplication_round_shift_sse4_1(temp, c1);
}

#endif  // VPX_DSP_X86_HIGHBD_INV_TXFM_SSE4_H_