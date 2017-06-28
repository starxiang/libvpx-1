/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <smmintrin.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/x86/highbd_inv_txfm_sse2.h"
#include "vpx_dsp/x86/highbd_inv_txfm_sse4.h"
#include "vpx_dsp/x86/inv_txfm_sse2.h"
#include "vpx_dsp/x86/transpose_sse2.h"

static INLINE void highbd_idct4(__m128i *const io) {
  const __m128i cospi_p16_p16 =
      _mm_setr_epi32(cospi_16_64 << 2, 0, cospi_16_64 << 2, 0);
  const __m128i cospi_p08_p08 =
      _mm_setr_epi32(cospi_8_64 << 2, 0, cospi_8_64 << 2, 0);
  const __m128i cospi_p24_p24 =
      _mm_setr_epi32(cospi_24_64 << 2, 0, cospi_24_64 << 2, 0);
  __m128i temp1[4], step[4];

  transpose_32bit_4x4(io, io);

  // stage 1
  temp1[0] = _mm_add_epi32(io[0], io[2]);  // input[0] + input[2]
  extend_64bit(temp1[0], temp1);
  step[0] = multiplication_round_shift(temp1, cospi_p16_p16);
  temp1[0] = _mm_sub_epi32(io[0], io[2]);  // input[0] - input[2]
  extend_64bit(temp1[0], temp1);
  step[1] = multiplication_round_shift(temp1, cospi_p16_p16);
  multiplication_and_add_2_ssse4_1(&io[1], &io[3], &cospi_p24_p24,
                                   &cospi_p08_p08, &step[2], &step[3]);

  // stage 2
  io[0] = _mm_add_epi32(step[0], step[3]);  // step[0] + step[3]
  io[1] = _mm_add_epi32(step[1], step[2]);  // step[1] + step[2]
  io[2] = _mm_sub_epi32(step[1], step[2]);  // step[1] - step[2]
  io[3] = _mm_sub_epi32(step[0], step[3]);  // step[0] - step[3]
}

void vpx_highbd_idct4x4_16_add_sse4_1(const tran_low_t *input, uint16_t *dest,
                                      int stride, int bd) {
  __m128i io[4];

  io[0] = _mm_load_si128((const __m128i *)(input + 0));
  io[1] = _mm_load_si128((const __m128i *)(input + 4));
  io[2] = _mm_load_si128((const __m128i *)(input + 8));
  io[3] = _mm_load_si128((const __m128i *)(input + 12));

  if (bd == 8) {
    __m128i io_short[2];

    io_short[0] = _mm_packs_epi32(io[0], io[1]);
    io_short[1] = _mm_packs_epi32(io[2], io[3]);
    idct4_sse2(io_short);
    idct4_sse2(io_short);
    io_short[0] = _mm_add_epi16(io_short[0], _mm_set1_epi16(8));
    io_short[1] = _mm_add_epi16(io_short[1], _mm_set1_epi16(8));
    io[0] = _mm_srai_epi16(io_short[0], 4);
    io[1] = _mm_srai_epi16(io_short[1], 4);
  } else {
    highbd_idct4(io);
    highbd_idct4(io);
    io[0] = wraplow_16bit(io[0], io[1], _mm_set1_epi32(8));
    io[1] = wraplow_16bit(io[2], io[3], _mm_set1_epi32(8));
  }

  recon_and_store_4(io, dest, stride, bd);
}
