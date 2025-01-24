// SPDX-FileCopyrightText: 2021 Billow <billow.fun@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-only

#include "rz_util/rz_ebcdic.h"
#include "rz_util/rz_assert.h"

/**
 *  \file ebcdic.c
 * Support charsets:
 * 1. IBM037
 * 2. IBM290
 * 3. EBCDIC-UK
 * 4. EBCDIC-US
 * 5. EBCDIC-ES
 *
 * see:
 *  - https://www.ibm.com/docs/en/zos/2.3.0?topic=sets-coded-character-sorted-by-ccsid
 *  - https://www.compart.com/en/unicode/search?q=EBCDIC#char-sets
 *
 */

static const RzCodePoint ibm037_to_uni[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x27 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28-0x2f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x37 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x38-0x3f */
	0x20, 0xa0, 0xe2, 0xe4, 0xe0, 0xe1, 0xe3, 0xe5, /* 0x40-0x47 */
	0xe7, 0xf1, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c, /* 0x48-0x4f */
	0x26, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef, /* 0x50-0x57 */
	0xec, 0xdf, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0xac, /* 0x58-0x5f */
	0x2d, 0x2f, 0xc2, 0xc4, 0xc0, 0xc1, 0xc3, 0xc5, /* 0x60-0x67 */
	0xc7, 0xd1, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, /* 0x68-0x6f */
	0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd, 0xce, 0xcf, /* 0x70-0x77 */
	0xcc, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22, /* 0x78-0x7f */
	0xd8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 0x80-0x87 */
	0x68, 0x69, 0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1, /* 0x88-0x8f */
	0xb0, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, /* 0x90-0x97 */
	0x71, 0x72, 0xaa, 0xba, 0xe6, 0xb8, 0xc6, 0xa4, /* 0x98-0x9f */
	0xb5, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, /* 0xa0-0xa7 */
	0x79, 0x7a, 0xa1, 0xbf, 0xd0, 0xdd, 0xde, 0xae, /* 0xa8-0xaf */
	0x5e, 0xa3, 0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc, /* 0xb0-0xb7 */
	0xbd, 0xbe, 0x5b, 0x5d, 0xaf, 0xa8, 0xb4, 0xd7, /* 0xb8-0xbf */
	0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, /* 0xc0-0xc7 */
	0x48, 0x49, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5, /* 0xc8-0xcf */
	0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, /* 0xd0-0xd7 */
	0x51, 0x52, 0xb9, 0xfb, 0xfc, 0xf9, 0xfa, 0xff, /* 0xd8-0xdf */
	0x5c, 0xf7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, /* 0xe0-0xe7 */
	0x59, 0x5a, 0xb2, 0xd4, 0xd6, 0xd2, 0xd3, 0xd5, /* 0xe8-0xef */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 0xf0-0xf7 */
	0x38, 0x39, 0xb3, 0xdb, 0xdc, 0xd9, 0xda, 0x7f, /* 0xf8-0xff */
};

static const ut8 ibm037_from_uni[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, /* 0x20-0x27 */
	0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61, /* 0x28-0x2f */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, /* 0x30-0x37 */
	0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f, /* 0x38-0x3f */
	0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, /* 0x40-0x47 */
	0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, /* 0x48-0x4f */
	0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, /* 0x50-0x57 */
	0xe7, 0xe8, 0xe9, 0xba, 0xe0, 0xbb, 0xb0, 0x6d, /* 0x58-0x5f */
	0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 0x60-0x67 */
	0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, /* 0x68-0x6f */
	0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, /* 0x70-0x77 */
	0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0xff, /* 0x78-0x7f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80-0x87 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
	0x41, 0xaa, 0x4a, 0xb1, 0x9f, 0xb2, 0x6a, 0xb5, /* 0xa0-0xa7 */
	0xbd, 0xb4, 0x9a, 0x8a, 0x5f, 0xca, 0xaf, 0xbc, /* 0xa8-0xaf */
	0x90, 0x8f, 0xea, 0xfa, 0xbe, 0xa0, 0xb6, 0xb3, /* 0xb0-0xb7 */
	0x9d, 0xda, 0x9b, 0x8b, 0xb7, 0xb8, 0xb9, 0xab, /* 0xb8-0xbf */
	0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9e, 0x68, /* 0xc0-0xc7 */
	0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77, /* 0xc8-0xcf */
	0xac, 0x69, 0xed, 0xee, 0xeb, 0xef, 0xec, 0xbf, /* 0xd0-0xd7 */
	0x80, 0xfd, 0xfe, 0xfb, 0xfc, 0xad, 0xae, 0x59, /* 0xd8-0xdf */
	0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9c, 0x48, /* 0xe0-0xe7 */
	0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57, /* 0xe8-0xef */
	0x8c, 0x49, 0xcd, 0xce, 0xcb, 0xcf, 0xcc, 0xe1, /* 0xf0-0xf7 */
	0x70, 0xdd, 0xde, 0xdb, 0xdc, 0x8d, 0x8e, 0xdf, /* 0xf8-0xff */
};

static const RzCodePoint ibm290_to_uni[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x27 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28-0x2f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x37 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x38-0x3f */
	0x20, 0x3002, 0x300c, 0x300d, 0x3001, 0x30fb, 0x30f2, 0x30a1, /* 0x40-0x47 */
	0x30a3, 0x30a5, 0xa3, 0x2e, 0x3c, 0x28, 0x2b, 0x7c, /* 0x48-0x4f */
	0x26, 0x30a7, 0x30a9, 0x30e3, 0x30e5, 0x30e7, 0x30c3, 0x00, /* 0x50-0x57 */
	0x30fc, 0x00, 0x21, 0xa5, 0x2a, 0x29, 0x3b, 0xac, /* 0x58-0x5f */
	0x2d, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x60-0x67 */
	0x00, 0x00, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, /* 0x68-0x6f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */
	0x00, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22, /* 0x78-0x7f */
	0x00, 0x30a2, 0x30a4, 0x30a6, 0x30a8, 0x30aa, 0x30ab, 0x30ad, /* 0x80-0x87 */
	0x30af, 0x30b1, 0x30b3, 0x00, 0x30b5, 0x30b7, 0x30b9, 0x30bb, /* 0x88-0x8f */
	0x30bd, 0x30bf, 0x30c1, 0x30c4, 0x30c6, 0x30c8, 0x30ca, 0x30cb, /* 0x90-0x97 */
	0x30cc, 0x30cd, 0x30ce, 0x00, 0x00, 0x30cf, 0x30d2, 0x30d5, /* 0x98-0x9f */
	0x00, 0xaf, 0x30d8, 0x30db, 0x30de, 0x30df, 0x30e0, 0x30e1, /* 0xa0-0xa7 */
	0x30e2, 0x30e4, 0x30e6, 0x00, 0x30e8, 0x30e9, 0x30ea, 0x30eb, /* 0xa8-0xaf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
	0x00, 0x00, 0x30ec, 0x30ed, 0x30ef, 0x30f3, 0x309b, 0x309c, /* 0xb8-0xbf */
	0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, /* 0xc0-0xc7 */
	0x48, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
	0x00, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, /* 0xd0-0xd7 */
	0x51, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
	0x24, 0x00, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, /* 0xe0-0xe7 */
	0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 0xf0-0xf7 */
	0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, /* 0xf8-0xff */
};

static const ut8 ibm290_page00[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x40, 0x5a, 0x7f, 0x7b, 0xe0, 0x6c, 0x50, 0x7d, /* 0x20-0x27 */
	0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61, /* 0x28-0x2f */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, /* 0x30-0x37 */
	0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f, /* 0x38-0x3f */
	0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, /* 0x40-0x47 */
	0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, /* 0x48-0x4f */
	0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, /* 0x50-0x57 */
	0xe7, 0xe8, 0xe9, 0x00, 0x00, 0x00, 0x00, 0x6d, /* 0x58-0x5f */
	0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x60-0x67 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x68-0x6f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */
	0x00, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x00, 0xff, /* 0x78-0x7f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80-0x87 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
	0x00, 0x00, 0x00, 0x4a, 0x00, 0x5b, 0x6a, 0x00, /* 0xa0-0xa7 */
	0x00, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0xa1, /* 0xa8-0xaf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc0-0xc7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd0-0xd7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe0-0xe7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf0-0xf7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf8-0xff */
};

static const ut8 ibm290_page30[256] = {
	[0x01] = 0x44,
	[0x02] = 0x41,
	[0x0c] = 0x42,
	[0x0d] = 0x43,
	[0xfb] = 0x45,
	[0xf2] = 0x46,
	[0xa1] = 0x47,
	[0xa3] = 0x48,
	[0xa5] = 0x49,
	[0xa7] = 0x51,
	[0xa9] = 0x52,
	[0xe3] = 0x53,
	[0xe5] = 0x54,
	[0xe7] = 0x55,
	[0xc3] = 0x56,
	[0xfc] = 0x58,
	[0xa2] = 0x81,
	[0xa4] = 0x82,
	[0xa6] = 0x83,
	[0xa8] = 0x84,
	[0xaa] = 0x85,
	[0xab] = 0x86,
	[0xad] = 0x87,
	[0xaf] = 0x88,
	[0xb1] = 0x89,
	[0xb3] = 0x8a,
	[0xb5] = 0x8c,
	[0xb7] = 0x8d,
	[0xb9] = 0x8e,
	[0xbb] = 0x8f,
	[0xbd] = 0x90,
	[0xbf] = 0x91,
	[0xc1] = 0x92,
	[0xc4] = 0x93,
	[0xc6] = 0x94,
	[0xc8] = 0x95,
	[0xca] = 0x96,
	[0xcb] = 0x97,
	[0xcc] = 0x98,
	[0xcd] = 0x99,
	[0xce] = 0x9a,
	[0xcf] = 0x9d,
	[0xd2] = 0x9e,
	[0xd5] = 0x9f,
	[0xd8] = 0xa2,
	[0xdb] = 0xa3,
	[0xde] = 0xa4,
	[0xdf] = 0xa5,
	[0xe0] = 0xa6,
	[0xe1] = 0xa7,
	[0xe2] = 0xa8,
	[0xe4] = 0xa9,
	[0xe6] = 0xaa,
	[0xe8] = 0xac,
	[0xe9] = 0xad,
	[0xea] = 0xae,
	[0xeb] = 0xaf,
	[0xec] = 0xba,
	[0xed] = 0xbb,
	[0xef] = 0xbc,
	[0xf3] = 0xbd,
	[0x9b] = 0xbe,
	[0x9c] = 0xbf, /* 0xb8-0xbf */

};

static const RzCodePoint ebcdic_uk_to_uni[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x27 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28-0x2f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x37 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x38-0x3f */
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x40-0x47 */
	0x00, 0x00, 0x24, 0x2e, 0x3c, 0x28, 0x2b, 0x7c, /* 0x48-0x4f */
	0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x50-0x57 */
	0x00, 0x00, 0x21, 0xa3, 0x2a, 0x29, 0x3b, 0xac, /* 0x58-0x5f */
	0x2d, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x60-0x67 */
	0x00, 0x00, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, /* 0x68-0x6f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */
	0x00, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22, /* 0x78-0x7f */
	0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 0x80-0x87 */
	0x68, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
	0x00, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, /* 0x90-0x97 */
	0x71, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
	0x00, 0xaf, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, /* 0xa0-0xa7 */
	0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa8-0xaf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */
	0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, /* 0xc0-0xc7 */
	0x48, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
	0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, /* 0xd0-0xd7 */
	0x51, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
	0x5c, 0x00, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, /* 0xe0-0xe7 */
	0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 0xf0-0xf7 */
	0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, /* 0xf8-0xff */
};

static const ut8 ebcdic_uk_from_uni[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x40, 0x5a, 0x7f, 0x7b, 0x4a, 0x6c, 0x50, 0x7d, /* 0x20-0x27 */
	0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61, /* 0x28-0x2f */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, /* 0x30-0x37 */
	0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f, /* 0x38-0x3f */
	0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, /* 0x40-0x47 */
	0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, /* 0x48-0x4f */
	0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, /* 0x50-0x57 */
	0xe7, 0xe8, 0xe9, 0x00, 0xe0, 0x00, 0x00, 0x6d, /* 0x58-0x5f */
	0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 0x60-0x67 */
	0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, /* 0x68-0x6f */
	0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, /* 0x70-0x77 */
	0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0x00, 0xff, /* 0x78-0x7f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80-0x87 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
	0x00, 0x00, 0x00, 0x5b, 0x00, 0x00, 0x6a, 0x00, /* 0xa0-0xa7 */
	0x00, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0xa1, /* 0xa8-0xaf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc0-0xc7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd0-0xd7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe0-0xe7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf0-0xf7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf8-0xff */
};

static const RzCodePoint ebcdic_us_to_uni[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x27 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28-0x2f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x37 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x38-0x3f */
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x40-0x47 */
	0x00, 0x00, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c, /* 0x48-0x4f */
	0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x50-0x57 */
	0x00, 0x00, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0xac, /* 0x58-0x5f */
	0x2d, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x60-0x67 */
	0x00, 0x00, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, /* 0x68-0x6f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */
	0x00, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22, /* 0x78-0x7f */
	0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 0x80-0x87 */
	0x68, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
	0x00, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, /* 0x90-0x97 */
	0x71, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
	0x00, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, /* 0xa0-0xa7 */
	0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa8-0xaf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */
	0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, /* 0xc0-0xc7 */
	0x48, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
	0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, /* 0xd0-0xd7 */
	0x51, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
	0x5c, 0x00, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, /* 0xe0-0xe7 */
	0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 0xf0-0xf7 */
	0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, /* 0xf8-0xff */
};

static const ut8 ebcdic_us_from_uni[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, /* 0x20-0x27 */
	0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61, /* 0x28-0x2f */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, /* 0x30-0x37 */
	0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f, /* 0x38-0x3f */
	0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, /* 0x40-0x47 */
	0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, /* 0x48-0x4f */
	0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, /* 0x50-0x57 */
	0xe7, 0xe8, 0xe9, 0x00, 0xe0, 0x00, 0x00, 0x6d, /* 0x58-0x5f */
	0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 0x60-0x67 */
	0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, /* 0x68-0x6f */
	0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, /* 0x70-0x77 */
	0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0xff, /* 0x78-0x7f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80-0x87 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
	0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x6a, 0x00, /* 0xa0-0xa7 */
	0x00, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, /* 0xa8-0xaf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc0-0xc7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd0-0xd7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe0-0xe7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf0-0xf7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf8-0xff */
};

static const RzCodePoint ebcdic_es_to_uni[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x27 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28-0x2f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x37 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x38-0x3f */
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x40-0x47 */
	0x00, 0x00, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c, /* 0x48-0x4f */
	0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x50-0x57 */
	0x00, 0x00, 0x21, 0x20a7, 0x2a, 0x29, 0x3b, 0xac, /* 0x58-0x5f */
	0x2d, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x60-0x67 */
	0x00, 0x00, 0xf1, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, /* 0x68-0x6f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */
	0x00, 0x60, 0x3a, 0xd1, 0x40, 0x27, 0x3d, 0x22, /* 0x78-0x7f */
	0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 0x80-0x87 */
	0x68, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
	0x00, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, /* 0x90-0x97 */
	0x71, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
	0x00, 0xa8, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, /* 0xa0-0xa7 */
	0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa8-0xaf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */
	0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, /* 0xc0-0xc7 */
	0x48, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
	0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, /* 0xd0-0xd7 */
	0x51, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
	0x5c, 0x00, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, /* 0xe0-0xe7 */
	0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 0xf0-0xf7 */
	0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, /* 0xf8-0xff */
};

static const ut8 ebcdic_es_page00[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 0x00-0x07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 0x08-0x0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 0x10-0x17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 0x18-0x1f */
	0x40, 0x5a, 0x7f, 0x00, 0x00, 0x6c, 0x50, 0x7d, /* 0x20-0x27 */
	0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61, /* 0x28-0x2f */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, /* 0x30-0x37 */
	0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f, /* 0x38-0x3f */
	0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, /* 0x40-0x47 */
	0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, /* 0x48-0x4f */
	0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, /* 0x50-0x57 */
	0xe7, 0xe8, 0xe9, 0x00, 0xe0, 0x00, 0x00, 0x6d, /* 0x58-0x5f */
	0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 0x60-0x67 */
	0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, /* 0x68-0x6f */
	0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, /* 0x70-0x77 */
	0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0x00, 0xff, /* 0x78-0x7f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80-0x87 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
	0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa0-0xa7 */
	0xa1, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, /* 0xa8-0xaf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc0-0xc7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
	0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd0-0xd7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe0-0xe7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */
	0x00, 0x6a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf0-0xf7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf8-0xff */
};

static const ut8 ebcdic_es_page20[256] = {
	[0xa7] = 0x5b,
};

/**
 * \name IBM037
 * see https://www.compart.com/en/unicode/charsets/IBM037
 */
/// @{

/**
 * \brief Convert an ibm037 char into an unicode RzRune
 *
 * \param src ibm037 char
 * \param dst unicode RzRune
 * \retval 0 if \p dst is null
 * \retval 1 if convert successful
 */
RZ_API int rz_str_ibm037_to_unicode(const ut8 src, RZ_NONNULL RZ_OUT RzCodePoint *dst) {
	rz_return_val_if_fail(dst, 0);
	*dst = ibm037_to_uni[src];
	return 1;
}

/**
 * \brief Convert an unicode RzRune into an ibm037 char
 *
 * \param dst ibm037 char
 * \param src unicode RzRune
 */
RZ_API int rz_str_ibm037_from_unicode(RZ_NONNULL RZ_OUT ut8 *dst, const RzCodePoint src) {
	rz_return_val_if_fail(dst, 0);
	if (src <= 0xff) {
		*dst = ibm037_from_uni[src];
		return 1;
	}
	return 0;
}

/**
 * \brief Convert an ibm037 char into an ascii char
 *
 * \param dst ibm037 char
 * \param src ascii char
 */
RZ_API int rz_str_ibm037_to_ascii(const ut8 src, RZ_NONNULL RZ_OUT ut8 *dst) {
	rz_return_val_if_fail(dst, 0);
	ut8 c = ibm037_to_uni[src];
	if (c < 0x80) {
		*dst = c;
		return 1;
	}
	return 0;
}

/**
 * \brief Convert an ascii char into an ibm037 char
 *
 * \param dst ibm037 char
 * \param src ascii char
 */
RZ_API int rz_str_ibm037_from_ascii(RZ_NONNULL RZ_OUT ut8 *dst, const ut8 src) {
	rz_return_val_if_fail(dst, 0);
	*dst = ibm037_from_uni[src];
	return 1;
}

/// @}

/**
 * \name IBM290
 * see https://www.compart.com/en/unicode/charsets/IBM290
 */

/// @{

/// Convert an ibm290 char into an unicode RzRune
RZ_API int rz_str_ibm290_to_unicode(const ut8 src, RZ_NONNULL RZ_OUT RzCodePoint *dst) {
	rz_return_val_if_fail(dst, 0);
	*dst = ibm290_to_uni[src];
	return 1;
}

/// Convert an unicode RzRune into an ibm290 char
RZ_API int rz_str_ibm290_from_unicode(RZ_NONNULL RZ_OUT ut8 *dst, const RzCodePoint src) {
	rz_return_val_if_fail(dst, 0);
	if (src <= 0xff) {
		*dst = ibm290_page00[src];
		return 1;
	} else if (src >= 0x3000 && src <= 0x30ff) {
		*dst = ibm290_page30[src & 0xff];
		return 1;
	}
	return 0;
}

/// Convert an ibm290 char into an ascii char
RZ_API int rz_str_ibm290_to_ascii(const ut8 src, RZ_NONNULL RZ_OUT ut8 *dst) {
	rz_return_val_if_fail(dst, 0);
	ut8 c = ibm290_to_uni[src];
	if (c < 0x80) {
		*dst = c;
		return 1;
	}
	return 0;
}

/// Convert an ascii char into an ibm290 char
RZ_API int rz_str_ibm290_from_ascii(RZ_NONNULL RZ_OUT ut8 *dst, const ut8 src) {
	rz_return_val_if_fail(dst, 0);
	*dst = ibm290_page00[src];
	return 1;
}

/// @}

/**
 * \name EBCDIC-UK
 * see https://www.compart.com/en/unicode/charsets/EBCDIC-UK
 */

/// @{

/// Convert an ebcdic_uk char into an unicode RzRune
RZ_API int rz_str_ebcdic_uk_to_unicode(const ut8 src, RZ_NONNULL RZ_OUT RzCodePoint *dst) {
	rz_return_val_if_fail(dst, 0);
	*dst = ebcdic_uk_to_uni[src];
	return 1;
}

/// Convert an unicode RzRune into an ebcdic_uk char
RZ_API int rz_str_ebcdic_uk_from_unicode(RZ_NONNULL RZ_OUT ut8 *dst, const RzCodePoint src) {
	rz_return_val_if_fail(dst, 0);
	if (src <= 0xff) {
		*dst = ebcdic_uk_from_uni[src];
		return 1;
	}
	return 0;
}

/// Convert an ebcdic_uk char into an ascii char
RZ_API int rz_str_ebcdic_uk_to_ascii(const ut8 src, RZ_NONNULL RZ_OUT ut8 *dst) {
	rz_return_val_if_fail(dst, 0);
	ut8 c = ebcdic_uk_to_uni[src];
	if (c < 0x80) {
		*dst = c;
		return 1;
	}
	return 0;
}

/// Convert an ascii char into an ebcdic_uk char
RZ_API int rz_str_ebcdic_uk_from_ascii(RZ_NONNULL RZ_OUT ut8 *dst, const ut8 src) {
	rz_return_val_if_fail(dst, 0);
	*dst = ebcdic_uk_from_uni[src];
	return 1;
}

/// @}

/**
 * \name EBCDIC-US
 * see https://www.compart.com/en/unicode/charsets/EBCDIC-US
 */

/// @{

/// Convert an ebcdic_us char into an unicode RzRune
RZ_API int rz_str_ebcdic_us_to_unicode(const ut8 src, RZ_NONNULL RZ_OUT RzCodePoint *dst) {
	rz_return_val_if_fail(dst, 0);
	*dst = ebcdic_us_to_uni[src];
	return 1;
}

/// Convert an unicode RzRune into an ebcdic_us char
RZ_API int rz_str_ebcdic_us_from_unicode(RZ_NONNULL RZ_OUT ut8 *dst, const RzCodePoint src) {
	rz_return_val_if_fail(dst, 0);
	if (src <= 0xff) {
		*dst = ebcdic_us_from_uni[src];
		return 1;
	}
	return 0;
}

/// Convert an ebcdic_us char into an ascii char
RZ_API int rz_str_ebcdic_us_to_ascii(const ut8 src, RZ_NONNULL RZ_OUT ut8 *dst) {
	rz_return_val_if_fail(dst, 0);
	ut8 c = ebcdic_us_to_uni[src];
	if (c < 0x80) {
		*dst = c;
		return 1;
	}
	return 0;
}

/// Convert an ascii char into an ebcdic_us char
RZ_API int rz_str_ebcdic_us_from_ascii(RZ_NONNULL RZ_OUT ut8 *dst, const ut8 src) {
	rz_return_val_if_fail(dst, 0);
	*dst = ebcdic_us_from_uni[src];
	return 1;
}

/// @}

/**
 * \name EBCDIC-ES
 * see https://www.compart.com/en/unicode/charsets/EBCDIC-ES
 */
/// @{

/// Convert an ebcdic_es char into an unicode RzRune
RZ_API int rz_str_ebcdic_es_to_unicode(const ut8 src, RZ_NONNULL RZ_OUT RzCodePoint *dst) {
	rz_return_val_if_fail(dst, 0);
	*dst = ebcdic_es_to_uni[src];
	return 1;
}

/// Convert an unicode RzRune into an ebcdic_es char
RZ_API int rz_str_ebcdic_es_from_unicode(RZ_NONNULL RZ_OUT ut8 *dst, const RzCodePoint src) {
	rz_return_val_if_fail(dst, 0);
	if (src <= 0xff) {
		*dst = ebcdic_es_page00[src];
		return 1;
	} else if (src >= 0x2000 && src <= 0x20ff) {
		*dst = ebcdic_es_page20[src & 0xff];
		return 1;
	}
	return 0;
}

/// Convert an ebcdic_es char into an ascii char
RZ_API int rz_str_ebcdic_es_to_ascii(const ut8 src, RZ_NONNULL RZ_OUT ut8 *dst) {
	rz_return_val_if_fail(dst, 0);
	ut8 c = ebcdic_es_to_uni[src];
	if (c < 0x80) {
		*dst = c;
		return 1;
	}
	return 0;
}

/// Convert an ascii char into an ebcdic char
RZ_API int rz_str_ebcdic_es_from_ascii(RZ_NONNULL RZ_OUT ut8 *dst, const ut8 src) {
	rz_return_val_if_fail(dst, 0);
	*dst = ebcdic_es_page00[src];
	return 1;
}

/// @}
