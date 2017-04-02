/*
 * aecm_core.h
 *
 *  Created on: 2015¦~11¤ë12¤é
 *      Author: ite01527
 */

#ifndef AECM_CORE_H_
#define AECM_CORE_H_
//#define PBFDAF_FLAG
#define MAX_DELAY 32

typedef struct {
	Word32 farc_history_pos;
	Word32 farr_history_pos;
} Bastiaan_DelayEstimator;

Word32 aecm_core(Word16 * const __restrict Sin, Word16 * const __restrict Rin,
		Word16 * const out, void * __restrict st);
Word32 DelayEstimator(Word16 *Sin, Word16 *Rin, void *Inst,
		Bastiaan_DelayEstimator *ptr);
void FreqWrapping(Word16 * __restrict Sin, Word16 * __restrict Rin, Word16 *out,
		void * __restrict Inst);

#endif /* AECM_CORE_H_ */
