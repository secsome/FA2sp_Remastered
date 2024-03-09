#include "StdAfx.h"

#include "LZO.h"

#pragma warning(disable : 4244)

#ifndef NDEBUG
#define NDEBUG
#endif
#include <assert.h>

#if !defined(LZO1X) && !defined(LZO1Y)
#  define LZO1X
#endif


/***********************************************************************
//
************************************************************************/

#define M1_MAX_OFFSET	0x0400
#if defined(LZO1X)
#define M2_MAX_OFFSET	0x0800
#elif defined(LZO1Y)
#define M2_MAX_OFFSET	0x0400
#endif
#define M3_MAX_OFFSET	0x4000
#define M4_MAX_OFFSET	0xbfff

#define MX_MAX_OFFSET	(M1_MAX_OFFSET + M2_MAX_OFFSET)

#define M1_MARKER		0
#define M2_MARKER		64
#define M3_MARKER		32
#define M4_MARKER		16


#define _DV2(p,shift1,shift2) \
		(((( (lzo_uint)(p[2]) << shift1) ^ p[1]) << shift2) ^ p[0])
#define DVAL_NEXT(dv,p) \
		dv ^= p[-1]; dv = (((dv) >> 5) ^ ((lzo_uint)(p[2]) << (2*5)))
#define _DV(p,shift) 		_DV2(p,shift,shift)
#define DVAL_FIRST(dv,p)	dv = _DV((p),5)
#define _DINDEX(dv,p)		((40799u * (dv)) >> 5)
#define DINDEX(dv,p)		(((_DINDEX(dv,p)) & 0x3fff) << 0)
#define UPDATE_D(dict,cycle,dv,p)		dict[ DINDEX(dv,p) ] = (p)
#define UPDATE_I(dict,cycle,index,p)	dict[index] = (p)


/***********************************************************************
// compress a block of data.
************************************************************************/

static int do_compress(const lzo_byte* in, lzo_uint  in_len,
	lzo_byte* out, lzo_uint* out_len,
	lzo_voidp wrkmem)
{
	const lzo_byte* ip;
	lzo_uint dv;
	lzo_byte* op;
	const lzo_byte* const in_end = in + in_len;
	const lzo_byte* const ip_end = in + in_len - 9 - 4;
	const lzo_byte* ii;
	const lzo_bytepp const dict = (const lzo_bytepp)wrkmem;

	op = out;
	ip = in;
	ii = ip;

	DVAL_FIRST(dv, ip); UPDATE_D(dict, cycle, dv, ip); ip++;
	DVAL_NEXT(dv, ip);  UPDATE_D(dict, cycle, dv, ip); ip++;
	DVAL_NEXT(dv, ip);  UPDATE_D(dict, cycle, dv, ip); ip++;
	DVAL_NEXT(dv, ip);  UPDATE_D(dict, cycle, dv, ip); ip++;

	while (1) {
		const lzo_byte* m_pos;
		lzo_uint m_len;
		lzo_ptrdiff_t m_off;
		lzo_uint lit;

		lzo_uint dindex = DINDEX(dv, ip);
		m_pos = dict[dindex];
		UPDATE_I(dict, cycle, dindex, ip);


		if (LZO_CHECK_MPOS_NON_DET(m_pos, m_off, in, ip, M4_MAX_OFFSET)) {
		}
#if defined(LZO_UNALIGNED_OK_2)
		else
			if (*(unsigned short*)m_pos != *(unsigned short*)ip)
#else
		else
			if (m_pos[0] != ip[0] || m_pos[1] != ip[1])
#endif
			{
			}
			else {
				if (m_pos[2] == ip[2]) {
					lit = ip - ii;
					m_pos += 3;
					if (m_off <= M2_MAX_OFFSET)
						goto match;

					/* better compression, but slower */
					if (lit == 3) {
						assert(op - 2 > out); op[-2] |= LZO_BYTE(3);
						*op++ = *ii++; *op++ = *ii++; *op++ = *ii++;
						goto code_match;
					}

					if (*m_pos == ip[3]) {
						goto match;
					}
				}
				else {
					/* still need a better way for finding M1 matches */
				}
			}


		/* a literal */
		++ip;
		if (ip >= ip_end) {
			break;
		}
		DVAL_NEXT(dv, ip);
		continue;


		/* a match */

	match:

		/* store current literal run */
		if (lit > 0) {
			lzo_uint t = lit;

			if (t <= 3) {
				assert(op - 2 > out);
				op[-2] |= LZO_BYTE(t);
			}
			else {
				if (t <= 18) {
					*op++ = LZO_BYTE(t - 3);
				}
				else {
					lzo_uint tt = t - 18;

					*op++ = 0;
					while (tt > 255) {
						tt -= 255;
						*op++ = 0;
					}
					assert(tt > 0);
					*op++ = LZO_BYTE(tt);
				}
			}

			do {
				*op++ = *ii++;
			} while (--t > 0);
		}


		/* code the match */
	code_match:
		assert(ii == ip);
		ip += 3;
		if (*m_pos++ != *ip++ || *m_pos++ != *ip++ || *m_pos++ != *ip++ ||
			*m_pos++ != *ip++ || *m_pos++ != *ip++ || *m_pos++ != *ip++)
		{
			--ip;
			m_len = ip - ii;
			assert(m_len >= 3); assert(m_len <= 8);

			if (m_off <= M2_MAX_OFFSET) {
				m_off -= 1;
				*op++ = LZO_BYTE(((m_len - 1) << 5) | ((m_off & 7) << 2));
				*op++ = LZO_BYTE(m_off >> 3);
			}
			else {
				if (m_off <= M3_MAX_OFFSET) {
					m_off -= 1;
					*op++ = LZO_BYTE(M3_MARKER | (m_len - 2));
					goto m3_m4_offset;
				}
				else {
					m_off -= 0x4000;
					assert(m_off > 0); assert(m_off <= 0x7fff);
					*op++ = LZO_BYTE(M4_MARKER |
						((m_off & 0x4000) >> 11) | (m_len - 2));
					goto m3_m4_offset;
				}
			}
		}
		else {
			const lzo_byte* end;
			end = in_end;
			while (ip < end && *m_pos == *ip) {
				m_pos++;
				ip++;
			}
			m_len = (ip - ii);
			assert(m_len >= 3);

			if (m_off <= M3_MAX_OFFSET) {
				m_off -= 1;
				if (m_len <= 33) {
					*op++ = LZO_BYTE(M3_MARKER | (m_len - 2));
				}
				else {
					m_len -= 33;
					*op++ = M3_MARKER | 0;
					goto m3_m4_len;
				}
			}
			else {
				m_off -= 0x4000;
				assert(m_off > 0); assert(m_off <= 0x7fff);
				if (m_len <= 9) {
					*op++ = LZO_BYTE(M4_MARKER |
						((m_off & 0x4000) >> 11) | (m_len - 2));
				}
				else {
					m_len -= 9;
					*op++ = LZO_BYTE(M4_MARKER | ((m_off & 0x4000) >> 11));
				m3_m4_len:
					while (m_len > 255) {
						m_len -= 255;
						*op++ = 0;
					}
					assert(m_len > 0);
					*op++ = LZO_BYTE(m_len);
				}
			}

		m3_m4_offset:
			*op++ = LZO_BYTE((m_off & 63) << 2);
			*op++ = LZO_BYTE(m_off >> 6);
		}

		ii = ip;
		if (ip >= ip_end) {
			break;
		}
		DVAL_FIRST(dv, ip);
	}

	/* store final literal run */
	if (in_end - ii > 0) {
		lzo_uint t = in_end - ii;

		if (op == out && t <= 238) {
			*op++ = LZO_BYTE(17 + t);
		}
		else {
			if (t <= 3) {
				op[-2] |= LZO_BYTE(t);
			}
			else {
				if (t <= 18) {
					*op++ = LZO_BYTE(t - 3);
				}
				else {
					lzo_uint tt = t - 18;

					*op++ = 0;
					while (tt > 255) {
						tt -= 255;
						*op++ = 0;
					}
					assert(tt > 0);
					*op++ = LZO_BYTE(tt);
				}
			}
		}
		do {
			*op++ = *ii++;
		} while (--t > 0);
	}

	*out_len = op - out;
	return LZO_E_OK;
}


/***********************************************************************
// public entry point
************************************************************************/

int lzo1x_1_compress(const lzo_byte* in, lzo_uint  in_len,
	lzo_byte* out, lzo_uint* out_len,
	lzo_voidp wrkmem)
{
	lzo_byte* op = out;
	int r = LZO_E_OK;

	if (in_len <= 0)
		*out_len = 0;
	else if (in_len <= 9 + 4)
	{
		*op++ = LZO_BYTE(17 + in_len);
		do *op++ = *in++; while (--in_len > 0);
		*out_len = op - out;
	}
	else
		r = do_compress(in, in_len, out, out_len, wrkmem);

	if (r == LZO_E_OK)
	{
		op = out + *out_len;
		*op++ = M4_MARKER | 1;
		*op++ = 0;
		*op++ = 0;
		*out_len += 3;
	}

	return r;
}

#if 1
#  define TEST_IP				1
#else
#  define TEST_IP				(ip < ip_end)
#endif

/***********************************************************************
// decompress a block of data.
************************************************************************/

int lzo1x_decompress(const lzo_byte* in, lzo_uint  in_len,
	lzo_byte* out, lzo_uint* out_len,
	lzo_voidp)
{
	lzo_byte* op;
	const lzo_byte* ip;
	lzo_uint t;
	const lzo_byte* m_pos;
	const lzo_byte* const ip_end = in + in_len;

	*out_len = 0;

	op = out;
	ip = in;

	if (*ip > 17)
	{
		t = *ip++ - 17;
		goto first_literal_run;
	}

	while (TEST_IP)
	{
		t = *ip++;
		if (t >= 16)
			goto match;
		/* a literal run */
		if (t == 0)
		{
			t = 15;
			while (*ip == 0)
				t += 255, ip++;
			t += *ip++;
		}
		/* copy literals */
		*op++ = *ip++; *op++ = *ip++; *op++ = *ip++;
	first_literal_run:
		do *op++ = *ip++; while (--t > 0);


		t = *ip++;

		if (t >= 16)
			goto match;
#if defined(LZO1X)
		m_pos = op - 1 - 0x800;
#elif defined(LZO1Y)
		m_pos = op - 1 - 0x400;
#endif
		m_pos -= t >> 2;
		m_pos -= *ip++ << 2;
		*op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos++;
		goto match_done;


		/* handle matches */
		while (TEST_IP)
		{
			if (t < 16)						/* a M1 match */
			{
				m_pos = op - 1;
				m_pos -= t >> 2;
				m_pos -= *ip++ << 2;
				*op++ = *m_pos++; *op++ = *m_pos++;
			}
			else
			{
			match:
				if (t >= 64)				/* a M2 match */
				{
					m_pos = op - 1;
#if defined(LZO1X)
					m_pos -= (t >> 2) & 7;
					m_pos -= *ip++ << 3;
					t = (t >> 5) - 1;
#elif defined(LZO1Y)
					m_pos -= (t >> 2) & 3;
					m_pos -= *ip++ << 2;
					t = (t >> 4) - 3;
#endif
				}
				else if (t >= 32)			/* a M3 match */
				{
					t &= 31;
					if (t == 0)
					{
						t = 31;
						while (*ip == 0)
							t += 255, ip++;
						t += *ip++;
					}
					m_pos = op - 1;
					m_pos -= *ip++ >> 2;
					m_pos -= *ip++ << 6;
				}
				else						/* a M4 match */
				{
					m_pos = op;
					m_pos -= (t & 8) << 11;
					t &= 7;
					if (t == 0)
					{
						t = 7;
						while (*ip == 0)
							t += 255, ip++;
						t += *ip++;
					}
					m_pos -= *ip++ >> 2;
					m_pos -= *ip++ << 6;
					if (m_pos == op)
						goto eof_found;
					m_pos -= 0x4000;
				}
				*op++ = *m_pos++; *op++ = *m_pos++;
				do *op++ = *m_pos++; while (--t > 0);
			}

		match_done:
			t = ip[-2] & 3;
			if (t == 0)
				break;
			/* copy literals */
			do *op++ = *ip++; while (--t > 0);
			t = *ip++;
		}
	}

	/* ip == ip_end and no EOF code was found */

	//Unreachable - ST 9/5/96 5:07PM
	//*out_len = op - out;
	//return (ip == ip_end ? LZO_E_EOF_NOT_FOUND : LZO_E_ERROR);

eof_found:
	assert(t == 1);
	*out_len = op - out;
	return (ip == ip_end ? LZO_E_OK : LZO_E_ERROR);
}