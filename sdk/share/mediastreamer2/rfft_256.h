/*
 * rfft_256.h
 *
 *  Created on: 2015¦~7¤ë20¤é
 *      Author: ite01527
 */

#ifndef RFFT_256_H_
#define RFFT_256_H_

#ifndef FIXED_POINT
#define FIXED_POINT
#endif
#define NFFT 256
#define U_Size 4
#define V_Size 32
#define end_startup 64
#define PartLen (NFFT>>1)
#define PartLen1 (PartLen+1)

extern Word32 CB_FREQ_INDICES_256[];
extern Word32 CB_OFST[];
extern const Word32 sprd_table[][18];
extern Word32 T_ABS[];

enum {
	Wiener_Mode = 0, Auditory_Mask_Mode, Wind_Mode
};

typedef struct {
	Word16 buffer[PartLen]; // prev
	Word16 sc_buffer[NFFT]; // single channel
	Word16 mOnSet[PartLen1];
	Word16 mEdge[PartLen1];
	Word16 mInterFere[PartLen1];

	Word32 reset;
	Word32 cntr;
	Word32 mode;
	Word32 blockInd; // frame idx
	Word32 beta;
	Word32 gamma_prev[PartLen1];
	Word32 xi[PartLen1];
	Word32 Gh1[PartLen1];
	Word32 bufferGamma[3][PartLen1];

	Word64 S[PartLen1];
	Word64 S_tild[PartLen1];
	Word64 Smin[PartLen1];
	Word64 Smin_sw[PartLen1];
	Word64 Smin_tild[PartLen1];
	Word64 Smin_sw_tild[PartLen1];
	Word64 lambda[PartLen1];
	Word64 Sxx[PartLen1];
	Word64 Nxx[PartLen1];

} nr_state;

typedef Word32 (*func_ptr)(Complex16_t *, Complex16_t *, Word32);

extern nr_state NR_STATE_TX;
extern nr_state NR_STATE_RX;
extern Word16 twiddle_fact256[512];
extern Word16 twiddle_fact512[512];
extern Word16 sqrt_hann_win[256];

Word32 cfft_128(Complex16_t *Sin, Complex16_t *twd, Word32 N);
Word32 cfft_128_temp(Complex16_t *Sin, Complex16_t *twd, Word32 N);
Word32 cFFT_256(ComplexInt16 *Sin, ComplexInt16 *Twd, Word32 N);
Word32 rfft_256(Complex16_t *Sin, Word32 N);
Word32 rfft_512(ComplexInt16 *Sin, Word32 N);
void hs_ifft_256(Complex16_t *p, Word16 *out, Word32 N);
void hs_ifft_512(Complex16_t *p, Word16 *out, Word32 N);
void init_nr_core(nr_state *st, Word32 mode);
void noise_suppression(nr_state *Inst, ComplexInt16 *Sin, Word32 blk_exponent);
void fifo_ctrl(Word16 *Sin, Word16 *Rin, Word16 *Sout, void *Inst);
void mask_thresh(Word64 *ps, Word64 * const mask, Word32 N, Word32 *idx);
void Overlap_Add(Word16 *Sin, Word16 *Sout, nr_state *NRI);

#endif /* RFFT_256_H_ */
