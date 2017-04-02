/*
 * aecm_core.c
 *
 *  Created on: 2015¦~11¤ë12¤é
 *      Author: ite01527
 */
#include "type_def.h"
#include "basic_op.h"
#include "aecm_core.h"
#include "dft_filt_bank.h"
#include "hd_aec.h"
#include "howling_ctrl.h"
#include "rfft_256.h"

void FreqWrapping(Word16 * __restrict Sin, Word16 * __restrict Rin, Word16 *out,
		void * __restrict Inst) {
	static Word16 BufferNear[2][32] = { { 0 }, { 0 } };
	static Word16 BufferFar[2][32] = { { 0 }, { 0 } };
	static Word16 ALIGN4_BEGIN MemoryNearLow[159] ALIGN4_END = { 0 };
	static Word16 ALIGN4_BEGIN MemoryNearHigh[159] ALIGN4_END = { 0 };
	static Word16 ALIGN4_BEGIN MemoryFarLow[159] ALIGN4_END = { 0 };
	static Word16 ALIGN4_BEGIN MemoryFarHigh[159] ALIGN4_END = { 0 };

	Word16 Near_Low[128];
	Word16 Near_High[128];
	Word16 Far_Low[128];
	Word16 Far_High[128];
	Word16 Out_Low[128];
	Word16 Out_High[128];
	Word32 avgHnl32;
	Word32 i;

	BandSplit(Sin, Near_Low, Near_High, BufferNear, MemoryNearLow,
			MemoryNearHigh);
	BandSplit(Rin, Far_Low, Far_High, BufferFar, MemoryFarLow, MemoryFarHigh);
	avgHnl32 = aecm_core(Near_Low, Far_Low, Out_Low, Inst);

	for (i = 0; i < 128; i += 1) {
		Out_High[i] = L_mult(Near_High[i], avgHnl32) >> 16;
	}
	BandSynthesis(Out_Low, Out_High, out);
	//fwrite(Near_High, sizeof(Word16), 128, dptr_iv);
}

Word32 aecm_core(Word16 * const __restrict Sin, Word16 * const __restrict Rin,
		Word16 * const out, void * __restrict st) {
#ifdef PBFDAF_FLAG
	static Word16 dd_buffer[320];
	static Word16 yy_buffer[320];
	static Word16 ee_buffer[256];
#endif
	Word16 en_buffer[RR];
	Word32 avgHnl32;
	Word32 i;

#if defined(DelayEst)
	static Bastiaan_DelayEstimator handle = {0};

	for (i = 0; i < RR / 32; i += 1) {
		DelayEstimator(&Sin[32 * i], &Rin[32 * i], st, &handle);
	}
#endif

#ifdef PBFDAF_FLAG
	PBFDAF(Sin, Rin, en_buffer, (FDAF_STATE*) st);
	PBFDAF_RAES((FDAF_STATE*) st, &nlp_state,
			((FDAF_STATE*) st)->ee_buffer, ((FDAF_STATE*) st)->dd_buffer,
			((FDAF_STATE*) st)->yy_buffer, en_buffer);
	PBFDAF(Sin + 64, Rin + 64, en_buffer + 64, (FDAF_STATE*) st);
	PBFDAF_RAES((FDAF_STATE*) st, &nlp_state, ((FDAF_STATE*) st)->ee_buffer,
			((FDAF_STATE*) st)->dd_buffer, ((FDAF_STATE*) st)->yy_buffer,
			en_buffer + 64);
// TODO : Substitute for UDFT SDAF, but change the OSR to 2
#else
//fwrite(Sin, sizeof(Word16), part_len, dptr_iv);
	avgHnl32 = PBFDSR(Sin, Rin, en_buffer, (FDSR_STATE*) st);
#endif
	memmove(out, en_buffer, sizeof(Word16) * RR);
//howling_ctrl(en_buffer, out, 128);
// TODO : Insert Time Domain Noise Gate ??
	return (avgHnl32);

}

void Aecm_UpdateFarHistory(Bastiaan_DelayEstimator *self,
		ComplexInt16 *far_spectrum, Word16 *qDomain, Word32 *xfa, void *handle,
		void *hndlq, void *hndlx) {
// Get new buffer position
	ComplexInt16 (*ptr)[part_len1] = (ComplexInt16 (*)[part_len1]) handle;
	Word32 (*xPtr)[part_len1] = (Word32 (*)[part_len1]) hndlx;

	self->farc_history_pos++;
	if (self->farc_history_pos >= MAX_DELAY) {
		self->farc_history_pos = 0;
	}

// Update farend spectrum buffer
	((Word16*) hndlq)[self->farc_history_pos] = qDomain[0];
	memcpy(&ptr[self->farc_history_pos][1], &far_spectrum[1],
			sizeof(ComplexInt16) * (part_len - 1));
	ptr[self->farc_history_pos][0].real = far_spectrum[0].real;
	ptr[self->farc_history_pos][0].imag = 0;
	ptr[self->farc_history_pos][part_len].real = far_spectrum[0].imag;
	ptr[self->farc_history_pos][part_len].imag = 0;
	memcpy(xPtr[self->farc_history_pos], xfa, sizeof(Word32) * part_len1);
}

static const Word32 Aecm_AlignedFarend(Bastiaan_DelayEstimator *self,
		Word32 delay) {
	Word32 buffer_position = 0;
	//assert(self != NULL);
	buffer_position = self->farc_history_pos - delay;

// Check buffer position
	if (buffer_position < 0) {
		buffer_position += MAX_DELAY;
	}
// Return farend spectrum
	return (buffer_position);
}

Word32 DelayEstimator(Word16 *Sin, Word16 *Rin, void *handle,
		Bastiaan_DelayEstimator *fptr) {
	static Word16 pdelay = MAX_DELAY - 1;
	static Word16 sin_buffer[nDFT] = { 0 };	//32 samples,4ms
	static Word16 rin_buffer[nDFT] = { 0 };
	static ComplexInt16 far_history[MAX_DELAY][part_len1]; // 200ms
	static Word16 far_qDomain[MAX_DELAY] = { 0 };
	//static Word16 qDomainOld[MAX_DELAY] = { 0 };
	static Word32 pcount[MAX_DELAY][part_len1]; //Q14,filtered significance
	static Word32 kthresholds = 16384L;
	static Word32 xfa_history[MAX_DELAY][part_len1];
	static Word32 Syy[part_len1] = { 0 }; //for nearEnd binary spectrum
	static Word32 Sxx[part_len1] = { 0 }; //for farEnd binary spectrum
	//static Word32 S_xx[MAX_DELAY][part_len1]; //(2*blk_exponent)
	static Word32 kCntr = 0;

	Word16 const kbandstart = 4;
	Word16 const kbandstop = part_len1;
	Word16 const kIter = 4 * M - 4;
	Word32 const kProbabilityOffset = 32768L;
	Word16 blk_expno1;
	Word16 blk_expno2;
	Word16 BufferSSin[nDFT];
	Word16 BufferRRin[nDFT];

	Word32 i, j, k, l;
	Word32 tmp32no1;
	Word32 tmp32no2;
	Word32 candidate_delay;
	Word32 valley_depth;
	Word32 value_best_candidate;
	Word32 value_worst_candidate;
	Word32 dfa[part_len1];
	Word32 xfa[part_len1];
	Word32 Bcount[MAX_DELAY] = { 0 };
	Word32 *yPtr;
	Word32 *xPtr;
	//Word32 qDomain;
	//Word32 tmp32no3;
	//Word32 zeros32 = 0;
	//Word64 tmp64no1;

	memmove(&sin_buffer[0], &sin_buffer[32], sizeof(Word16) * 224);
	memmove(&sin_buffer[224], Sin, sizeof(Word16) * 32);
	memmove(&rin_buffer[0], &rin_buffer[32], sizeof(Word16) * 224);
	memmove(&rin_buffer[224], Rin, sizeof(Word16) * 32);

	for (i = 0; i < nDFT; i += 1) {
		BufferSSin[i] = L_add(L_mult(sin_buffer[i], sqrt_hann_256[i]), 0x8000)
				>> 16;
		BufferRRin[i] = L_add(L_mult(rin_buffer[i], sqrt_hann_256[i]), 0x8000)
				>> 16;
	}
	blk_expno1 = rfft_256((ComplexInt16*) BufferSSin, 256); //nearEnd
	blk_expno2 = rfft_256((ComplexInt16*) BufferRRin, 256); //farEnd

#if defined(DEBUG)
	tmp32no1 = BufferSSin[0];
	tmp32no1 <<= blk_expno1;
	//fwrite(&tmp32no1, sizeof(Word32), 1, dptr_ii);
	//fwrite(&zeros32, sizeof(Word32), 1, dptr_ii);
	tmp32no2 = BufferRRin[0];
	tmp32no2 <<= blk_expno2;
	//fwrite(&tmp32no2, sizeof(Word32), 1, dptr_iii);
	//fwrite(&zeros32, sizeof(Word32), 1, dptr_iii);

	for (i = 1; i < part_len; i += 1) {
		tmp32no1 = ((ComplexInt16*) BufferSSin)[i].real;
		tmp32no1 <<= blk_expno1;
		//fwrite(&tmp32no1, sizeof(Word32), 1, dptr_ii);
		tmp32no1 = ((ComplexInt16*) BufferSSin)[i].imag;
		tmp32no1 <<= blk_expno1;
		//fwrite(&tmp32no1, sizeof(Word32), 1, dptr_ii);
		tmp32no2 = ((ComplexInt16*) BufferRRin)[i].real;
		tmp32no2 <<= blk_expno2;
		//fwrite(&tmp32no2, sizeof(Word32), 1, dptr_iii);
		tmp32no2 = ((ComplexInt16*) BufferRRin)[i].imag;
		tmp32no2 <<= blk_expno2;
		//fwrite(&tmp32no2, sizeof(Word32), 1, dptr_iii);
	}
	tmp32no1 = BufferSSin[1];
	tmp32no1 <<= blk_expno1;
	//fwrite(&tmp32no1, sizeof(Word32), 1, dptr_ii);
	//fwrite(&zeros32, sizeof(Word32), 1, dptr_ii);
	tmp32no2 = BufferRRin[1];
	tmp32no2 <<= blk_expno2;
	//fwrite(&tmp32no2, sizeof(Word32), 1, dptr_iii);
	//fwrite(&zeros32, sizeof(Word32), 1, dptr_iii);
#endif
	//memmove(&qDomainOld[0], &qDomainOld[1], sizeof(Word16) * (MAX_DELAY - 1));

	/*for (i = 1; i < MAX_DELAY; i += 1) {
	 memmove(S_xx[i - 1], S_xx[i], sizeof(Word32) * part_len1);
	 }*/

	/*tmp32no2 = qDomainOld[MAX_DELAY - 1]; //-blk_exp[n-1]

	 for (i = 1; i < part_len; i += 1) {
	 Word32 qDomainDiff;

	 tmp32no1 = L_mult(((ComplexInt16*) BufferRRin)[i].real,
	 ((ComplexInt16*) BufferRRin)[i].real);
	 tmp32no1 = L_mac(tmp32no1, ((ComplexInt16*) BufferRRin)[i].imag,
	 ((ComplexInt16*) BufferRRin)[i].imag); //qDomain=-2*blk_exp
	 if (tmp32no2 < blk_expno2) { //(S_xx[n-1]*2^(-2*tmp32no2)),(tmp32no1*2^(-2*blk_expno2))
	 qDomainDiff = blk_expno2 - tmp32no2; //a*S_xx[n-1]+(1-a)*|X(n,k)|^2
	 tmp32no3 = S_xx[MAX_DELAY - 1][i] >> (2 * qDomainDiff);
	 tmp32no1 = (tmp32no3 - tmp32no1) >> 8; // qDomain=min(q(S_xx[n-1]),q(|X(n,k)|^2))
	 tmp32no3 -= tmp32no1;
	 qDomain = blk_expno2;
	 } else {
	 qDomainDiff = tmp32no2 - blk_expno2;
	 tmp32no1 >>= (2 * qDomainDiff);
	 tmp32no1 = ((tmp32no3 = S_xx[MAX_DELAY - 1][i]) - tmp32no1) >> 8;
	 tmp32no3 -= tmp32no1;
	 qDomain = tmp32no2;
	 }
	 S_xx[MAX_DELAY - 1][i] = tmp32no3;
	 }
	 qDomainOld[MAX_DELAY - 1] = qDomain;*/

	yPtr = (Word32*) &BufferSSin[2 * kbandstart];
	xPtr = (Word32*) &BufferRRin[2 * kbandstart];

	for (i = kbandstart; i < part_len; i += 4) {
		tmp32no1 = L_mult(((Word16*) yPtr)[0], ((Word16*) yPtr)[0]);
		tmp32no1 = L_mac(tmp32no1, ((Word16*) yPtr)[1], ((Word16*) yPtr)[1]);
		tmp32no1 = usqrt(tmp32no1 >> 1);
		dfa[i] = tmp32no1 <<= blk_expno1; // |Y(n,k)|
		tmp32no2 = Syy[i] - tmp32no1; //ythresholds
		Syy[i] -= tmp32no2 >> 6;
		tmp32no1 = L_mult(((Word16*) xPtr)[0], ((Word16*) xPtr)[0]);
		tmp32no1 = L_mac(tmp32no1, ((Word16*) xPtr)[1], ((Word16*) xPtr)[1]);

		tmp32no1 = usqrt(tmp32no1 >> 1); //xfa_history=&xfa[0],&xfa[mD-1]
		/*((Word32*) (xfa_history + MAX_DELAY - 1)[0])[i] = tmp32no1 <<=
		 blk_expno2;*/
		xfa[i] = tmp32no1 <<= blk_expno2;
		tmp32no2 = Sxx[i] - tmp32no1;
		Sxx[i] -= tmp32no2 >> 6;
		yPtr += 4;
		xPtr += 4;
	}

	tmp32no1 = ABS(BufferSSin[0]); //dc
	dfa[0] = tmp32no1 <<= blk_expno1;
	tmp32no2 = Syy[0] - tmp32no1;
	Syy[0] -= tmp32no2 >> 6;
	tmp32no1 = ABS(BufferSSin[1]); //nyquist
	dfa[part_len] = tmp32no1 <<= blk_expno1;
	tmp32no2 = Syy[part_len] - tmp32no1;
	Syy[part_len] -= tmp32no2 >> 6;

	tmp32no1 = ABS(BufferRRin[0]); //dc
	//(xfa_history + MAX_DELAY - 1)[0][0] = tmp32no1 <<= blk_expno2;
	xfa[0] = tmp32no1 <<= blk_expno2;
	tmp32no2 = Sxx[0] - tmp32no1;
	Sxx[0] -= tmp32no2 >> 6;
	tmp32no1 = ABS(BufferRRin[1]); //nyquist
	//(xfa_history + MAX_DELAY - 1)[0][part_len] = tmp32no1 <<= blk_expno2;
	xfa[part_len] = tmp32no1 <<= blk_expno2;
	tmp32no2 = Sxx[part_len] - tmp32no1;
	Sxx[part_len] -= tmp32no2 >> 6;

	//fwrite(&Sxx[1], sizeof(Word32), 127, dptr_iv);
	//fwrite(&Syy[0], sizeof(Word32), part_len1, dptr_iv);

	Aecm_UpdateFarHistory(fptr, (ComplexInt16*) BufferRRin, &blk_expno2, xfa,
			far_history, far_qDomain, xfa_history);

	for (i = 0; i < MAX_DELAY; i += 1) {
		Word32 *ptr = (Word32*) pcount[i];
		Word32 *dptr;
		//Word32 *dptr = (Word32*) xfa_history[i];
		tmp32no1 = Aecm_AlignedFarend(fptr, MAX_DELAY - 1 - i);
		dptr = xfa_history[tmp32no1];

		for (j = kbandstart; j < kbandstop; j += 4) {
			tmp32no1 = (dptr[j] >= Sxx[j]); //filtered significance
			if (tmp32no1) {
				tmp32no1 ^= (dfa[j] >= Syy[j]);
				tmp32no2 = ptr[j] - (tmp32no1 << 14);
				ptr[j] -= tmp32no2 >> 6;
			}
		}
	}
	//fwrite(&pcount[MAX_DELAY - 1][1], sizeof(Word32), part_len, dptr_iv);

	for (i = 0; i < MAX_DELAY; i += 1) {
		Word32 *ptr = (Word32*) pcount[i];
		tmp32no1 = 0;
		for (j = kbandstart; j < kbandstop; j += 4) {
			tmp32no1 += ptr[j];
		}
		Bcount[i] = tmp32no1;
	}
	//fwrite(Bcount, sizeof(Word32), MAX_DELAY, dptr_iv);

	value_best_candidate = 32 << 14;
	value_worst_candidate = 0;
	candidate_delay = MAX_DELAY - 1; //index
	for (i = MAX_DELAY - 1; i >= kIter; i -= 1) {
		if (value_best_candidate > Bcount[i]) {
			value_best_candidate = Bcount[i];
			candidate_delay = i;
		}
		if (value_worst_candidate < Bcount[i]) {
			value_worst_candidate = Bcount[i];
		}
	}

	valley_depth = value_worst_candidate - value_best_candidate;

	if (value_best_candidate <= kthresholds
			&& valley_depth > kProbabilityOffset) {
		pdelay = candidate_delay;
		kthresholds = max(value_best_candidate, 16384);
	} else {
		kthresholds += kthresholds >> 6;
		kthresholds = min(kthresholds, 131072L);
	}

	if ((kCntr++ & (RR / 32 - 1)) == (RR / 32 - 1)) {
		register const ComplexInt16 *cPtr;
		i = pdelay; //0
		j = pdelay - 4 * M + 4; //pdelay-4*M+4>=0,pdelay>=4*M-4=4*6-4=20
		k = 0; //20,16,12,8,4,0
		do {
			ComplexInt32 *ptr = ((FDSR_STATE*) handle)->XFm[M - 1 - k];
			tmp32no1 = Aecm_AlignedFarend(fptr, MAX_DELAY - 1 - i);
			cPtr = far_history[tmp32no1];
			tmp32no2 = far_qDomain[tmp32no1];
			for (l = 0; l < part_len1; l += 1) {
				ptr[l].real = (Word32) cPtr[l].real << tmp32no2;
				ptr[l].imag = (Word32) cPtr[l].imag << tmp32no2;
			}
			k += 1;
			//fwrite(ptr, sizeof(ComplexInt32), part_len1, dptr_iv);
		} while ((i -= 4) >= j);

		for (i = 1; i < part_len; i += 1) {
			((FDSR_STATE*) handle)->Yk[i].real =
					((ComplexInt16*) BufferSSin)[i].real << blk_expno1;
			((FDSR_STATE*) handle)->Yk[i].imag =
					((ComplexInt16*) BufferSSin)[i].imag << blk_expno1;
		}
		((FDSR_STATE*) handle)->Yk[0].real = BufferSSin[0] << blk_expno1;
		((FDSR_STATE*) handle)->Yk[0].imag = BufferSSin[1] << blk_expno1;

	}
	return (pdelay);
}
