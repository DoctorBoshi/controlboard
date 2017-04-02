/*
 * hd_aec.c
 *
 *  Created on: 2015¦~9¤ë15¤é
 *      Author: ite01527
 */

#include "type_def.h"
#include "basic_op.h"
#include "dft_filt_bank.h"
#include "hd_aec.h"
#include "howling_ctrl.h"
#include "rfft_256.h"
#include "signal_processing_library.h"

#define OTIME
//#undef FIXED_POINT

AecmCore aecm;
FDAF_STATE aec_state; //for pbfdaf
FDSR_STATE anc_state; //for low bands
FDSR_STATE agc_state; //for high bands
NLP_STATE nlp_state;

static Word64 alphaS = 64;
static Word64 alpha_s = 24576;
static Word64 alpha_d = 1311;

/*C16_t*/Word16 ALIGN4_BEGIN TWD_64[nFFT] ALIGN4_END = { 32767, 0, 32609, -3212,
		32137, -6393, 31356, -9512, 30272, -12540, 28897, -15447, 27244, -18205,
		25329, -20788, 23169, -23170, 20787, -25330, 18204, -27245, 15446,
		-28898, 12539, -30273, 9511, -31357, 6392, -32138, 3211, -32610, 0,
		-32767, -3212, -32610, -6393, -32138, -9512, -31357, -12540, -30273,
		-15447, -28898, -18205, -27245, -20788, -25330, -23170, -23170, -25330,
		-20788, -27245, -18205, -28898, -15447, -30273, -12540, -31357, -9512,
		-32138, -6393, -32610, -3212, -32767, -1, -32610, 3211, -32138, 6392,
		-31357, 9511, -30273, 12539, -28898, 15446, -27245, 18204, -25330,
		20787, -23170, 23169, -20788, 25329, -18205, 27244, -15447, 28897,
		-12540, 30272, -9512, 31356, -6393, 32137, -3212, 32609, -1, 32767,
		3211, 32609, 6392, 32137, 9511, 31356, 12539, 30272, 15446, 28897,
		18204, 27244, 20787, 25329, 23169, 23169, 25329, 20787, 27244, 18204,
		28897, 15446, 30272, 12539, 31356, 9511, 32137, 6392, 32609, 3211 };

/*C16_t*/Word16 ALIGN4_BEGIN TWD_128[nFFT] ALIGN4_END = { 32767, 0, 32727,
		-1608, 32609, -3212, 32412, -4808, 32137, -6393, 31785, -7962, 31356,
		-9512, 30851, -11039, 30272, -12540, 29621, -14010, 28897, -15447,
		28105, -16846, 27244, -18205, 26318, -19520, 25329, -20788, 24278,
		-22005, 23169, -23170, 22004, -24279, 20787, -25330, 19519, -26319,
		18204, -27245, 16845, -28106, 15446, -28898, 14009, -29622, 12539,
		-30273, 11038, -30852, 9511, -31357, 7961, -31786, 6392, -32138, 4807,
		-32413, 3211, -32610, 1607, -32728, 0, -32767, -1608, -32728, -3212,
		-32610, -4808, -32413, -6393, -32138, -7962, -31786, -9512, -31357,
		-11039, -30852, -12540, -30273, -14010, -29622, -15447, -28898, -16846,
		-28106, -18205, -27245, -19520, -26319, -20788, -25330, -22005, -24279,
		-23170, -23170, -24279, -22005, -25330, -20788, -26319, -19520, -27245,
		-18205, -28106, -16846, -28898, -15447, -29622, -14010, -30273, -12540,
		-30852, -11039, -31357, -9512, -31786, -7962, -32138, -6393, -32413,
		-4808, -32610, -3212, -32728, -1608 };

static Word16 ALIGN4_BEGIN bit_rev_tab[(nFFT >> 1)] ALIGN4_END = { 0, 32, 16,
		48, 8, 40, 24, 56, 4, 36, 20, 52, 12, 44, 28, 60, 2, 34, 18, 50, 10, 42,
		26, 58, 6, 38, 22, 54, 14, 46, 30, 62, 1, 33, 17, 49, 9, 41, 25, 57, 5,
		37, 21, 53, 13, 45, 29, 61, 3, 35, 19, 51, 11, 43, 27, 59, 7, 39, 23,
		55, 15, 47, 31, 63 };

static Word16 ALIGN4_BEGIN sqrt_hann_128[nFFT] ALIGN4_END = { 0, 804, 1607,
		2410, 3211, 4011, 4807, 5601, 6392, 7179, 7961, 8739, 9511, 10278,
		11038, 11792, 12539, 13278, 14009, 14732, 15446, 16150, 16845, 17530,
		18204, 18867, 19519, 20159, 20787, 21402, 22004, 22594, 23169, 23731,
		24278, 24811, 25329, 25831, 26318, 26789, 27244, 27683, 28105, 28510,
		28897, 29268, 29621, 29955, 30272, 30571, 30851, 31113, 31356, 31580,
		31785, 31970, 32137, 32284, 32412, 32520, 32609, 32678, 32727, 32757,
		32767, 32757, 32727, 32678, 32609, 32520, 32412, 32284, 32137, 31970,
		31785, 31580, 31356, 31113, 30851, 30571, 30272, 29955, 29621, 29268,
		28897, 28510, 28105, 27683, 27244, 26789, 26318, 25831, 25329, 24811,
		24278, 23731, 23169, 22594, 22004, 21402, 20787, 20159, 19519, 18867,
		18204, 17530, 16845, 16150, 15446, 14732, 14009, 13278, 12539, 11792,
		11038, 10278, 9511, 8739, 7961, 7179, 6392, 5601, 4807, 4011, 3211,
		2410, 1607, 804 };

Word16 ALIGN4_BEGIN sqrt_hann_256[nDFT] ALIGN4_END = { 0, 402, 804, 1206, 1607,
		2009, 2410, 2811, 3211, 3611, 4011, 4409, 4807, 5205, 5601, 5997, 6392,
		6786, 7179, 7571, 7961, 8351, 8739, 9126, 9511, 9895, 10278, 10659,
		11038, 11416, 11792, 12166, 12539, 12909, 13278, 13645, 14009, 14372,
		14732, 15090, 15446, 15799, 16150, 16499, 16845, 17189, 17530, 17868,
		18204, 18537, 18867, 19194, 19519, 19840, 20159, 20474, 20787, 21096,
		21402, 21705, 22004, 22301, 22594, 22883, 23169, 23452, 23731, 24006,
		24278, 24546, 24811, 25072, 25329, 25582, 25831, 26077, 26318, 26556,
		26789, 27019, 27244, 27466, 27683, 27896, 28105, 28309, 28510, 28706,
		28897, 29085, 29268, 29446, 29621, 29790, 29955, 30116, 30272, 30424,
		30571, 30713, 30851, 30984, 31113, 31236, 31356, 31470, 31580, 31684,
		31785, 31880, 31970, 32056, 32137, 32213, 32284, 32350, 32412, 32468,
		32520, 32567, 32609, 32646, 32678, 32705, 32727, 32744, 32757, 32764,
		32767, 32764, 32757, 32744, 32727, 32705, 32678, 32646, 32609, 32567,
		32520, 32468, 32412, 32350, 32284, 32213, 32137, 32056, 31970, 31880,
		31785, 31684, 31580, 31470, 31356, 31236, 31113, 30984, 30851, 30713,
		30571, 30424, 30272, 30116, 29955, 29790, 29621, 29446, 29268, 29085,
		28897, 28706, 28510, 28309, 28105, 27896, 27683, 27466, 27244, 27019,
		26789, 26556, 26318, 26077, 25831, 25582, 25329, 25072, 24811, 24546,
		24278, 24006, 23731, 23452, 23169, 22883, 22594, 22301, 22004, 21705,
		21402, 21096, 20787, 20474, 20159, 19840, 19519, 19194, 18867, 18537,
		18204, 17868, 17530, 17189, 16845, 16499, 16150, 15799, 15446, 15090,
		14732, 14372, 14009, 13645, 13278, 12909, 12539, 12166, 11792, 11416,
		11038, 10659, 10278, 9895, 9511, 9126, 8739, 8351, 7961, 7571, 7179,
		6786, 6392, 5997, 5601, 5205, 4807, 4409, 4011, 3611, 3211, 2811, 2410,
		2009, 1607, 1206, 804, 402 };

static Word16 ALIGN4_BEGIN sqrt_hann_512[PART_LEN1] ALIGN4_END = { 0, 201, 402,
		604, 805, 1007, 1208, 1409, 1610, 1812, 2013, 2214, 2415, 2616, 2816,
		3017, 3217, 3418, 3618, 3818, 4018, 4218, 4418, 4617, 4817, 5016, 5215,
		5414, 5612, 5811, 6009, 6207, 6404, 6602, 6799, 6996, 7193, 7389, 7585,
		7781, 7976, 8172, 8367, 8561, 8756, 8950, 9143, 9336, 9529, 9722, 9914,
		10106, 10297, 10488, 10679, 10869, 11059, 11249, 11438, 11626, 11814,
		12002, 12189, 12376, 12562, 12748, 12933, 13118, 13302, 13486, 13670,
		13853, 14035, 14217, 14398, 14578, 14759, 14938, 15117, 15296, 15474,
		15651, 15827, 16004, 16179, 16354, 16528, 16702, 16875, 17047, 17219,
		17390, 17560, 17730, 17899, 18068, 18235, 18402, 18569, 18734, 18899,
		19063, 19227, 19390, 19552, 19713, 19873, 20033, 20192, 20351, 20508,
		20665, 20821, 20976, 21130, 21284, 21437, 21589, 21740, 21890, 22039,
		22188, 22336, 22483, 22629, 22774, 22919, 23062, 23205, 23347, 23488,
		23628, 23767, 23905, 24042, 24179, 24314, 24449, 24582, 24715, 24847,
		24978, 25108, 25237, 25365, 25492, 25618, 25743, 25867, 25990, 26112,
		26234, 26354, 26473, 26591, 26708, 26825, 26940, 27054, 27167, 27279,
		27390, 27500, 27609, 27717, 27824, 27930, 28035, 28139, 28241, 28343,
		28443, 28543, 28641, 28739, 28835, 28930, 29024, 29117, 29209, 29300,
		29389, 29478, 29565, 29651, 29737, 29821, 29904, 29985, 30066, 30145,
		30224, 30301, 30377, 30452, 30526, 30599, 30670, 30740, 30810, 30877,
		30944, 31010, 31074, 31138, 31200, 31261, 31321, 31379, 31437, 31493,
		31548, 31602, 31654, 31706, 31756, 31805, 31853, 31900, 31945, 31989,
		32032, 32074, 32115, 32154, 32192, 32229, 32265, 32299, 32333, 32365,
		32395, 32425, 32454, 32481, 32507, 32531, 32555, 32577, 32598, 32618,
		32636, 32654, 32670, 32685, 32698, 32711, 32722, 32732, 32740, 32748,
		32754, 32759, 32763, 32765, 32766, 32766 };

static Word32 CB_FREQ_INDICES[(nFFT >> 1) + 1] = { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5,
		5, 6, 6, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12,
		12, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15,
		15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17 };

static Word32 ToBARKScale[PART_LEN1] = { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3,
		4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9,
		9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
		12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17,
		17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18,
		18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
		18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
		20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
		20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21,
		21, 21, 21 };

static Word16 ALIGN4_BEGIN HBCoeff[32] ALIGN4_END = { -28, 35, -52, 76, -113,
		160, -224, 303, -407, 540, -718, 964, -1335, 1972, -3408, 10407, 10407,
		-3408, 1972, -1335, 964, -718, 540, -407, 303, -224, 160, -113, 76, -52,
		35, -28 };

Word32 cfft_64(Complex16_t *Sin, Complex16_t *TWD) {
	Word32 R = 1;
	Word32 S = 64;
	Word32 stage = 0;
	Word32 group = 0;
	Word32 s;
	Word32 blk_exponent = 0;
	Word32 sb;
	Complex16_t *a, *b, *c, *d;

	sb = -4;
	s = 0;
	do {
		sb = exp_adj(Sin[s].real, sb);
		sb = exp_adj(Sin[s].imag, sb);
	} while (++s < 64);

	s = 0;
	do {
		switch (sb) {
		case 0:
			Sin[s].real = L_add(Sin[s].real * 2048, 0x4000) >> 15;
			Sin[s].imag = L_add(Sin[s].imag * 2048, 0x4000) >> 15;
			break;
		case -1:
			Sin[s].real = L_add(Sin[s].real * 4096, 0x4000) >> 15;
			Sin[s].imag = L_add(Sin[s].imag * 4096, 0x4000) >> 15;
			break;
		case -2:
			Sin[s].real = L_add(Sin[s].real * 8192, 0x4000) >> 15;
			Sin[s].imag = L_add(Sin[s].imag * 8192, 0x4000) >> 15;
			break;
		case -3:
			Sin[s].real = L_add(Sin[s].real * 16384, 0x4000) >> 15;
			Sin[s].imag = L_add(Sin[s].imag * 16384, 0x4000) >> 15;
			break;
		default:
			break;
		}
	} while (++s < 64);

	blk_exponent += (sb + 4);

	stage = 0;
	do {
		sb = -4;
		group = 0;
		do {
			s = S / 4 - 1;
			a = Sin + group * S;
			a += s;
			b = a + S / 4;
			c = b + S / 4;
			d = c + S / 4;
			do {
				Complex16_t x_tab, y_tab, z_tab; // W(sm, N), m = 0:3, s = 0:S/4-1
				x_tab = TWD[s * R * 2];
				y_tab = TWD[s * R];
				z_tab = TWD[s * R * 3];

				sb = btr_fly_r4(a, b, c, d, sb); // radix 2.^2
				b[0] = complex_mul(b[0], x_tab);
				c[0] = complex_mul(c[0], y_tab);
				d[0] = complex_mul(d[0], z_tab);

				sb = exp_adj(b[0].real, sb); // clz(0)-1
				sb = exp_adj(b[0].imag, sb);
				sb = exp_adj(c[0].real, sb);
				sb = exp_adj(c[0].imag, sb);
				sb = exp_adj(d[0].real, sb);
				sb = exp_adj(d[0].imag, sb);
				a -= 1;
				b -= 1;
				c -= 1;
				d -= 1;

			} while (--s >= 0);

		} while (++group < R);

		if (stage != 2) { // last stage ?
			s = 0;
			do {
				switch (sb) {
				case 0:
					Sin[s].real = L_add(Sin[s].real * 2048, 0x4000) >> 15;
					Sin[s].imag = L_add(Sin[s].imag * 2048, 0x4000) >> 15;
					break;
				case -1:
					Sin[s].real = L_add(Sin[s].real * 4096, 0x4000) >> 15;
					Sin[s].imag = L_add(Sin[s].imag * 4096, 0x4000) >> 15;
					break;
				case -2:
					Sin[s].real = L_add(Sin[s].real * 8192, 0x4000) >> 15;
					Sin[s].imag = L_add(Sin[s].imag * 8192, 0x4000) >> 15;
					break;
				case -3:
					Sin[s].real = L_add(Sin[s].real * 16384, 0x4000) >> 15;
					Sin[s].imag = L_add(Sin[s].imag * 16384, 0x4000) >> 15;
					break;
				}
			} while (++s < 64);
			blk_exponent += (sb + 4);
		}

		R *= 4;
		S /= 4;
	} while (++stage < 3);

	for (s = 0; s < 64; s += 1) {
		register Word32 n;
		n = bit_rev_tab[s];
		if (n > s)
			swap_data(&Sin[s], &Sin[n]);
	}
	return (blk_exponent);
}

Word32 rfft_128(Complex16_t *Sin) {
	Word32 i;
	Word32 blk_exponent;
	Complex16_t S_in[64];

	for (i = 0; i < nFFT / 2; i += 1) {
		S_in[i].real = L_add(16384 * Sin[i].real, 0x4000) >> 15;
		S_in[i].imag = L_add(16384 * Sin[i].imag, 0x4000) >> 15;
	}

	blk_exponent = cfft_64(S_in, (Complex16_t *) TWD_64);
	for (i = 0; i < 64; i += 1) {
		register Word32 L_var_out;
		Complex16_t X_even, X_odd;

		X_even = complex_add(S_in[i], complex_conj(S_in[(64 - i) & 0x3f]));
		X_odd = complex_swap(
				complex_sub(S_in[i], complex_conj(S_in[(64 - i) & 0x3f])));
		if (i) {
			Sin[i] = complex_add(X_even,
					complex_mul(X_odd, ((ComplexInt16*) TWD_128)[i]));
		} else {
			L_var_out = L_sub(X_even.real << 16, X_odd.real << 16);
			Sin[0].imag = L_var_out >> 16; // nyquist
			L_var_out = L_add(X_even.real << 16, X_odd.real << 16);
			Sin[0].real = L_var_out >> 16; // dc
		}
	}

	return (blk_exponent);
}

void hs_ifft_128(Complex16_t *p, Word16 *out) {
	Word32 blk_exponent;
	Word32 i;
	Complex16_t Zk[64];
	i = 63;
	do {
		Complex16_t X_even, X_odd;
		if (i) {
			X_even = complex_add(complex_asr(p[i], 1),
					complex_conj(complex_asr(p[64 - i], 1))); // p[0:1+N/2]
			X_odd = complex_sub(complex_asr(p[i], 1),
					complex_conj(complex_asr(p[64 - i], 1)));
			X_odd = complex_mul(X_odd,
					complex_conj(((ComplexInt16*) TWD_128)[i]));
		} else {
			register Word32 L_var_out;
			L_var_out = L_add(p[0].real << 15, p[0].imag << 15); // -2^31
			X_even.real = L_var_out >> 16;
			X_even.imag = 0;
			L_var_out = L_sub(p[0].real << 15, p[0].imag << 15);
			X_odd.real = L_var_out >> 16;
			X_odd.imag = 0;
		}
		Zk[i] = complex_conj(
				complex_sub(complex_asr(X_even, 1),
						complex_asr(complex_swap(X_odd), 1))); // Xe[k]+j*Xo[k]
	} while (--i >= 0);

	blk_exponent = cfft_64(Zk, (Complex16_t *) TWD_64);
	blk_exponent -= 5;
	i = 63;
	do {
		Zk[i] = complex_asr(complex_conj(Zk[i]), -blk_exponent);
	} while (--i >= 0);

	memmove(out, Zk, sizeof(Word16) * 128);
}

void Overlap_Save(Word16 * const Sin, Word16 *Sout) { // M+L-1, 0:-N+M+L-2
	Word32 i; // -N+M+L-1
	Word32 blk_exponent;
	Word16 buffer_fft[128];
	static Word16 buffer_in[128] = { 0 };

	memmove(&buffer_in[0], &buffer_in[64], sizeof(Word16) * 64);
	memmove(&buffer_in[64], Sin, sizeof(Word16) * 64);

	memcpy(buffer_fft, buffer_in, sizeof(Word16) * 128);

	blk_exponent = rfft_128((Complex16_t *) buffer_fft);
	hs_ifft_128((Complex16_t *) buffer_fft, buffer_fft);
	for (i = 0; i < 64; i += 1) {
		register Word32 L_var_out;
		L_var_out = asr(L_mult(buffer_fft[i], 32767), 16 - blk_exponent);
		Sout[i] = sat_32_16(L_var_out);
	}
}

/*Partition Block Frequency Domain Adaptive Filter ~= FDAF Performance*/
void NLP_Init(NLP_STATE *st) {
	Word32 i;

	st->seed = 0;
	st->timer = 4;
	st->diverge_state = 0;

	for (i = 1; i < nFFT / 2; i += 1) {
		st->Gainno1[i] = 32767;
		st->Gainno3[i] = 0;
		st->gammano1[i] = 1;
		st->Sdd[i] = 1;
		st->Sed[i] = 1;
		st->See[i] = 1;
		st->Syy[i] = 1;
		st->Sym[i] = INT_MAX;
	}
}

void PBFDAF_Init(FDAF_STATE *st) {
	Word32 i, j;
	for (i = 0; i < M; i += 1) {
		j = nFFT / 2;
		do {
			(*(st->WFb[i] + j)).real = 0;
			(*(st->WFb[i] + j)).imag = 0;
		} while (--j >= 0);
	}

	memset(st->pn, 0, sizeof(Word64) * (1 + nFFT / 2));
}

void PBFDSR_Init(FDSR_STATE *st) {
	Word32 i, j;

	st->alpha_m = 655;
	st->nlpFlag = 0;
	st->currentVADvalue = 0;

	memset(st->buffer_olpa, 0, sizeof(Word16) * (nDFT - RR));
	memset(st->buffer_rrin, 0, sizeof(Word16) * nDFT);
	memset(st->buffer_ssin, 0, sizeof(Word16) * nDFT);

	for (i = 0; i < M; i += 1) {
		for (j = 0; j < part_len1; j += 1) {
			st->S_vs[i][2 * j] = 1; // real
			st->S_vs[i][2 * j + 1] = 0; // imag
			st->S_ss[i][j] = 1;
			st->XFm[i][j].real = 0;
			st->XFm[i][j].imag = 0;
			st->qDomainOld[i][j] = 0;
		}
	}

	for (i = 0; i < part_len1; i += 1) {
		st->nearFilt[i] = 1;
		st->echoEst64Gained[i] = 1;
		st->S_nn[i] = 1;
		st->S_ee[i] = 1;
		st->gammano1[i] = 32767; // post-snr
		st->gammano2[i] = 32767;
		st->lambda[i] = 1;
		st->Gainno1[i] = 32767;
		st->Gainno2[i] = 32767;
	}

}

Word32 norm2Word16(void *Sin, Word32 N) { // note dc & nyquists
	Word32 i;
	Word32 tmp32no3;
	Word32 tmp32no4;
	Word32 tmp32no5;
	Word32 tmp32no6;

	tmp32no5 = 32;
	tmp32no6 = 32;
	for (i = 0; i < N / 2; i += 1) { // AGC
		Word32 tmp32no1;
		Word32 tmp32no2;
		Float32 xr;
		Float32 xi;

#if	0
		xr = ((ComplexFloat32 *) Sin)[i].real;
		xi = ((ComplexFloat32 *) Sin)[i].imag;
#else
		xr = ((Complex32_t *) Sin + i)->real;
		xi = ((Complex32_t *) Sin + i)->imag;
#endif

		tmp32no1 = ABS((Word32 )xr);
		tmp32no2 = ABS((Word32 )xi);

		if (i) {

			tmp32no3 = CLZ(tmp32no1);
			tmp32no3 =
					(xr < 0) ?
							(!(tmp32no1 & (tmp32no1 - 1))) ?
									tmp32no3 + 1 : tmp32no3
							: tmp32no3;
			tmp32no4 = CLZ(tmp32no2);
			tmp32no4 =
					(xi < 0) ?
							(!(tmp32no2 & (tmp32no2 - 1))) ?
									tmp32no4 + 1 : tmp32no4
							: tmp32no4;
			tmp32no5 = (tmp32no3 < tmp32no5) ? tmp32no3 : tmp32no5;
			tmp32no6 = (tmp32no4 < tmp32no6) ? tmp32no4 : tmp32no6;

		} else {

			tmp32no3 = CLZ(tmp32no1);
			tmp32no3 =
					(xr < 0) ?
							(!(tmp32no1 & (tmp32no1 - 1))) ?
									tmp32no3 + 1 : tmp32no3
							: tmp32no3;
			tmp32no4 = CLZ(tmp32no2);
			tmp32no4 =
					(xi < 0) ?
							(!(tmp32no2 & (tmp32no2 - 1))) ?
									tmp32no4 + 1 : tmp32no4
							: tmp32no4;
			tmp32no5 = (tmp32no3 < tmp32no5) ? tmp32no3 : tmp32no5;
			tmp32no5 = (tmp32no4 < tmp32no5) ? tmp32no4 : tmp32no5;
		}
	}

	tmp32no5 = min(tmp32no5, tmp32no6); // number of sign bits
	tmp32no6 = (tmp32no5 < 17) ? 17 - tmp32no5 : 0;
	return (tmp32no6);
}

void BandSplit(Word16 *InPut, Word16 *OutL, Word16 *OutH, Word16 (*buffer)[32],
		Word16 *memoryL, Word16 *memoryH) {
	Word16 *ptr;
	Word16 *signal;
	Word16 * __restrict cPtr = HBCoeff + 31;
	Word32 i, j;
	Word32 tmp32no1;
	Word32 tmp32no2;
	Word32 tmp32no3;
	Word32 tmp32no4;

#if 0
	for (i = 0; i < 256; i += 2) {
		memmove(buffer[0], buffer[0] + 1, sizeof(Word16) * 31);
		memmove(buffer[1], buffer[1] + 1, sizeof(Word16) * 31);
		buffer[0][31] = *InPut++;
		buffer[1][31] = *InPut++;
		ptr = buffer[0];
		cPtr = HBCoeff + 31;
		tmp32no1 = 0;
		j = 32;
		do {
			tmp32no1 = L_mac(tmp32no1, *ptr++, *cPtr--);
		}while (--j != 0);
		tmp32no2 = L_mac(tmp32no1, buffer[1][15], 16384);
		tmp32no1 = L_msu(tmp32no1, buffer[1][15], 16384);
		*OutH++ = sat_32_16(tmp32no1 >> 16);
		*OutL++ = sat_32_16(tmp32no2 >> 16);
	}
#else
	Word32 a0, a1, a2, a3;
	Word32 a4, a5, a6, a7;
	Word16 x0, x1, x2, x3;
	Word16 d0, d1, d2, d3;
	Word16 c0, c1, c2, c3;

	memmove(&memoryL[0], &memoryL[128], sizeof(Word16) * 31);
	memmove(&memoryH[0], &memoryH[128], sizeof(Word16) * 31);
	ptr = &memoryL[31];
	for (i = 0; i < 256; i += 2) {
		*ptr++ = InPut[i];
	}
	ptr = &memoryH[31];
	for (i = 1; i < 256; i += 2) {
		*ptr++ = InPut[i];
	}

	ptr = memoryL;
	signal = memoryH;
	i = 128;
	do {
		a0 = 0;
		a1 = 0;
		a2 = 0;
		a3 = 0;
		a4 = 0;
		a5 = 0;
		a6 = 0;
		a7 = 0;
		x0 = *ptr++;
		x1 = *ptr++;
		x2 = *ptr++;
		x3 = *ptr++;
		d0 = *signal++;
		d1 = *signal++;
		d2 = *signal++;
		d3 = *signal++;

		j = 32;
		do {
			c0 = *cPtr--;
			c1 = *cPtr--;
			c2 = *cPtr--;
			c3 = *cPtr--;
			a0 = L_mac(a0, c0, x0);
			a0 = L_mac(a0, c1, x1);
			a0 = L_mac(a0, c2, x2);
			a0 = L_mac(a0, c3, x3);
			a1 = L_mac(a1, c0, x1);
			a1 = L_mac(a1, c1, x2);
			a1 = L_mac(a1, c2, x3);
			a2 = L_mac(a2, c0, x2);
			a2 = L_mac(a2, c1, x3);
			a3 = L_mac(a3, c0, x3);
			a4 = (j == 20) ? L_mult(d3, 16384) : a4;

			x0 = *ptr++;
			x1 = *ptr++;
			x2 = *ptr++;
			x3 = *ptr++;
			d0 = *signal++;
			d1 = *signal++;
			d2 = *signal++;
			d3 = *signal++;

			a1 = L_mac(a1, c3, x0);
			a2 = L_mac(a2, c2, x0);
			a2 = L_mac(a2, c3, x1);
			a3 = L_mac(a3, c1, x0);
			a3 = L_mac(a3, c2, x1);
			a3 = L_mac(a3, c3, x2);
			a5 = (j == 20) ? L_mult(d0, 16384) : a5;
			a6 = (j == 20) ? L_mult(d1, 16384) : a6;
			a7 = (j == 20) ? L_mult(d2, 16384) : a7;

		} while ((j -= 4) != 0);
		ptr -= 32;
		signal -= 32;
		cPtr += 32;

		tmp32no1 = L_add(a0, a4);
		tmp32no2 = L_add(a1, a5);
		tmp32no3 = L_add(a2, a6);
		tmp32no4 = L_add(a3, a7);
		*OutL++ = sat_32_16(tmp32no1 >> 16);
		*OutL++ = sat_32_16(tmp32no2 >> 16);
		*OutL++ = sat_32_16(tmp32no3 >> 16);
		*OutL++ = sat_32_16(tmp32no4 >> 16);
		tmp32no1 = L_sub(a0, a4);
		tmp32no2 = L_sub(a1, a5);
		tmp32no3 = L_sub(a2, a6);
		tmp32no4 = L_sub(a3, a7);
		*OutH++ = sat_32_16(tmp32no1 >> 16);
		*OutH++ = sat_32_16(tmp32no2 >> 16);
		*OutH++ = sat_32_16(tmp32no3 >> 16);
		*OutH++ = sat_32_16(tmp32no4 >> 16);

	} while ((i -= 4) != 0);
#endif
}

void BandSynthesis(Word16 * __restrict InPutL, Word16 * __restrict InPutH,
		Word16 * __restrict Out) {
	Word32 i, j;
	Word32 tmp32no1;
	Word32 tmp32no2;
	Word16 * __restrict ptr = HBCoeff + 31;
	Word16 * __restrict cPtr;
	Word16 * __restrict cPtrI;
	static Word16 BufferLow[159] = { 0 };
	static Word16 BufferHigh[159] = { 0 };
#if 0
	for (i = 0; i < 128; i += 1) {
		memmove(BufferLow, BufferLow + 1, sizeof(Word16) * 31);
		BufferLow[31] = *InPutL++;
		memmove(BufferHigh, BufferHigh + 1, sizeof(Word16) * 31);
		BufferHigh[31] = *InPutH++;
		tmp32no1 = L_mult(BufferLow[15], 16384);
		tmp32no2 = L_mult(BufferHigh[15], 16384);
		tmp32no1 = sat_32_16(L_add(tmp32no1, tmp32no2) >> 15);
		*Out++ = tmp32no1;
		ptr = HBCoeff + 31;
		cPtr = BufferLow;
		cPtrI = BufferHigh;
		tmp32no1 = 0;
		tmp32no2 = 0;
		j = 32;
		do {
			tmp32no2 = L_msu(tmp32no2, *ptr, *cPtrI++);
			tmp32no1 = L_mac(tmp32no1, *ptr--, *cPtr++);
		}while (--j != 0);

		tmp32no1 = sat_32_16(L_add(tmp32no1, tmp32no2) >> 15);
		*Out++ = tmp32no1;
	}
#else
	Word32 a0, a1, a2, a3;
	Word32 a4, a5, a6, a7;
	Word32 e0, e1, e2, e3;
	Word32 f0, f1, f2, f3;
	Word16 c0, c1, c2, c3;
	Word16 x0, x1, x2, x3;
	Word16 d0, d1, d2, d3;

	memmove(&BufferLow[0], &BufferLow[128], sizeof(Word16) * 31);
	memmove(&BufferLow[31], InPutL, sizeof(Word16) * 128);
	memmove(&BufferHigh[0], &BufferHigh[128], sizeof(Word16) * 31);
	memmove(&BufferHigh[31], InPutH, sizeof(Word16) * 128);

	cPtr = BufferLow;
	cPtrI = BufferHigh;
	i = 128;
	do {
		a0 = 0;
		a1 = 0;
		a2 = 0;
		a3 = 0;
		a4 = 0;
		a5 = 0;
		a6 = 0;
		a7 = 0;
		e0 = 0;
		e1 = 0;
		e2 = 0;
		e3 = 0;
		f0 = 0;
		f1 = 0;
		f2 = 0;
		f3 = 0;
		x0 = *cPtr++;
		x1 = *cPtr++;
		x2 = *cPtr++;
		x3 = *cPtr++;
		d0 = *cPtrI++;
		d1 = *cPtrI++;
		d2 = *cPtrI++;
		d3 = *cPtrI++;
		j = 32;
		do {
			c0 = *ptr--;
			c1 = *ptr--;
			c2 = *ptr--;
			c3 = *ptr--;

			a0 = L_mac(a0, x0, c0);
			a0 = L_mac(a0, x1, c1);
			a0 = L_mac(a0, x2, c2);
			a0 = L_mac(a0, x3, c3);
			a1 = L_mac(a1, x1, c0);
			a1 = L_mac(a1, x2, c1);
			a1 = L_mac(a1, x3, c2);
			a2 = L_mac(a2, x2, c0);
			a2 = L_mac(a2, x3, c1);
			a3 = L_mac(a3, x3, c0);
			a4 = L_msu(a4, d0, c0);
			a4 = L_msu(a4, d1, c1);
			a4 = L_msu(a4, d2, c2);
			a4 = L_msu(a4, d3, c3);
			a5 = L_msu(a5, d1, c0);
			a5 = L_msu(a5, d2, c1);
			a5 = L_msu(a5, d3, c2);
			a6 = L_msu(a6, d2, c0);
			a6 = L_msu(a6, d3, c1);
			a7 = L_msu(a7, d3, c0);
			e0 = (j == 20) ? L_mult(x3, 16384) : e0;
			f0 = (j == 20) ? L_mult(d3, 16384) : f0;

			x0 = *cPtr++;
			x1 = *cPtr++;
			x2 = *cPtr++;
			x3 = *cPtr++;
			d0 = *cPtrI++;
			d1 = *cPtrI++;
			d2 = *cPtrI++;
			d3 = *cPtrI++;

			a1 = L_mac(a1, x0, c3);
			a2 = L_mac(a2, x0, c2);
			a2 = L_mac(a2, x1, c3);
			a3 = L_mac(a3, x0, c1);
			a3 = L_mac(a3, x1, c2);
			a3 = L_mac(a3, x2, c3);
			a5 = L_msu(a5, d0, c3);
			a6 = L_msu(a6, d0, c2);
			a6 = L_msu(a6, d1, c3);
			a7 = L_msu(a7, d0, c1);
			a7 = L_msu(a7, d1, c2);
			a7 = L_msu(a7, d2, c3);

			e1 = (j == 20) ? L_mult(x0, 16384) : e1;
			e2 = (j == 20) ? L_mult(x1, 16384) : e2;
			e3 = (j == 20) ? L_mult(x2, 16384) : e3;
			f1 = (j == 20) ? L_mult(d0, 16384) : f1;
			f2 = (j == 20) ? L_mult(d1, 16384) : f2;
			f3 = (j == 20) ? L_mult(d2, 16384) : f3;

		} while ((j -= 4) != 0);
		cPtr -= 32;
		cPtrI -= 32;
		ptr += 32;
		tmp32no1 = sat_32_16(L_add(e0, f0) >> 15);
		tmp32no2 = sat_32_16(L_add(a0, a4) >> 15);
		*Out++ = tmp32no1;
		*Out++ = tmp32no2;
		tmp32no1 = sat_32_16(L_add(e1, f1) >> 15);
		tmp32no2 = sat_32_16(L_add(a1, a5) >> 15);
		*Out++ = tmp32no1;
		*Out++ = tmp32no2;
		tmp32no1 = sat_32_16(L_add(e2, f2) >> 15);
		tmp32no2 = sat_32_16(L_add(a2, a6) >> 15);
		*Out++ = tmp32no1;
		*Out++ = tmp32no2;
		tmp32no1 = sat_32_16(L_add(e3, f3) >> 15);
		tmp32no2 = sat_32_16(L_add(a3, a7) >> 15);
		*Out++ = tmp32no1;
		*Out++ = tmp32no2;

	} while ((i -= 4) != 0);
#endif
}

/*STFT based AES*/
Word32 PBFDSR(Word16 * const __restrict Sin, Word16 * const __restrict Rin,
		Word16 * const out, FDSR_STATE * __restrict st) { // stage-wise regressions
	Word16 hnl[part_len];
	Word16 buffer_rstft[nDFT];
	Word16 buffer_sstft[nDFT];

	Word32 i, j;
	Word32 zeros = 0;
	Word32 numPosCoef = 0;
	Word32 avgHnl32 = 0;

	Word32 tmp32no1;
	Word32 tmp32no2;
	Word32 tmp32no3;
	Word32 zeros32;
	Word32 absVf;
	Word32 supGain = 0;
	Word32 qDomainDiff;
	Word32 qDomain;
	Word32 qDomainAlpha;

	Word32 const kMinPrefBand = 10;
	Word32 const kMaxPrefBand = 107;

	Word32 blk_expno1;
	Word32 blk_expno2;

	ComplexInt32 * __restrict c32ptrno1;
	ComplexInt32 * __restrict c32ptrno2;
	Word64 * __restrict r64ptrno1;
	Word64 * __restrict r64ptrno2;
	Word64 * __restrict r64ptrno3;

	ComplexInt32 V_k[part_len]; // residual echo + near_end
	ComplexInt32 Y_k[part_len];
	ComplexInt32 B_k[part_len];
	ComplexInt32 E_k[part_len]; // estimated echo
	ComplexInt32 F_k[part_len];
	ComplexInt32 H_k[part_len];

	Word64 norm;
	Word64 Thresh;
	Word64 tmp64no1;
	Word64 tmp64no2;
	Word64 tmp64no3;

	/*printf("%d\n", CLZ(0x03ffffff));
	 printf("%d\n", L_add(0x7fffffff, 0x1234));
	 printf("%d\n", L_mac(0x1234, 0x1234, 0x5678));
	 printf("%d\n", L_msu(0x1234, 0x1234, 0x5678));
	 printf("%d\n", L_mult(-32768, -32768));
	 printf("%d\n", L_sub(0x80000000, 0x1234));
	 printf("%d\n", lsl(0x7f7f, 2));
	 printf("%d\n", lsl(0x7f7f, -2));
	 printf("%d\n", sat_32_16(0x10000));
	 printf("%lld\n", SMLAL(0x1234, 0x5678, 0x4567));
	 printf("%lld\n", SMULL(0x4567, 0x1234));
	 printf("%lld\n", udiv_128_64(0x1234567812345678, 0x234567812345678));*/

	memset(E_k, 0, sizeof(ComplexInt32) * nDFT / 2); // WLS estimated echo set zeros
	memmove(st->buffer_rrin, &st->buffer_rrin[RR],
			sizeof(Word16) * (nDFT - RR));
	memmove(st->buffer_ssin, &st->buffer_ssin[RR],
			sizeof(Word16) * (nDFT - RR));
	memmove(&st->buffer_rrin[nDFT - RR], Rin, sizeof(Word16) * RR);
	memmove(&st->buffer_ssin[nDFT - RR], Sin, sizeof(Word16) * RR);

	//fwrite(st->buffer_ssin, sizeof(Word16), nDFT, dptr_iv);

	for (i = 0; i < nDFT; i += 1) { // STFT Windowing
		buffer_rstft[i] = L_add(L_mult(st->buffer_rrin[i], sqrt_hann_256[i]),
				0x8000) >> 16; // fars
		buffer_sstft[i] = L_add(L_mult(st->buffer_ssin[i], sqrt_hann_256[i]),
				0x8000) >> 16; // nears
	}
#if !defined(DelayEst)
	blk_expno1 = rfft_256((Complex16_t *) buffer_rstft, 256);
	blk_expno2 = rfft_256((Complex16_t *) buffer_sstft, 256);
	for (j = 1; j < nDFT / 2; j += 1) { // near-end init value
		V_k[j].real = lsl(((Complex16_t *) buffer_sstft)[j].real, blk_expno2);
		V_k[j].imag = lsl(((Complex16_t *) buffer_sstft)[j].imag, blk_expno2);
	}

	V_k[0].real = lsl(buffer_sstft[0], blk_expno2); // dc
	V_k[0].imag = lsl(buffer_sstft[1], blk_expno2); // nyquist
#else
			memmove(V_k,st->Yk,sizeof(ComplexInt32)*part_len);
#endif

	/*fwrite(&V_k[0].real, sizeof(Word32), 1, dptr_ii);
	 fwrite(&zeros, sizeof(Word32), 1, dptr_ii);
	 fwrite(&V_k[1], sizeof(ComplexInt32), nDFT / 2 - 1, dptr_ii);
	 fwrite(&V_k[0].imag, sizeof(Word32), 1, dptr_ii);
	 fwrite(&zeros, sizeof(Word32), 1, dptr_ii);*/

	memmove(Y_k, V_k, sizeof(ComplexInt32) * nDFT / 2);

#if !defined(DelayEst)
	for (i = 0; i < M - 1; i += 1) { // M blocks, (1+nDFT/2) bins
		memmove(st->XFm[i], st->XFm[i + 1],
				sizeof(ComplexInt32) * (1 + nDFT / 2)); // [old,...,new]
	}

	for (j = 1; j < nDFT / 2; j += 1) { // farend blocks
		st->XFm[M - 1][j].real = lsl(((ComplexInt16 *) buffer_rstft)[j].real,
				blk_expno1);
		st->XFm[M - 1][j].imag = lsl(((ComplexInt16 *) buffer_rstft)[j].imag,
				blk_expno1);
	}

	st->XFm[M - 1][0].real = lsl(buffer_rstft[0], blk_expno1); // dc
	st->XFm[M - 1][0].imag = 0;
	st->XFm[M - 1][nDFT / 2].real = lsl(buffer_rstft[1], blk_expno1); // nyquists
	st->XFm[M - 1][nDFT / 2].imag = 0;
#endif

	//fwrite(st->XFm[M - 1], sizeof(ComplexInt32), part_len1, dptr_iii);

	for (i = 0; i < M - 1; i += 1) {
		memmove(st->S_ss[i], st->S_ss[i + 1], sizeof(Word64) * (1 + nDFT / 2));
		memmove(st->qDomainOld[i], st->qDomainOld[i + 1],
				sizeof(Word16) * part_len1);
	}

	qDomainAlpha = CLZ(st->alpha_m);

	for (j = 0; j < part_len1; j += 1) {
		if (j != 0 && j != nDFT / 2) {
			tmp64no1 = (Word64) st->XFm[M - 1][j].real * st->XFm[M - 1][j].real;
			tmp64no1 += (Word64) st->XFm[M - 1][j].imag
					* st->XFm[M - 1][j].imag;
		} else if (!j) {
			tmp64no1 = (Word64) st->XFm[M - 1][0].real * st->XFm[M - 1][0].real;
		} else {
			tmp64no1 = (Word64) st->XFm[M - 1][part_len].real
					* st->XFm[M - 1][part_len].real;
		}
#if 1
		tmp32no1 = CLZ64(tmp64no1);
		tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
		tmp32no2 = st->qDomainOld[M - 1][j];
		if (tmp32no1 > tmp32no2) {
			tmp64no1 <<= tmp32no2;
			tmp64no3 = (tmp64no2 = st->S_ss[M - 1][j]) - tmp64no1;
			tmp32no3 = CLZ64(llabs(tmp64no3));
			zeros32 = tmp32no3 + qDomainAlpha;
			if (/*tmp32no3 < 10*/zeros32 < 32) {
				tmp64no3 >>= 15;
				tmp64no3 *= st->alpha_m;
				tmp64no2 -= tmp64no3;
			} else {
				tmp64no3 = tmp64no3 * st->alpha_m >> 15;
				tmp64no2 -= tmp64no3;
			}

			qDomain = tmp32no2;

		} else {
			qDomainDiff = tmp32no2 - tmp32no1;
			tmp64no2 = tmp64no3 = st->S_ss[M - 1][j] >> qDomainDiff;
			tmp64no1 <<= tmp32no1;
			tmp64no3 -= tmp64no1;
			tmp32no3 = CLZ64(llabs(tmp64no3));
			zeros32 = tmp32no3 + qDomainAlpha;

			if (/*tmp32no3 < 10*/zeros32 < 32) {
				tmp64no3 >>= 15;
				tmp64no3 *= st->alpha_m;
				tmp64no2 -= tmp64no3;
			} else {
				tmp64no3 = tmp64no3 * st->alpha_m >> 15;
				tmp64no2 -= tmp64no3;
			}

			qDomain = tmp32no1;
		}

		tmp32no3 = CLZ64(tmp64no2);
		//assert(tmp32no3 > 0);
		tmp32no3 = (tmp32no3 > 0) ? tmp32no3 - 1 : 0;
		st->S_ss[M - 1][j] = tmp64no2 <<= tmp32no3;
		//assert(qDomain + tmp32no3 < 63);

		st->qDomainOld[M - 1][j] = qDomain + tmp32no3;
#else
		tmp64no2 = tmp64no1 - st->S_ss[M - 1][j];
		tmp32no1 = CLZ64(llabs(tmp64no2));
		zeros32 = tmp32no1 + qDomainAlpha;
		if (/*tmp32no1 < 10*/zeros32 < 32) {
			tmp64no2 >>= 15;
			tmp64no1 = tmp64no2 * st->alpha_m;
		} else {
			tmp64no1 = tmp64no2 * st->alpha_m >> 15;
		}
		st->S_ss[M - 1][j] += tmp64no1;
#endif
	}
#if 0
	tmp64no1 = (Word64) st->XFm[M - 1][0].real * st->XFm[M - 1][0].real;
	tmp64no2 = (tmp64no1 - st->S_ss[M - 1][0]);
	tmp32no1 = CLZ64(llabs(tmp64no2));
	zeros32 = tmp32no1 + qDomainAlpha;
	if (/*tmp32no1 < 10*/zeros32 < 32) {
		tmp64no2 >>= 15;
		tmp64no1 = tmp64no2 * st->alpha_m;
	} else {
		tmp64no1 = tmp64no2 * st->alpha_m >> 15;
	}
	st->S_ss[M - 1][0] += tmp64no1; // dc

	tmp64no1 = (Word64) st->XFm[M - 1][nDFT / 2].real
	* st->XFm[M - 1][nDFT / 2].real;
	tmp64no2 = tmp64no1 - st->S_ss[M - 1][nDFT / 2];
	tmp32no1 = CLZ64(llabs(tmp64no2));
	zeros32 = tmp32no1 + qDomainAlpha;
	if (/*tmp32no1 < 10*/zeros32 < 32) {
		tmp64no2 >>= 15;
		tmp64no1 = tmp64no2 * st->alpha_m;
	} else {
		tmp64no1 = tmp64no2 * st->alpha_m >> 15;
	}
	st->S_ss[M - 1][nDFT / 2] += tmp64no1; // nyquists
#endif

	/*for (i = 0; i < part_len1; i += 1) {
	 float temp;
	 temp = st->S_ss[M - 1][i] * powf(2.f, -st->qDomainOld[M - 1][i]);
	 //fwrite(&temp, sizeof(float), 1, dptr_iv);
	 }*/

	tmp64no1 = 0;
	tmp64no2 = 0;
	for (i = kMinPrefBand; i < kMaxPrefBand; i += 1) {
		tmp64no1 += st->S_ss[M - 1][i] >> st->qDomainOld[M - 1][i];
		tmp64no2 += (Word64) st->XFm[M - 1][i].real * st->XFm[M - 1][i].real;
		tmp64no2 += (Word64) st->XFm[M - 1][i].imag * st->XFm[M - 1][i].imag;
	}
	tmp32no1 = (tmp64no1) ? 63 - CLZ64(tmp64no1) : 0;

	st->currentVADvalue = (tmp32no1 > Noise_Floor) ? 1 : 0;
//fwrite(&tmp32no1, sizeof(Word32), 1, dptr_ii);

#if 1
	for (i = M - 1; i >= 0; i -= 1) {

		tmp64no1 = (Word64) V_k[0].real * st->XFm[i][0].real; // mostly correlated blocks
		tmp32no1 = CLZ64(llabs(tmp64no2 = tmp64no1 - st->S_vs[i][0]));
		zeros32 = tmp32no1 + qDomainAlpha;
		if (/*tmp32no1 < 10*/zeros32 < 32) {
			tmp64no2 >>= 15;
			tmp64no1 = tmp64no2 * st->alpha_m;
		} else {
			tmp64no1 = tmp64no2 * st->alpha_m >> 15;
		}

		st->S_vs[i][0] += tmp64no1; // real
		st->S_vs[i][1] = 0; // imag

		tmp32no1 = CLZ64(tmp64no1 = llabs(st->S_vs[i][0]));
		tmp32no2 = CLZ64(
				tmp64no2 = max(1uLL << st->qDomainOld[i][0], st->S_ss[i][0]));
		tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
		tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;

		tmp64no1 <<= tmp32no1;
		tmp64no2 <<= tmp32no2;

#if defined(FIXED_POINT)

		if ((UWord64) tmp64no1 < (UWord64) tmp64no2) {
			H_k[0].real = asr(udiv_128_64(tmp64no2, tmp64no1),
					tmp32no1 - tmp32no2 - st->qDomainOld[i][0]);
		} else {
			register Word32 tmp32no3;

			tmp64no1 -= tmp64no2;
			tmp32no3 = udiv_128_64(tmp64no2, tmp64no1);
			tmp32no3 += 32767;
			H_k[0].real = asr(tmp32no3,
					tmp32no1 - tmp32no2 - st->qDomainOld[i][0]);
		}
		H_k[0].real *= (st->S_vs[i][0] < 0) ? -1 : 1;
#else
		H_k[0].real = (Float32) 32767.f
		* st->S_vs[i][0] / max(1uLL, st->S_ss[i][0]>>st->qDomainOld[i][0]);
#endif
		V_k[0].real -= SMULL(st->XFm[i][0].real, H_k[0].real) >> 15;
		E_k[0].real += SMULL(st->XFm[i][0].real, H_k[0].real) >> 15;

		//fwrite(&st->S_vs[i][0], sizeof(Word64), 1, dptr_ii);
		c32ptrno1 = &st->XFm[i][1];
		c32ptrno2 = &V_k[1];
		r64ptrno1 = &st->S_vs[i][2];
		r64ptrno2 = &st->S_vs[i][3];
		r64ptrno3 = &st->S_ss[i][1];

		for (j = 1; j < nDFT / 2; j += 1) {
			//float tmpfno1;
			//float tmpfno2;
			ComplexInt32 tmpc32no1;
#if !defined(OTIME)
			tmp64no1 = (Word64) V_k[j].real * st->XFm[i][j].real; //S_vs.real
			tmp64no1 += (Word64) V_k[j].imag * st->XFm[i][j].imag;

			tmp64no2 = (Word64) V_k[j].imag * st->XFm[i][j].real;//S_vs.imag
			tmp64no2 -= (Word64) V_k[j].real * st->XFm[i][j].imag;
#else
			tmp64no1 = (Word64) c32ptrno2->real * c32ptrno1->real;
			tmp64no1 += (Word64) c32ptrno2->imag * c32ptrno1->imag;

			tmp64no2 = (Word64) c32ptrno2->imag * c32ptrno1->real;
			tmp64no2 -= (Word64) c32ptrno2->real * c32ptrno1->imag;
			//__builtin_prefetch((c32ptrno2 + 1), 0, 1);
#endif

#if !defined(OTIME)
			//tmp64no1 = (tmp64no1 - st->S_vs[i][2 * j]) * st->alpha_m >> 15;
			tmp32no1 = CLZ64(llabs(tmp64no3 = tmp64no1 - st->S_vs[i][2 * j]));
			zeros32 = tmp32no1 + qDomainAlpha;
			if (/*tmp32no1 < 10*/zeros32 < 32) {
				tmp64no3 >>= 15;
				tmp64no1 = tmp64no3 * st->alpha_m;
			} else {
				tmp64no1 = tmp64no3 * st->alpha_m >> 15;
			}

			st->S_vs[i][2 * j] += tmp64no1;

			//tmp64no2 = (tmp64no2 - st->S_vs[i][2 * j + 1]) * st->alpha_m >> 15;
			tmp32no1 = CLZ64(
					llabs(tmp64no2 = tmp64no2 - st->S_vs[i][2 * j + 1]));
			zeros32 = tmp32no1 + qDomainAlpha;
			if (/*tmp32no1 < 10*/zeros32 < 32) {
				tmp64no2 >>= 15;
				tmp64no2 *= st->alpha_m;
			} else {
				tmp64no2 = tmp64no2 * st->alpha_m >> 15;
			}

			st->S_vs[i][2 * j + 1] += tmp64no2;
#else
			tmp32no1 = CLZ64(llabs(tmp64no3 = tmp64no1 - *r64ptrno1));
			zeros32 = tmp32no1 + qDomainAlpha;
			if (/*tmp32no1 < 10*/zeros32 < 32) {
				tmp64no3 >>= 15;
				tmp64no1 = tmp64no3 * st->alpha_m;
			} else {
				tmp64no1 = tmp64no3 * st->alpha_m >> 15;
			}
			*r64ptrno1 += tmp64no1;

			tmp32no1 = CLZ64(llabs(tmp64no3 = tmp64no2 - *r64ptrno2));
			zeros32 = tmp32no1 + qDomainAlpha;
			if (/*tmp32no1 < 10*/zeros32 < 32) {
				tmp64no3 >>= 15;
				tmp64no2 = tmp64no3 * st->alpha_m;
			} else {
				tmp64no2 = tmp64no3 * st->alpha_m >> 15;
			}
			*r64ptrno2 += tmp64no2;
#endif
			//fwrite(&st->S_vs[i][2 * j], sizeof(Word64), 1, dptr_ii);
#if !defined(OTIME)
			tmp32no1 = CLZ64(ABS(st->S_vs[i][2 * j]));
			tmp32no2 = CLZ64(max(1uLL<<st->qDomainOld[i][j], st->S_ss[i][j]));
#else
			tmp32no1 = CLZ64(tmp64no1 = ABS(*r64ptrno1));
			tmp32no2 = CLZ64(
					tmp64no2 = max(1uLL << st->qDomainOld[i][j], *r64ptrno3));
#endif
			//assert(tmp32no1 > 0);
			//assert(tmp32no2 > 0);
			tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
			tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;

#if !defined(OTIME)
			tmp64no1 = ABS(st->S_vs[i][2 * j]) << tmp32no1;
			tmp64no2 = max(1uLL<<st->qDomainOld[i][j],st->S_ss[i][j]) << tmp32no2;
#else
			tmp64no1 <<= tmp32no1;
			tmp64no2 <<= tmp32no2;
			r64ptrno3++;
#endif
#if defined(FIXED_POINT)
			if ((UWord64) tmp64no1 < (UWord64) tmp64no2) {
				H_k[j].real = asr(udiv_128_64(tmp64no2, tmp64no1),
						tmp32no1 - tmp32no2 - st->qDomainOld[i][j]);
			} else {
				register Word32 tmp32no3;

				tmp64no1 -= tmp64no2;
				tmp32no3 = udiv_128_64(tmp64no2, tmp64no1);
				tmp32no3 += 32767;
				H_k[j].real = asr(tmp32no3,
						tmp32no1 - tmp32no2 - st->qDomainOld[i][j]);
			}
#if !defined(OTIME)
			H_k[j].real *= (st->S_vs[i][2 * j] < 0) ? -1 : 1;
#else
			H_k[j].real *= (*r64ptrno1 < 0) ? -1 : 1;
#endif

#else
			H_k[j].real = (Float32) 32767.f
			* st->S_vs[i][2 * j]/ max(1uLL, st->S_ss[i][j]>>st->qDomainOld[i][j]);
			//fwrite(&H_k[j].real, sizeof(Word32), 1, dptr_ii);
#endif
			r64ptrno1 += 2;

#if defined(FIXED_POINT)
#if !defined(OTIME)
			tmp32no1 = CLZ64(ABS(st->S_vs[i][2 * j + 1]));
			tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;

			tmp64no1 = ABS(st->S_vs[i][2 * j + 1]) << tmp32no1;
#else
			tmp32no1 = CLZ64(tmp64no1 = ABS(*r64ptrno2));
			tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
			tmp64no1 <<= tmp32no1;
#endif
#endif

#if defined(FIXED_POINT)
			if ((UWord64) tmp64no1 < (UWord64) tmp64no2)
				H_k[j].imag = asr(udiv_128_64(tmp64no2, tmp64no1),
						tmp32no1 - tmp32no2 - st->qDomainOld[i][j]);
			else {
				register Word32 tmp32no3;

				tmp64no1 -= tmp64no2;
				tmp32no3 = udiv_128_64(tmp64no2, tmp64no1);
				tmp32no3 += 32767;
				H_k[j].imag = asr(tmp32no3,
						tmp32no1 - tmp32no2 - st->qDomainOld[i][j]);
			}
#if !defined(OTIME)
			H_k[j].imag *= (st->S_vs[i][2 * j + 1] < 0) ? -1 : 1;
#else
			H_k[j].imag *= (*r64ptrno2 < 0) ? -1 : 1;
#endif

#else
			H_k[j].imag = (Float32) 32767.f
			* st->S_vs[i][2 * j + 1]/ max(1uLL, st->S_ss[i][j]>>st->qDomainOld[i][j]);
#endif
			r64ptrno2 += 2;

#if !defined(OTIME)
			V_k[j] = complex32_sub(V_k[j],
					complex32_mul(&st->XFm[i][j], &H_k[j]));

			E_k[j] = complex32_add(E_k[j],
					complex32_mul(&st->XFm[i][j], &H_k[j]));
#else
#if 1
			tmpc32no1 = complex32_mul(c32ptrno1++, &H_k[j]);
#else
			tmpc32no1.real = ((Word64) c32ptrno1->real * H_k[j].real
					- (Word64) c32ptrno1->imag * H_k[j].imag) >> 13;
			tmpc32no1.imag = ((Word64) c32ptrno1->real * H_k[j].imag
					+ (Word64) c32ptrno1->imag * H_k[j].real) >> 13;
			c32ptrno1++;
#endif

			V_k[j] = complex32_sub(*c32ptrno2++, tmpc32no1);
			E_k[j] = complex32_add(E_k[j], tmpc32no1);

#endif
		}

		tmp64no1 = (Word64) V_k[0].imag * st->XFm[i][nDFT / 2].real;
		tmp32no1 = CLZ64(llabs(tmp64no2 = tmp64no1 - st->S_vs[i][nDFT]));
		zeros32 = tmp32no1 + qDomainAlpha;
		if (/*tmp32no1 < 10*/zeros32 < 32) {
			tmp64no2 >>= 15;
			tmp64no1 = tmp64no2 * st->alpha_m;
		} else {
			tmp64no1 = tmp64no2 * st->alpha_m >> 15;
		}

		st->S_vs[i][nDFT] += tmp64no1;
		st->S_vs[i][nDFT + 1] = 0;

		tmp32no1 = CLZ64(tmp64no1 = ABS(st->S_vs[i][nDFT]));
		tmp32no2 = CLZ64(
				tmp64no2 = max(1uLL << st->qDomainOld[i][part_len],
						st->S_ss[i][part_len]));
		tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0; // TODO: over-estimated?
		tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;

		tmp64no1 <<= tmp32no1;
		tmp64no2 <<= tmp32no2;
#if defined(FIXED_POINT)
		if ((UWord64) tmp64no1 < (UWord64) tmp64no2)
			H_k[0].imag = asr(udiv_128_64(tmp64no2, tmp64no1),
					tmp32no1 - tmp32no2 - st->qDomainOld[i][part_len]);
		else {
			register Word32 tmp32no3;

			tmp64no1 -= tmp64no2;
			tmp32no3 = udiv_128_64(tmp64no2, tmp64no1);
			tmp32no3 += 32767;
			H_k[0].imag = tmp32no3 = asr(tmp32no3,
					tmp32no1 - tmp32no2 - st->qDomainOld[i][part_len]);
		}
		H_k[0].imag *= (st->S_vs[i][nDFT] < 0) ? -1 : 1;
#else
		H_k[0].imag = (Float32) 32767.f
		* st->S_vs[i][nDFT]/ max(1uLL, st->S_ss[i][nDFT / 2]>>st->qDomainOld[i][part_len]);
#endif
		V_k[0].imag -= SMULL(st->XFm[i][nDFT / 2].real, H_k[0].imag) >> 15;
		E_k[0].imag += SMULL(st->XFm[i][nDFT / 2].real, H_k[0].imag) >> 15;

		//fwrite(&st->S_vs[i][nDFT], sizeof(Word64), 1, dptr_ii);
#if 0
		fwrite(&V_k[0].real, sizeof(Word32), 1, dptr_ii);
		fwrite(&zeros, sizeof(Word32), 1, dptr_ii);
		fwrite(&V_k[1], sizeof(ComplexInt32), nDFT / 2 - 1, dptr_ii);
		fwrite(&V_k[0].imag, sizeof(Word32), 1, dptr_ii);
		fwrite(&zeros, sizeof(Word32), 1, dptr_ii);
#endif
	}

	//fwrite(st->S_vs[M - 1], sizeof(Word64), part_len1 << 1, dptr_iv);

	/*fwrite(&H_k[0].real, sizeof(Word32), 1, dptr_iv);
	 fwrite(&zeros, sizeof(Word32), 1, dptr_iv);
	 fwrite(&H_k[1], sizeof(ComplexInt32), nDFT / 2 - 1, dptr_iv);
	 fwrite(&H_k[0].imag, sizeof(Word32), 1, dptr_iv);
	 fwrite(&zeros, sizeof(Word32), 1, dptr_iv);*/

	for (i = 0; i < nDFT / 2; i += 1) { // Constrainted Magnitude
		if (i) {
			norm = (Word64) V_k[i].real * V_k[i].real;
			norm += (Word64) V_k[i].imag * V_k[i].imag;
			//norm = sqrtf(norm);
			norm = usqrt_ll(norm);

			Thresh = (Word64) Y_k[i].real * Y_k[i].real;
			Thresh += (Word64) Y_k[i].imag * Y_k[i].imag;
			//Thresh = sqrtf(Thresh);
			Thresh = usqrt_ll(Thresh);

			absVf = max(1, max(norm, Thresh));
			//absVf = Thresh / (1 + absVf); // Q15 ?
			tmp32no1 = CLZ(Thresh);
			tmp32no2 = CLZ(absVf);
			tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
			tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;
			Thresh <<= tmp32no1;
			absVf <<= tmp32no2;
			if (Thresh > absVf) {
				Thresh -= absVf;
				absVf = udiv_64_32(absVf, Thresh);
				absVf += 32767;
			} else {
				absVf = udiv_64_32(absVf, Thresh);
			}
			absVf = asr(absVf, tmp32no1 - tmp32no2);
			//V_k[i].real *= absVf;
			//V_k[i].imag *= absVf;
			V_k[i].real = (Word64) V_k[i].real * absVf >> 15;
			V_k[i].imag = (Word64) V_k[i].imag * absVf >> 15;

		} else {
			norm = ABS(V_k[i].real);
			Thresh = ABS(Y_k[i].real);
			absVf = max(1, max(norm, Thresh));
			//absVf = Thresh / (1 + absVf);
			tmp32no1 = CLZ(Thresh);
			tmp32no2 = CLZ(absVf);
			tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
			tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;
			Thresh <<= tmp32no1;
			absVf <<= tmp32no2;
			if (Thresh > absVf) {
				Thresh -= absVf;
				absVf = udiv_64_32(absVf, Thresh);
				absVf += 32767;
			} else {
				absVf = udiv_64_32(absVf, Thresh);
			}
			absVf = asr(absVf, tmp32no1 - tmp32no2);
			//V_k[i].real *= absVf;
			V_k[i].real = (Word64) V_k[i].real * absVf >> 15;

			norm = ABS(V_k[i].imag);
			Thresh = ABS(Y_k[i].imag);
			absVf = max(1, max(norm, Thresh));
			//absVf = Thresh / (1 + absVf);
			tmp32no1 = CLZ(Thresh);
			tmp32no2 = CLZ(absVf);
			tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
			tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;
			Thresh <<= tmp32no1;
			absVf <<= tmp32no2;

			if (Thresh > absVf) {
				Thresh -= absVf;
				absVf = udiv_64_32(absVf, Thresh);
				absVf += 32767;
			} else {
				absVf = udiv_64_32(absVf, Thresh);
			}
			absVf = asr(absVf, tmp32no1 - tmp32no2);
			//V_k[i].imag *= absVf;
			V_k[i].imag = (Word64) V_k[i].imag * absVf >> 15;
		}
	}

#endif

#if 0
	fwrite(&Y_k[0].real, sizeof(Word32), 1, dptr_ii);
	fwrite(&zeros, sizeof(Word32), 1, dptr_ii);
	fwrite(&Y_k[1], sizeof(ComplexInt32), nDFT / 2 - 1, dptr_ii);
	fwrite(&Y_k[0].imag, sizeof(Word32), 1, dptr_ii);
	fwrite(&zeros, sizeof(Word32), 1, dptr_ii);
#endif

	blk_expno1 = norm2Word16(V_k, nDFT);

	for (i = 0; i < nDFT / 2; i += 1) {
		((ComplexInt16*) buffer_rstft + i)->real = asr(V_k[i].real, blk_expno1);
		((ComplexInt16*) buffer_rstft + i)->imag = asr(V_k[i].imag, blk_expno1);
	}

	//noise_suppression(&NR_STATE_TX, (ComplexInt16*) buffer_rstft, blk_expno1);
	for (i = 0; i < nDFT / 2; i += 1) {

		if (i) {
			tmp64no1 = SMULL(V_k[i].real, V_k[i].real);
			tmp64no1 = SMLAL(tmp64no1, V_k[i].imag, V_k[i].imag);
			tmp32no1 = CLZ64(llabs(tmp64no1 = st->S_ee[i] - tmp64no1));
			if (tmp32no1 < 15) {
				tmp64no1 >>= 15;
				tmp64no1 *= alpha_s;
			} else {
				tmp64no1 = mul_64_64(tmp64no1, alpha_s) >> 15;
			}

			//tmp64no1 = mul_64_64(st->S_ee[i] - tmp64no1, alpha_s) >> 15;
			st->S_ee[i] -= tmp64no1; // regards only ambient noise plus near speech

			/*F_k[i].real = ((ComplexInt16*) buffer_rstft + i)->real
			 << blk_expno1; // clean speech
			 F_k[i].imag = ((ComplexInt16*) buffer_rstft + i)->imag
			 << blk_expno1;*/
			F_k[i].real = V_k[i].real;
			F_k[i].imag = V_k[i].imag;
			tmp64no1 = SMULL((F_k + i)->real, (F_k + i)->real);
			tmp64no1 = SMLAL(tmp64no1, (F_k + i)->imag, (F_k + i)->imag);
			tmp32no1 = CLZ64(llabs(tmp64no2 = st->S_nn[i] - tmp64no1));
			if (tmp32no1 < 15) {
				tmp64no2 >>= 15;
				tmp64no2 = mul_64_64(tmp64no2, alpha_s);
			} else {
				tmp64no2 = mul_64_64(tmp64no2, alpha_s) >> 15;
			}

			//tmp64no2 = mul_64_64(st->S_nn[i] - tmp64no1, alpha_s) >> 15;
			st->S_nn[i] -= tmp64no2; // near speech

			tmp64no1 = usqrt_ll(tmp64no1);
			tmp64no1 = (st->nearFilt[i] - tmp64no1);
			st->nearFilt[i] -= tmp64no1 >> 4; // filt |res+near-speech|

			tmp64no2 = SMULL(Y_k[i].real, Y_k[i].real);
			tmp64no2 = SMLAL(tmp64no2, Y_k[i].imag, Y_k[i].imag);

			tmp32no1 = CLZ64(tmp64no2);
			tmp32no2 = CLZ64(tmp64no1 = max(1uLL, st->S_ee[i]));
			tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
			tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0; // norm to (0.5,1)<=1

			tmp64no2 <<= tmp32no1; // numerator
			tmp64no1 <<= tmp32no2; // denominator

			if (tmp64no2 < tmp64no1) {
				tmp32no3 = udiv_128_64(tmp64no1, tmp64no2); // post-snr
			} else {
				tmp64no2 -= tmp64no1;
				tmp32no3 = udiv_128_64(tmp64no1, tmp64no2);
				tmp32no3 += 32767;
			}

			tmp32no2 = asr(tmp32no3, tmp32no1 - tmp32no2); // Q15
			//fwrite(&tmp32no2, sizeof(Word32), 1, dptr_iv);
			tmp32no1 = L_add(L_mult(st->Gainno1[i], st->Gainno1[i]), 0x8000)
					>> 16; // Q15
			tmp32no1 = sat_64_32(
					((Word64) st->gammano1[i] * tmp32no1 + 0x2000) >> 14); // Q16

			st->gammano1[i] = tmp32no2;
			tmp32no1 -= ((Word64) tmp32no1 * alpha_d + 0x4000) >> 15;

			tmp32no2 = max(tmp32no2 - 32767, 0);
			tmp32no2 = ((Word64) tmp32no2 * alpha_d + 0x2000) >> 14; // Q16
			tmp32no1 += tmp32no2; // ksi
			//fwrite(&tmp32no1, sizeof(Word32), 1, dptr_iv);
			st->Gainno1[i] = udiv_64_32(tmp32no1 + 65536, tmp32no1); // wiener gain

			B_k[i].real = ((Word64) st->Gainno1[i] * Y_k[i].real >> 15)
					- E_k[i].real;
			B_k[i].imag = ((Word64) st->Gainno1[i] * Y_k[i].imag >> 15)
					- E_k[i].imag;

			tmp64no1 = SMULL(B_k[i].real, B_k[i].real);
			tmp64no1 = SMLAL(tmp64no1, B_k[i].imag, B_k[i].imag);
			tmp32no1 = CLZ64(llabs(tmp64no1 = st->lambda[i] - tmp64no1));

			if (tmp32no1 < 15) {
				tmp64no1 >>= 15;
				tmp64no1 *= alpha_s;
			} else {
				tmp64no1 = tmp64no1 * alpha_s >> 15;
			}

			//tmp64no1 = (st->lambda[i] - tmp64no1) * alpha_s >> 15;
			st->lambda[i] -= tmp64no1;
			//fwrite(&st->lambda[i], sizeof(Word64), 1, dptr_iv);
			// Recursion II for roughly speech spectral
#if 0
			tmp64no1 = SMULL(V_k[i].real, V_k[i].real);
			tmp64no1 = SMLAL(tmp64no1, V_k[i].imag, V_k[i].imag);

			tmp32no1 = CLZ64(tmp64no1); // numerators
			tmp32no2 = CLZ64(tmp64no2 = max(1uLL, st->lambda[i]));// denominators
			tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
			tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;

			tmp64no1 <<= tmp32no1;
			tmp64no2 <<= tmp32no2;
			if (tmp64no1 > tmp64no2) {
				tmp64no1 -= tmp64no2;
				tmp32no3 = udiv_128_64(tmp64no2, tmp64no1);
				tmp32no3 += 32767;
			} else {
				tmp32no3 = udiv_128_64(tmp64no2, tmp64no1);
			}
			tmp32no2 = asr(tmp32no3, tmp32no1 - tmp32no2); // Q15,post-snr
			tmp32no1 = L_add(L_mult(st->Gainno2[i], st->Gainno2[i]), 0x8000)
			>> 16;
			tmp32no1 = sat_64_32(((Word64) tmp32no1 * st->gammano2[i] + 0x2000) >> 14);
			st->gammano2[i] = tmp32no2;
			tmp32no1 -= ((Word64) tmp32no1 * alpha_d + 0x4000) >> 15;
			tmp32no2 = max(tmp32no2 - 32767, 0);
			tmp32no2 = ((Word64) tmp32no2 * alpha_d + 0x2000) >> 14;
			tmp32no1 += tmp32no2;// a priori SNR
			st->Gainno2[i] = udiv_64_32(tmp32no1 + 65536, tmp32no1);
#else
			tmp64no1 = SMULL((E_k + i)->real, (E_k + i)->real);
			tmp64no1 = SMLAL(tmp64no1, (E_k + i)->imag, (E_k + i)->imag);

			tmp64no1 = usqrt_ll(tmp64no1);
			tmp64no1 = (tmp64no1 - st->echoEst64Gained[i]);
			st->echoEst64Gained[i] += tmp64no1 * 50 >> 7;
			//fwrite(&st->echoEst64Gained[i], sizeof(Word64), 1, dptr_iv);
#endif

		} else {
			tmp64no1 = (Word64) V_k[0].real * V_k[0].real;
			tmp32no1 = CLZ64(llabs(tmp64no1 = st->S_ee[0] - tmp64no1));
			if (tmp32no1 < 15) {
				tmp64no1 >>= 15;
				tmp64no1 *= alpha_s;
			} else {
				tmp64no1 = tmp64no1 * alpha_s >> 15;
			}
			st->S_ee[0] -= tmp64no1;

			tmp64no1 = (Word64) V_k[0].imag * V_k[0].imag;
			tmp32no1 = CLZ64(llabs(tmp64no1 = st->S_ee[part_len] - tmp64no1));
			if (tmp32no1 < 15) {
				tmp64no1 >>= 15;
				tmp64no1 *= alpha_s;
			} else {
				tmp64no1 = tmp64no1 * alpha_s >> 15;
			}
			st->S_ee[nDFT / 2] -= tmp64no1;

			st->gammano1[0] = 0;
			st->gammano1[part_len] = 0;
			st->gammano2[0] = 0;
			st->gammano2[part_len] = 0;

			B_k[0].real = 0; // residual-dc
			B_k[0].imag = 0; // residual-nyquist
			F_k[0].real = 0;
			F_k[0].imag = 0;
		}
	}

//fwrite(st->nearFilt, sizeof(Word64), part_len1, dptr_iv);

	tmp64no1 = 0;
	tmp64no2 = 0;
	for (i = kMinPrefBand; i < kMaxPrefBand; i += 1) {
		tmp64no1 += st->S_nn[i];
		//tmp64no1 += st->S_ee[i];
		tmp64no2 += st->lambda[i];
	}

//fwrite(&tmp64no2, sizeof(Word64), 1, dptr_iv);
	/*#if defined(__GNUC__)
	 float tmpfno1;
	 tmpfno1 = min((float)tmp64no2 / max(1uLL, tmp64no1), 1.f);
	 #endif*/

	tmp32no1 = CLZ64(tmp64no1 = max(1uLL, tmp64no1));
	tmp32no2 = CLZ64(tmp64no2);
	tmp32no1 = (tmp32no1 > 0) ? tmp32no1 - 1 : 0;
	tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;
	tmp64no1 <<= tmp32no1;
	tmp64no2 <<= tmp32no2;

	if (tmp64no2 > tmp64no1) {
		tmp64no2 -= tmp64no1;
		tmp64no1 = udiv_128_64(tmp64no1, tmp64no2);
		tmp64no1 += 32767;
	} else {
		tmp64no1 = udiv_128_64(tmp64no1, tmp64no2);
	}

	tmp64no1 = asr(tmp64no1, tmp32no2 - tmp32no1);
	tmp32no1 = max(0x3f, min(tmp64no1, 32767));
//fwrite(&tmp32no1, sizeof(Word32), 1, dptr_iv);
	//st->alpha_m = (1 - powf(0.98f, ((float) tmp32no1 / 32768.f))) * 32767;
//fwrite(&st->alpha_m, sizeof(Word16), 1, dptr_iv);
	supGain = (st->currentVADvalue) ? SUPGAIN_DEFAULT : 1;

	for (i = 1; i < part_len; i += 1) {
		tmp64no1 = st->echoEst64Gained[i] * tmp32no1 >> 15;
		//tmp64no1 = st->echoEst64Gained[i];
		tmp64no1 *= supGain;

		tmp32no2 = CLZ64(tmp64no1);
		tmp32no3 = CLZ64(tmp64no2 = max(1uLL, st->nearFilt[i]));
		tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;
		tmp32no3 = (tmp32no3 > 0) ? tmp32no3 - 1 : 0;
		tmp64no1 <<= tmp32no2;
		tmp64no2 <<= tmp32no3;

		if (tmp64no1 > tmp64no2) {
			tmp64no1 -= tmp64no2;
			tmp64no1 = udiv_128_64(tmp64no2, tmp64no1);
			tmp64no1 += 32767;
		} else {
			tmp64no1 = udiv_128_64(tmp64no2, tmp64no1);
		}

		tmp32no2 = asr(tmp64no1, tmp32no2 - tmp32no3);
		//fwrite(&tmp32no2, sizeof(Word32), 1, dptr_iv);
		tmp32no2 = max(32767 - tmp32no2, 0);
		*(hnl + i) = tmp32no2;
		numPosCoef += (tmp32no2) ? 1 : 0;
		avgHnl32 += hnl[i];
		(V_k + i)->real = (Word64) hnl[i] * F_k[i].real >> 15;
		(V_k + i)->imag = (Word64) hnl[i] * F_k[i].imag >> 15;
	}

	if (st->nlpFlag) {
		tmp64no1 = 0; // NLP
		tmp64no2 = 0;
		for (i = kMinPrefBand; i < kMaxPrefBand; i += 1) {
			tmp64no2 = SMLAL(tmp64no2, (F_k + i)->real, (F_k + i)->real);
			tmp64no2 = SMLAL(tmp64no2, (F_k + i)->imag, (F_k + i)->imag);
			tmp64no1 = SMLAL(tmp64no1, (V_k + i)->real, (V_k + i)->real);
			tmp64no1 = SMLAL(tmp64no1, (V_k + i)->imag, (V_k + i)->imag);
		}

		//tmp32no1 = (tmp64no1 > tmp64no2 * NLP_COMP_LOW);
		tmp32no1 = (numPosCoef > 3) ? 1 : 0;
		if (!tmp32no1) {
			memset(V_k, 0, sizeof(ComplexInt32) * part_len);
		}
	}

	tmp32no1 = norm2Word16(V_k, nDFT);

	for (i = 0; i < nDFT / 2; i += 1) {
		((ComplexInt16*) buffer_sstft + i)->real = asr(V_k[i].real, tmp32no1);
		((ComplexInt16*) buffer_sstft + i)->imag = asr(V_k[i].imag, tmp32no1);
	}

	hs_ifft_256((Complex16_t *) buffer_sstft, buffer_sstft, 256);

	for (i = 0; i < nDFT; i += 1) { // inverse windowing
		register Word32 L_var_out;
		L_var_out = asr(L_mult(buffer_sstft[i], sqrt_hann_256[i]),
				16 - tmp32no1);
		buffer_sstft[i] = sat_32_16(L_var_out);
	}

	/*for (i = 0; i < nDFT / 2; i += 1) {
	 register Word32 L_var_out;
	 L_var_out = L_add(buffer_sstft[i] << 16, st->buffer_olpa[i] << 16);
	 out[i] = L_var_out >> 16;
	 st->buffer_olpa[i] = buffer_sstft[i + nDFT / 2];
	 }*/
	for (i = 0; i < nDFT - RR; i += 1) {
		register Word32 L_var_out;
		L_var_out = L_add(buffer_sstft[i] << 16, st->buffer_olpa[i] << 16);
		st->buffer_olpa[i] = L_var_out >> 16;
	}
	memmove(out, st->buffer_olpa, sizeof(Word16) * RR);
	memmove(&st->buffer_olpa[0], &st->buffer_olpa[RR],
			sizeof(Word16) * (nDFT - 2 * RR));
	memmove(&st->buffer_olpa[nDFT - 2 * RR], &buffer_sstft[nDFT - RR],
			sizeof(Word16) * RR);

	//assert(kMaxPrefBand - kMinPrefBand >= 0);
	avgHnl32 /= (kMaxPrefBand - kMinPrefBand + 1);
	st->timer += 1;

	return (avgHnl32);
}

void PBFDAF(Word16 * const __restrict Sin, Word16 * const __restrict Rin,
		Word16 * const out, FDAF_STATE * __restrict st) {

	Word16 buffer_sfft[nFFT];
	Word16 buffer_rfft[nFFT];
//Word32 blk_expno1;
	Word32 blk_expno2;
	Word32 blk_expno3;

	Word32 i, j;

#if !defined(FIXED_POINT)
	ComplexFloat32 yfk[nFFT / 2];
	ComplexFloat32 E2k[M][nFFT / 2];
	ComplexFloat32 YFb[M][1 + nFFT / 2];
	Float32 tmp32no1;
	Float32 tmp32no2;

#else
	Complex32_t YFb[M][1 + nFFT / 2];
	Complex32_t yfk[nFFT / 2];
	Complex32_t E2k[M][nFFT / 2];
	Word32 tmp32no1;
	Word32 tmp32no2;
#endif
	Word32 tmp32no3;

	Complex16_t yfk_buffer[nFFT / 2];
	Word16 efk_buffer[nFFT];
	Word16 en_buffer[nFFT / 2];
	Word16 yn_buffer[nFFT / 2];

	memmove(st->buffer_rrin, &st->buffer_rrin[nFFT / 2],
			sizeof(Word16) * nFFT / 2);
	memmove(st->buffer_ssin, &st->buffer_ssin[nFFT / 2],
			sizeof(Word16) * nFFT / 2);
	memmove(&st->buffer_rrin[nFFT / 2], Rin, sizeof(Word16) * nFFT / 2);
	memmove(&st->buffer_ssin[nFFT / 2], Sin, sizeof(Word16) * nFFT / 2);

	memcpy(buffer_sfft, st->buffer_ssin, sizeof(Word16) * nFFT);
	memcpy(buffer_rfft, st->buffer_rrin, sizeof(Word16) * nFFT);
	blk_expno2 = rfft_128((Complex16_t*) buffer_rfft); // far-end

	for (i = 0; i < 1 + nFFT / 2; i += 1) {
		register Word32 xr;
		register Word32 xi;

		st->pn[i] -= (st->pn[i] >> 3);

		if (i && i != nFFT / 2) {
			xr = ((Complex16_t*) buffer_rfft + i)->real << blk_expno2;
			xi = ((Complex16_t*) buffer_rfft + i)->imag << blk_expno2;
			st->pn[i] += (((Word64) xr * xr + (Word64) xi * xi) >> 3);
		} else {
			if (!i) {
				xr = ((Complex16_t*) buffer_rfft)->real << blk_expno2;
				st->pn[0] += ((Word64) xr * xr >> 3);
			} else {
				xr = ((Complex16_t*) buffer_rfft)->imag << blk_expno2;
				st->pn[nFFT / 2] += ((Word64) xr * xr >> 3);
			}
		}
	}

	for (i = 0; i < M - 1; i += 1)
		memmove(st->XFm[i], st->XFm[i + 1],
				sizeof(Complex32_t) * (1 + nFFT / 2)); // Time Shift
	for (i = 1; i < nFFT / 2; i += 1) { // for M-1 block
		st->XFm[M - 1][i].real = lsl(((Complex16_t *) buffer_rfft)[i].real,
				blk_expno2);
		st->XFm[M - 1][i].imag = lsl(((Complex16_t*) buffer_rfft)[i].imag,
				blk_expno2);
	}

	st->XFm[M - 1][nFFT / 2].real = lsl(((Complex16_t *) buffer_rfft)[0].imag,
			blk_expno2);
	st->XFm[M - 1][nFFT / 2].imag = 0; // nyquist
	st->XFm[M - 1][0].real = lsl(((Complex16_t*) buffer_rfft)[0].real,
			blk_expno2);
	st->XFm[M - 1][0].imag = 0; // dc

	for (i = 0; i < M; i += 1) {
		j = nFFT / 2; // 1+nFFT/2 freq bins
		do {
#if defined(FIXED_POINT)
			Complex32_t cmpno1;
			cmpno1 = complex32_mul(&st->XFm[i][j], &st->WFb[i][j]); // Q15 coeffs
			(YFb[i] + j)->real = cmpno1.real;
			(YFb[i] + j)->imag = cmpno1.imag;
#else
			(*(YFb[i] + j)).real = st->XFm[i][j].real * st->WFb[i][j].real
			- st->XFm[i][j].imag * st->WFb[i][j].imag;
			(*(YFb[i] + j)).imag = st->XFm[i][j].real * st->WFb[i][j].imag
			+ st->XFm[i][j].imag * st->WFb[i][j].real;
#endif
		} while (--j >= 0);
	}

	for (i = 1; i < nFFT / 2; i += 1) {
		j = M - 1;
		tmp32no1 = 0;
		tmp32no2 = 0;
		do {
			tmp32no1 += (*(YFb[j] + i)).real; // block j, ith freq bin
			tmp32no2 += (*(YFb[j] + i)).imag; // filter sum outputs
		} while (--j >= 0);

#if !defined(FIXED_POINT)
		tmp32no1 /= 32768.f; // Q15 coeffs
		tmp32no2 /= 32768.f;
#endif
		yfk[i].real = tmp32no1;
		yfk[i].imag = tmp32no2;
	}

	tmp32no1 = 0;
	tmp32no2 = 0;
	for (j = M - 1; j >= 0; j -= 1) { // filter sum real dc, nyquist
		tmp32no1 += YFb[j][0].real;
		tmp32no2 += YFb[j][nFFT / 2].real;
	}

#if !defined(FIXED_POINT)
	yfk[0].real = tmp32no1 / 32768.f; // dc
	yfk[0].imag = tmp32no2 / 32768.f;// nyquist
#else
	yfk[0].real = tmp32no1;
	yfk[0].imag = tmp32no2;
#endif

//fwrite(yfk, sizeof(ComplexFloat32), nFFT / 2, dptr_i);
	tmp32no3 = norm2Word16(yfk, nFFT);

	for (i = 0; i < nFFT / 2; i += 1) {
		yfk_buffer[i].real = asr((Word32) yfk[i].real, tmp32no3);
		yfk_buffer[i].imag = asr((Word32) yfk[i].imag, tmp32no3);
	}

	hs_ifft_128(yfk_buffer, (Word16*) yfk_buffer); // error function

	memset(efk_buffer, 0, sizeof(Word16) * nFFT);
	for (i = 0; i < 64; i += 1) {
		efk_buffer[i + 64] = sat_32_16(
				L_sub(st->buffer_ssin[i + 64],
						lsl(((Word16*) yfk_buffer)[i + 64], tmp32no3)));
	}

	for (i = 0; i < nFFT / 2; i += 1) {
		yn_buffer[i] = sat_32_16(
				lsl(((Word16*) yfk_buffer)[i + nFFT / 2], tmp32no3));
	}

	memmove(en_buffer, efk_buffer + nFFT / 2, sizeof(Word16) * nFFT / 2);
	memmove(out, efk_buffer + 64, sizeof(Word16) * 64);

	blk_expno3 = rfft_128((Complex16_t*) efk_buffer);

	for (i = 0; i < 1 + nFFT / 2; i += 1) {
		Word64 tmp64no1;
		register Word32 tmp32;
		register Word32 tmp32no1;
		register Word32 tmp32no2;
		register Word32 xr;
		register Word32 xi;

#if !defined(FIXED_POINT)
		Float32 yr;
		Float32 yi;
#else
		Word64 yr;
		Word64 yi;
#endif

		tmp64no1 = epsilon + M * st->pn[i];
		tmp32 = ulog2(tmp64no1);
		j = M - 1;
		do {
			if (i && i != nFFT / 2) {

#if !defined(FIXED_POINT)
				Float32 norm;
				Float32 tmpno1;
				Float32 tmpno2;
#else
				Word64 norm;
				//Word32 tmpno1;
				//Word32 tmpno2;
#endif
				tmp32no1 = lsl(((Complex16_t*) efk_buffer)[i].real, blk_expno3);
				tmp32no2 = lsl(((Complex16_t*) efk_buffer)[i].imag, blk_expno3);
				xr = st->XFm[j][i].real;
				xi = st->XFm[j][i].imag; // (a-jb) * (c+jd)
#if !defined(FIXED_POINT)

#if 0
						tmpno1 = (Float32) tmp32no1 / tmp64no1;
						tmpno2 = (Float32) tmp32no2 / tmp64no1;
#else
						tmpno1 = tmp32no1 / powf(2, tmp32);
						tmpno2 = tmp32no2 / powf(2, tmp32);
#endif
						norm = powf(tmpno1, 2);
						norm += powf(tmpno2, 2);
						norm = sqrtf(norm);

						if (norm > thresh) {
							tmpno1 /= norm;
							tmpno2 /= norm;
							tmpno1 *= thresh;
							tmpno2 *= thresh;
						}

						yr = tmpno1 * xr + tmpno2 * xi;
						yi = tmpno2 * xr - tmpno1 * xi;
						(E2k[j] + i)->real = yr * 32767;
						(E2k[j] + i)->imag = yi * 32767;

#else
				norm = (Word64) tmp32no1 * tmp32no1;
				norm += (Word64) tmp32no2 * tmp32no2;
				//TODO: unsigned long long sqrt implementations
				//norm = sqrtf(norm);
				norm = usqrt_ll(norm);
				//fwrite(&norm, sizeof(Word64), 1, dptr_i);

				if (norm >= ((tmp64no1 + 0x20000) >> 18)) { // norm(|xr+j*xi|/P) >= thresh
					register Word32 tmp32;
					tmp32 = ulog2(norm);
					yr = (Word64) tmp32no1 * xr + (Word64) tmp32no2 * xi;
					yi = (Word64) tmp32no2 * xr - (Word64) tmp32no1 * xi;
					yr = (Word64) yr >> tmp32;
					yi = (Word64) yi >> tmp32;
					(E2k[j] + i)->real = (Word64) yr >> 3;
					(E2k[j] + i)->imag = (Word64) yi >> 3;

				} else {
					yr = (Word64) tmp32no1 * xr + (Word64) tmp32no2 * xi;
					yi = (Word64) tmp32no2 * xr - (Word64) tmp32no1 * xi;
					if (tmp32 > 15) {
						yr = (Word64) yr >> (tmp32 - 15);
						yi = (Word64) yi >> (tmp32 - 15);
					} else {
						yr = (Word64) yr << (15 - tmp32);
						yi = (Word64) yr << (15 - tmp32);
					}

					(E2k[j] + i)->real = yr;
					(E2k[j] + i)->imag = yi;
				}
#endif

			} else {
				if (!i) { // dc
#if 0
				Word32 tmpno1;

				tmp32no1 = lsl(((Complex16_t*) efk_buffer)[0].real,
						blk_expno3);
				xr = st->XFm[j][0].real;

				if (ABS(tmp32no1) > (tmp64no1 >> 18)) { // norm((xr+jxi)/P) > thresh?
					yr = xr >> 3;
				} else {
					if (tmp32 > 15) {
						tmpno1 = tmp32no1 >> (tmp32 - 15);
						yr = (Word32) tmpno1 * xr;
					} else {
						tmpno1 = lsl(tmp32no1, (15 - tmp32));
						yr = (Word32) tmpno1 * xr;
					}
				}

				E2k[j]->real = yr;
#endif
					E2k[j]->real = 0;

				} else { // nyquist, nFFT/2
#if 0
				Word32 tmpno1;

				tmp32no1 = lsl(((Complex16_t*) efk_buffer)[0].imag,
						blk_expno3);
				xr = st->XFm[j][nFFT / 2].real;

				if (ABS(tmp32no1) > (tmp64no1 >> 18)) {
					yr = xr >> 3;
				} else {
					if (tmp32 > 15) {
						tmpno1 = tmp32no1 >> (tmp32 - 15);
						yr = (Word32) tmpno1 * xr;
					} else {
						tmpno1 = lsl(tmp32no1, (15 - tmp32));
						yr = (Word32) tmpno1 * xr;
					}
				}
				E2k[j]->imag = yr;
#endif
					E2k[j]->imag = 0;
				}
			}
		} while (--j >= 0);
	}

	j = M - 1;
	do {
		Word32 tmp32;
		tmp32 = norm2Word16(E2k[j], nFFT);
		i = nFFT / 2 - 1;
		do {
			((Complex16_t*) efk_buffer)[i].real = asr((Word32) E2k[j][i].real,
					tmp32);
			((Complex16_t*) efk_buffer)[i].imag = asr((Word32) E2k[j][i].imag,
					tmp32);
		} while (--i >= 0);

		hs_ifft_128((Complex16_t*) efk_buffer, efk_buffer);
		memset(efk_buffer + 64, 0, sizeof(Word16) * nFFT / 2);
		blk_expno3 = rfft_128((Complex16_t*) efk_buffer);
		i = nFFT / 2; // NLMS update
		do {
			if (i && i != nFFT / 2) {
#if 0
				st->WFb[j][i].real += ((Complex16_t*) efk_buffer)[i].real
				* powf(2, blk_expno3 + tmp32 - 1);
				st->WFb[j][i].imag += ((Complex16_t*) efk_buffer)[i].imag
				* powf(2, blk_expno3 + tmp32 - 1);
#else
				(st->WFb[j] + i)->real += lsl(
						((Complex16_t *) efk_buffer + i)->real,
						blk_expno3 + tmp32 - 1);
				(st->WFb[j] + i)->imag += lsl(
						((Complex16_t *) efk_buffer + i)->imag,
						blk_expno3 + tmp32 - 1);
#endif
			} else {
				if (i) {
					(st->WFb[j] + nFFT / 2)->real += lsl(
							((Complex16_t*) efk_buffer)[0].imag,
							blk_expno3 + tmp32 - 1);
					st->WFb[j][nFFT / 2].imag = 0;
				} else {
					st->WFb[j]->real += lsl(
							((Complex16_t *) efk_buffer)[0].real,
							blk_expno3 + tmp32 - 1);
					st->WFb[j][0].imag = 0;
				}
			}
		} while (--i >= 0);

	} while (--j >= 0);

	memmove(st->ee_buffer, &st->ee_buffer[nFFT / 2], sizeof(Word16) * nFFT / 2);
	memmove(&st->ee_buffer[nFFT / 2], en_buffer, sizeof(Word16) * nFFT / 2); // windowed error
	memmove(st->yy_buffer, &st->yy_buffer[nFFT / 2], sizeof(Word16) * nFFT / 2);
	memmove(&st->yy_buffer[nFFT / 2], yn_buffer, sizeof(Word16) * nFFT / 2); // windowed echo
	memmove(st->dd_buffer, &st->dd_buffer[nFFT / 2], sizeof(Word16) * nFFT / 2);
	memmove(&st->dd_buffer[nFFT / 2], Sin, sizeof(Word16) * nFFT / 2); // windowed near_end

}

void PBFDAF_RAES(FDAF_STATE *ft, NLP_STATE *st, Word16 *ee, Word16 *dd,
		Word16 *yy, Word16 *out) { // error, mic, est_echo

	Word32 i;

	Word16 en[nFFT]; // 1+nFFT/2
	Word16 dn[nFFT];
	Word16 yn[nFFT];
	Word16 randv[nFFT / 2 - 1]; // excludes dc

	Word32 tgain[1 + nFFT / 2];
	Word32 blk_expno1;
	Word32 blk_expno2;
	Word32 blk_expno3;
	Word64 acc64no1 = 0;
	Word64 acc64no2 = 0;

	Complex16_t Rb[nFFT / 2];
	Complex16_t tn[nFFT / 2];
	Word64 ps[1 + nFFT / 2];
	Word64 mask[1 + nFFT / 2];

	/*sqrt Windowing*/
	for (i = 0; i < nFFT; i += 1) {
		en[i] = L_mult(ee[i], sqrt_hann_128[i]) >> 16;
		yn[i] = L_mult(yy[i], sqrt_hann_128[i]) >> 16;
		dn[i] = L_mult(dd[i], sqrt_hann_128[i]) >> 16;
	}

	blk_expno1 = rfft_128((ComplexInt16 *) en); // a+jb
	blk_expno2 = rfft_128((ComplexInt16 *) yn); // c+jd
	blk_expno3 = rfft_128((ComplexInt16 *) dn); // e-jf

	for (i = 0; i < 1 + nFFT / 2; i += 1) {
		register Word32 tmp32no1;
		register Word32 tmp32no2;
		register Word32 gain;
		register Word32 gain_1;
		Word64 tmp64no1;
		Word64 tmp64no2;
		Word64 tmp64no3;

		if (i && i != nFFT / 2) {
			st->Sed[i] -= st->Sed[i] >> 3;
			tmp64no1 = ((ComplexInt16 *) en + i)->real
					* ((ComplexInt16 *) dn + i)->real;
			tmp64no1 += ((ComplexInt16 *) en + i)->imag
					* ((ComplexInt16*) dn + i)->imag;
			tmp64no1 <<= (blk_expno1 + blk_expno3);
			st->Sed[i] += tmp64no1 >> 3; // Sed(e.^jw)

			st->See[i] -= st->See[i] >> 3;
			tmp64no1 = ((ComplexInt16 *) en + i)->real
					* ((ComplexInt16 *) en + i)->real;
			tmp64no1 += ((ComplexInt16 *) en + i)->imag
					* ((ComplexInt16*) en + i)->imag;
			tmp64no1 <<= (2 * blk_expno1);
			st->See[i] += tmp64no1 >> 3; // See(e.^jw)

			st->Sdd[i] -= st->Sdd[i] >> 3;
			tmp64no1 = ((ComplexInt16 *) dn + i)->real
					* ((ComplexInt16 *) dn + i)->real;
			tmp64no1 += ((ComplexInt16 *) dn + i)->imag
					* ((ComplexInt16*) dn + i)->imag;
			tmp64no1 <<= 2 * blk_expno3;
			st->Sdd[i] += tmp64no1 >> 3; // Sdd(e.^jw)

			tmp64no1 = min(st->Sdd[i], st->Sym[i]);
			tmp64no1 -= tmp64no1 >> 2;
			st->Sym[i] >>= 2;
			st->Sym[i] += tmp64no1;
			st->Sym[i] += st->Sym[i] >> 8; // Sym(e.^jw)

			st->Syy[i] -= st->Syy[i] >> 3;
			tmp64no1 = ((ComplexInt16 *) yn + i)->real
					* ((ComplexInt16 *) yn + i)->real;
			tmp64no1 += ((ComplexInt16 *) yn + i)->imag
					* ((ComplexInt16*) yn + i)->imag;
			tmp64no1 <<= 2 * blk_expno2;
			st->Syy[i] += tmp64no1 >> 3; // Syy(e.^jw)

			tmp32no1 = L_add(L_mult(st->Gainno1[i], st->Gainno1[i]), 0x8000)
					>> 16; // Q15-gain
			tmp32no1 = L_mult(tmp32no1, st->gammano1[i]); // Q16
			tmp32no1 -= tmp32no1 >> 5; // aa.*Gain.^2.*post_snr(k,l-1)

			tmp32no2 = min(udiv_64_64(1 + st->See[i], st->Sdd[i]), 8192); // post-snr, Sdd[k]./See[k]
			tmp32no1 += L_mult(max(tmp32no2 - 1, 0), 32767) >> 5; // Q16, priori-snr
			st->Gainno1[i] = udiv_64_32(tmp32no1 + 65536, tmp32no1); // (1-aa).*max(post_snr(k,l)-1, 0)
			st->gammano1[i] = tmp32no2;
			Rb[i].real = sat_32_16(
					asr(L_mult(st->Gainno1[i], ((Complex16_t *) dn)[i].real),
							16 - blk_expno3)
							- lsl(((Complex16_t *) yn)[i].real, blk_expno2));
			Rb[i].imag = sat_32_16(
					asr(L_mult(st->Gainno1[i], ((Complex16_t *) dn)[i].imag),
							16 - blk_expno3)
							- lsl(((Complex16_t *) yn)[i].imag, blk_expno2));
			tmp64no3 = L_mult(Rb[i].real, Rb[i].real); // residual echo power-spectral
			st->lambda[i] = asr(L_mac(tmp64no3, Rb[i].imag, Rb[i].imag), 1);

			tmp64no1 = alphaS * st->Syy[i];
			tmp64no2 = tmp64no1;
			tmp64no1 += st->Sed[i];
			gain = min(ABS(sdiv_64_64(1 + tmp64no1, st->Sed[i] << 15)), 32767);
			tmp64no2 += st->Sdd[i];
			gain_1 = min(ABS(sdiv_64_64(1 + tmp64no2, st->Sed[i] << 15)),
					32767);
			gain = L_mult(gain_1, gain) >> 16; // gain_all

			tn[i].real = L_mult(gain, ((Complex16_t *) en)[i].real) >> 16;
			tn[i].imag = L_mult(gain, ((Complex16_t *) en)[i].imag) >> 16;
			ps[i] = L_mac(0, tn[i].real, tn[i].real);
			ps[i] = L_mac(ps[i], tn[i].imag, tn[i].imag);
			ps[i] <<= 2 * blk_expno1;

		} else {
			if (!i) {
				((Complex16_t *) tn)[0].real = 0; // dc
				st->lambda[0] = 0;
				Rb[0].real = 0;
				ps[0] = 0;
			} else {
				((Complex16_t *) tn)[0].imag = 0; // nyquist
				st->lambda[nFFT / 2] = 0;
				Rb[0].imag = 0;
				ps[nFFT / 2] = 0;
			}
		}
	}

	acc64no1 = 0;
	acc64no2 = 0;

	for (i = 1; i < nFFT / 2; i += 1) {
		acc64no1 += st->See[i];
		acc64no2 += st->Sdd[i];
	}

	if (st->diverge_state == 0) {
		if (acc64no1 > acc64no2) {
			st->diverge_state = 1;
			memmove(en, dn, sizeof(Word16) * nFFT);
			blk_expno1 = blk_expno3;
		}
	} else {
		if ((acc64no1 += acc64no1 >> 4) < acc64no2) {
			st->diverge_state = 0;
		} else {
			memmove(en, dn, sizeof(Word16) * nFFT);
			blk_expno1 = blk_expno3;
		}
	}

	if (acc64no1 > acc64no2 << 4) { // out-off ctrls
		for (i = 0; i < M; i += 1) {
			memset(ft->WFb[i], 0, sizeof(ComplexInt32) * (1 + nFFT / 2));
		}
	}

	mask_thresh(ps, mask, nFFT, CB_FREQ_INDICES); // double talk relief ??

	for (i = 0; i < 1 + nFFT / 2; i += 1) {
		if (i && i != nFFT / 2) {
			Word32 register gain;

			gain = min(
					usqrt(2 * udiv_128_64(1 + 1.47f * st->lambda[i], mask[i])),
					255); // Q8
			// FIXME : OverScaling
			gain = L_mult(32767, lsl(gain, 7)) >> 16; // Q15
			st->Gainno2[i] = L_mult(gain, gain) >> 16; // perceptual-masked gain

			((Complex16_t *) tn)[i].real = L_add(
					L_mult(gain, ((Complex16_t *) en)[i].real), 0x10000) >> 17;
			((Complex16_t *) tn)[i].imag = L_add(
					L_mult(gain, ((Complex16_t *) en)[i].imag), 0x10000) >> 17;
		} else {
			if (!i) {
				((Complex16_t *) tn)[0].real = 0;
				st->Gainno2[0] = 0;
			} else {
				((Complex16_t *) tn)[0].imag = 0;
				st->Gainno2[nFFT / 2] = 0;
			}
		}
	}

	acc64no1 = 0;
	acc64no2 = 0;

	for (i = FREQ_BIN_LOW; i < FREQ_BIN_HIGH; i += 1) { // BandRange
		acc64no1 += tn[i].real * tn[i].real;
		acc64no1 += tn[i].imag * tn[i].imag;
		acc64no2 += ((Complex16_t *) en)[i].real * ((Complex16_t *) en)[i].real;
		acc64no2 += ((Complex16_t *) en)[i].imag * ((Complex16_t *) en)[i].imag;
	}

	if (acc64no1 > (acc64no2 >> 3)) { // attenuation more than 6 dB, 10*log10(4)=6.02dB
		st->timer = 4;
		memmove(tgain, st->Gainno2, sizeof(Word32) * (1 + nFFT / 2));

	} else {
		if (st->timer > 0) {
			st->timer -= 1;
			memmove(tgain, st->Gainno2, sizeof(Word32) * (1 + nFFT / 2));
		} else {
			memset(tgain, 0, sizeof(Word32) * (1 + nFFT / 2));
		}
	}

	for (i = 1; i < nFFT / 2; i += 1) {
		st->Gainno3[i] -= st->Gainno3[i] >> 2;
		st->Gainno3[i] += tgain[i] >> 2;
		((Complex16_t *) tn)[i].real = L_add(
				L_mult(st->Gainno3[i], ((Complex16_t *) en)[i].real), 0x10000)
				>> 17;
		((Complex16_t *) tn)[i].imag = L_add(
				L_mult(st->Gainno3[i], ((Complex16_t *) en)[i].imag), 0x10000)
				>> 17;
	}

// TODO : Comfort Noise Generations

	WebRtcSpl_RandUArray(randv, nFFT / 2 - 1, (uint32_t *) &st->seed);

	for (i = 1; i < nFFT / 2; i += 1) {
		Word16 vector[2];
		Word32 gain;
		Word64 gain_1;

		gain = L_add(L_mult(st->Gainno3[i], st->Gainno3[i]), 0x8000) >> 16; // Q15
		gain = L_sub(32767, gain); // aa.^2.*2.^15
		gain = usqrt(2 * gain); // Q8
		gain_1 = usqrt_ll(1.66f * st->Sym[i]);
		gain_1 = gain_1 * gain; // Q8

		oscillator_core(randv[i - 1] << 17, vector, sin_table, cos_table); // Q14

		Rb[i].real = gain_1 * vector[0] >> 22;
		Rb[i].imag = gain_1 * vector[1] >> 22;
		((ComplexInt16 *) tn)[i].real += Rb[i].real >> (1 + blk_expno1);
		((ComplexInt16 *) tn)[i].imag -= Rb[i].imag >> (1 + blk_expno1);
	}
	Rb[0].real = 0; // dc
	Rb[0].imag = 0; // nyquist

	hs_ifft_128((Complex16_t *) tn, (Word16 *) en);

	for (i = 0; i < nFFT; i += 1) {
//Word16 *ptr = (Word16 *) Rb;
		en[i] = sat_32_16(
				asr(L_mult(en[i], sqrt_hann_128[i]), 15 - blk_expno1));
//ptr[i] = asr(L_mult(ptr[i], sqrt_hann_128[i]), 16);
	}
	for (i = 0; i < nFFT / 2; i += 1) {
//Word16 *ptr = (Word16 *) Rb;
		*out++ = L_add(lsl(en[i], 16), lsl(st->buffer[i], 16)) >> 16;
		st->buffer[i] = en[i + nFFT / 2];
//*out++ = ptr[i] + st->buffer[i];
//st->buffer[i] = ptr[i + nFFT / 2];
	}
}

void PAES_Process_Block(AecmCore * __restrict aecm,
		const Word16 * __restrict Sin, const Word16 * __restrict Rin,
		Word16 *output) {

	Word16 fft_buffer[PART_LEN4 + 2 + 16];
	Word16 *fft = (Word16*) (((uintptr_t) fft_buffer + 31) & ~31); // align to 32 bytes

	Word32 efw_buf[PART_LEN2 + 8];
	ComplexInt16 *efw = (ComplexInt16*) (((uintptr_t) efw_buf + 31) & ~31);

	Word32 dfw_buf[PART_LEN2 + 8];
	ComplexInt16 *dfw = (ComplexInt16*) (((uintptr_t) dfw_buf + 31) & ~31);

	Word16 *ifft_out = (Word16*) efw;
	UWord16 hnl[PART_LEN1];
	UWord16 xfa[PART_LEN1]; //nyquist
	UWord16 dfaNoisy[PART_LEN1];
	UWord16 gfa[BARK_LEN];
	UWord32 efa[BARK_LEN]; //error
	UWord32 echoEst32[BARK_LEN]; //Estimated
	UWord32 nfa[BARK_LEN];

	Word16 tmp16no1;
	Word32 i, j;
	Word32 tmp32no1;
	Word32 tmp32no2;
	Word32 tmp32no3;
	Word32 tmp32no4;
	Word32 time_signal_scaling = 0;
	Word32 zerosDBufNoisy = 0;

	Word64 tmp64no1;
	Word64 tmp64no2;

	memmove(aecm->xBuf + PART_LEN, Rin, sizeof(Word16) * PART_LEN);
	memmove(aecm->dBufNoisy + PART_LEN, Sin, sizeof(Word16) * PART_LEN);

	tmp32no2 = INT_MAX;
	tmp32no3 = INT_MAX;

	for (i = 0; i < 2 * PART_LEN; i += 1) {
		tmp32no1 = CLZ(labs(aecm->xBuf[i] << 16));
		tmp32no2 = (tmp32no2 > tmp32no1) ? tmp32no1 : tmp32no2;
		tmp32no1 = CLZ(labs(aecm->dBufNoisy[i] << 16));
		tmp32no3 = (tmp32no3 > tmp32no1) ? tmp32no1 : tmp32no3;
	}

//assert(tmp32no2 > 0);
	time_signal_scaling = (tmp32no2 > 0) ? tmp32no2 - 1 : tmp32no2; // norm
	zerosDBufNoisy = (tmp32no3 > 0) ? tmp32no3 - 1 : tmp32no3;

//printf("%d\n", time_signal_scaling);
//printf("%d\n", zerosDBufNoisy);

	for (i = 0; i < PART_LEN; i += 1) {
		Word16 scaled_time_signal = aecm->xBuf[i] << time_signal_scaling;
		fft[i] = L_mult(scaled_time_signal, sqrt_hann_512[i]) >> 16;
		scaled_time_signal = aecm->xBuf[i + PART_LEN] << time_signal_scaling;
		fft[i + PART_LEN] = L_mult(scaled_time_signal,
				sqrt_hann_512[PART_LEN - i]) >> 16;
	}

	for (i = 0; i < PART_LEN; i += 1) {
		Word16 scaled = aecm->dBufNoisy[i] << zerosDBufNoisy;
		((Word16*) dfw)[i] = L_mult(scaled, sqrt_hann_512[i]) >> 16;
		scaled = aecm->dBufNoisy[i + PART_LEN] << zerosDBufNoisy;
		((Word16*) dfw)[i + PART_LEN] = L_mult(scaled,
				sqrt_hann_512[PART_LEN - i]) >> 16;
	}

	tmp32no1 = rfft_512((ComplexInt16*) fft, 512);
	tmp32no4 = tmp32no3 = rfft_512((ComplexInt16*) dfw, 512);

	for (i = 1; i < PART_LEN; i += 1) {
		if (((ComplexInt16*) fft)[i].real == 0)
			xfa[i] = labs(((ComplexInt16*) fft)[i].imag);
		else if (((ComplexInt16*) fft)[i].imag == 0)
			xfa[i] = labs(((ComplexInt16*) fft)[i].real);
		else {
			tmp32no2 = ((Complex16_t*) fft)[i].real
					* ((Complex16_t*) fft)[i].real;
			tmp32no2 += ((Complex16_t*) fft)[i].imag
					* ((Complex16_t*) fft)[i].imag;
			tmp32no2 = usqrt(tmp32no2);
			xfa[i] = (UWord16) tmp32no2;
		}

		if (dfw[i].real == 0)
			//dfaNoisy[i] = asr(labs(dfw[i].imag), zerosDBufNoisy - tmp32no3);
			dfaNoisy[i] = labs(dfw[i].imag);
		else if (dfw[i].imag == 0)
			//dfaNoisy[i] = asr(labs(dfw[i].real), zerosDBufNoisy - tmp32no3);
			dfaNoisy[i] = labs(dfw[i].real);
		else {
			tmp32no2 = dfw[i].real * dfw[i].real;
			tmp32no2 += dfw[i].imag * dfw[i].imag;
			tmp32no2 = usqrt(tmp32no2);
			//dfaNoisy[i] = (UWord16) asr(tmp32no2, zerosDBufNoisy - tmp32no3);
			dfaNoisy[i] = (UWord16) tmp32no2;
		}
	}

	xfa[0] = labs(fft[0]); //dc
	xfa[PART_LEN] = labs(fft[1]); //nyquist
//dfaNoisy[0] = asr(labs(dfw[0].real), zerosDBufNoisy - tmp32no3);
//dfaNoisy[PART_LEN] = asr(labs(dfw[0].imag), zerosDBufNoisy - tmp32no3);
	dfaNoisy[0] = labs(dfw[0].real);
	dfaNoisy[PART_LEN] = labs(dfw[0].imag);

//fwrite(dfaNoisy, sizeof(UWord16), PART_LEN1, dptr_ii);

	for (i = 0; i < Q - 1; i += 1) {
		memmove(aecm->XFm[i], aecm->XFm[i + 1], sizeof(UWord32) * BARK_LEN);
	}

	tmp32no2 = ToBARKScale[0];
	tmp64no1 = 0;
	tmp64no2 = 0;
	for (i = 0; i < PART_LEN; i += 1) {
		tmp64no1 += xfa[i] * xfa[i];
		tmp64no2 += dfaNoisy[i] * dfaNoisy[i];
		if (tmp32no2 != ToBARKScale[i + 1]) {
			tmp64no1 = usqrt_ll(tmp64no1); // SQRT(|Xd[i,k]|.^2)
			tmp64no2 = usqrt_ll(tmp64no2); // SQRT(|Y[i,k]|.^2)
			aecm->XFm[Q - 1][tmp32no2] = asr(tmp64no1,
					time_signal_scaling - tmp32no1);
			efa[tmp32no2] = asr(tmp64no2, zerosDBufNoisy - tmp32no3); // initial error
			tmp64no1 = 0; //Sxy(i,k)=alpha*Sxy(i-1,k)+(1-alpha)*|Xd[i,k]||efa[k]|
			tmp64no2 = 0;
			tmp32no2 = ToBARKScale[i + 1];
		}
	}

	tmp64no1 += xfa[PART_LEN] * xfa[PART_LEN];
	tmp64no1 = usqrt_ll(tmp64no1);
	aecm->XFm[Q - 1][BARK_LEN - 1] = asr(tmp64no1,
			time_signal_scaling - tmp32no1);

	tmp64no2 += dfaNoisy[PART_LEN] * dfaNoisy[PART_LEN];
	tmp64no2 = usqrt_ll(tmp64no2);
	efa[BARK_LEN - 1] = asr(tmp64no2, zerosDBufNoisy - tmp32no3);
	memcpy(nfa, efa, sizeof(UWord32) * BARK_LEN);

//fwrite(aecm->XFm[Q - 1], sizeof(UWord32), BARK_LEN, dptr_ii);
	for (i = 0; i < Q - 1; i += 1) {
		memmove(aecm->Sxx[i], aecm->Sxx[i + 1], sizeof(UWord64) * BARK_LEN);
	}

	for (i = 0; i < BARK_LEN; i += 1) {
		tmp64no1 = (Word64) aecm->XFm[Q - 1][i] * aecm->XFm[Q - 1][i];
		tmp64no2 = aecm->Sxx[Q - 1][i] - tmp64no1;
		tmp32no2 = CLZ64(llabs(tmp64no2));
		if (tmp32no2 < 10) {
			tmp64no2 >>= 15;
			tmp64no1 = tmp64no2 * 655;
		} else {
			tmp64no1 = tmp64no2 * 655 >> 15;
		}
		aecm->Sxx[Q - 1][i] -= tmp64no1;
	}

//fwrite(aecm->Sxx[Q - 1], sizeof(UWord64), BARK_LEN, dptr_ii);
//fwrite(efa, sizeof(Word32), BARK_LEN, dptr_ii);
	memset(echoEst32, 0, sizeof(UWord32) * BARK_LEN);

	for (i = 0; i < Q; i += 1) { //Step Number
		for (j = 0; j < BARK_LEN; j += 1) {
			tmp64no1 = (Word64) efa[j] * aecm->XFm[Q - 1 - i][j];
			tmp64no2 = aecm->Sxy[i][j] - tmp64no1;
			tmp32no2 = CLZ64(llabs(tmp64no2));
			if (tmp32no2 < 10) {
				tmp64no2 >>= 15;
				tmp64no1 = tmp64no2 * 655;
			} else {
				tmp64no1 = tmp64no2 * 655 >> 15;
			}
			aecm->Sxy[i][j] -= tmp64no1;
			tmp32no2 = CLZ64(tmp64no1 = aecm->Sxy[i][j]);
			tmp32no3 = CLZ64(tmp64no2 = max(1uLL, aecm->Sxx[Q - 1 - i][j]));
			tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;
			tmp32no3 = (tmp32no3 > 0) ? tmp32no3 - 1 : 0;
			tmp64no1 <<= tmp32no2;
			tmp64no2 <<= tmp32no3;

			if (tmp64no1 > tmp64no2) {
				tmp64no1 -= tmp64no2;
				tmp64no1 = udiv_128_64(tmp64no2, tmp64no1);
				tmp64no1 += 32767;
			} else {
				tmp64no1 = udiv_128_64(tmp64no2, tmp64no1);
			}

			tmp64no1 = asr(tmp64no1, tmp32no2 - tmp32no3); //Q15
			tmp64no2 = tmp64no1 * aecm->XFm[Q - 1 - i][j] >> 15;

			efa[j] = max(efa[j] - tmp64no2, 0);
			echoEst32[j] += tmp64no2;
		}
	}

	memset(hnl, 0, sizeof(UWord16) * PART_LEN1);

	for (i = 0; i < BARK_LEN; i += 1) {
		Word32 register qDomainDiff;
		Word32 register tmp32no1;
		Word32 register tmp32no4;

		tmp32no2 = CLZ(aecm->nearFilt[i]);
		tmp32no2 = (tmp32no2 > 0) ? tmp32no2 - 1 : 0;
		//assert(tmp32no2 >= 0);
		tmp32no3 = CLZ(nfa[i]);
		tmp32no3 = (tmp32no3 > 0) ? tmp32no3 - 1 : 0;
		//assert(tmp32no3 >= 0);
		if (tmp32no2 < tmp32no3) {
			qDomainDiff = tmp32no2;
			tmp32no1 = aecm->nearFilt[i] << tmp32no2;
			tmp32no4 = nfa[i] << tmp32no2;
		} else {
			qDomainDiff = tmp32no3;
			tmp32no1 = aecm->nearFilt[i] << tmp32no3;
			tmp32no4 = nfa[i] << tmp32no3;
		}
		tmp32no4 = (tmp32no1 - tmp32no4) >> 4;
		tmp32no1 -= tmp32no4;
		aecm->nearFilt[i] = tmp32no1 >>= qDomainDiff;
	}

	for (i = 0; i < BARK_LEN; i += 1) {
		Word32 register tmp32no1;
		tmp32no1 = echoEst32[i] - aecm->echoFilt[i];
		aecm->echoFilt[i] += tmp32no1 * 50 >> 8;
	}

	for (i = 0; i < BARK_LEN; i += 1) {
		tmp32no2 = CLZ64(tmp64no1 = echoEst32[i]);
		tmp32no3 = CLZ64(tmp64no2 = max(1uLL, nfa[i]));
		tmp32no2 = tmp32no2 > 0 ? tmp32no2 - 1 : 0;
		tmp32no3 = tmp32no3 > 0 ? tmp32no3 - 1 : 0;
		tmp64no1 <<= tmp32no2;
		tmp64no2 <<= tmp32no3;
		if (tmp64no1 < tmp64no2) {
			tmp64no1 = udiv_128_64(tmp64no2, tmp64no1);
		} else {
			tmp64no1 -= tmp64no2;
			tmp64no1 = udiv_128_64(tmp64no2, tmp64no1);
			tmp64no1 += 32767;
		}
		//tmp64no1 *= 2;
		tmp64no1 = asr(tmp64no1, tmp32no2 - tmp32no3);
		gfa[i] = max(32767 - tmp64no1, 0); // G(n,k)
	}

//fwrite(echoEst32, sizeof(Word32), BARK_LEN, dptr_ii);

	for (i = 0; i < PART_LEN1; i += 1) {
		hnl[i] = gfa[ToBARKScale[i]];
	}

	for (i = 0; i < PART_LEN1; i += 1) {
		dfw[i].real = L_mult((dfw + i)->real, hnl[i]) >> 16;
		dfw[i].imag = L_mult((dfw + i)->imag, hnl[i]) >> 16;
	}

	hs_ifft_512((ComplexInt16*) dfw, ifft_out, 512);

	for (i = 0; i < PART_LEN; i += 1) {
		ifft_out[i] = L_mult(ifft_out[i], sqrt_hann_512[i]) >> 16;
		tmp32no2 = asr(ifft_out[i], zerosDBufNoisy - tmp32no4);
		output[i] = sat_32_16(tmp32no2 + aecm->outBuf[i]);

		tmp32no2 = L_mult(ifft_out[i + PART_LEN], sqrt_hann_512[PART_LEN - i])
				>> 16;
		aecm->outBuf[i] = sat_32_16(asr(tmp32no2, zerosDBufNoisy - tmp32no4));
	}

	memmove(aecm->xBuf, aecm->xBuf + PART_LEN, sizeof(Word16) * PART_LEN);
	memmove(aecm->dBufNoisy, aecm->dBufNoisy + PART_LEN,
			sizeof(Word16) * PART_LEN);

}
