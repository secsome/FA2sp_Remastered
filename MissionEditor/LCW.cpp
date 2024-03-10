#include "StdAfx.h"

#include "LCW.h"

int LCW_Compress(void const* input, void* output, unsigned int size)
{
	bool relative = size > UINT16_MAX ? true : false;

	if (!size || !input || !output)
		return 0;

	const unsigned char* getp = static_cast<const unsigned char*>(input);
	unsigned char* putp = static_cast<unsigned char*>(output);
	const unsigned char* getstart = getp;
	const unsigned char* getend = getp + size;
	unsigned char* putstart = putp;

	if (relative)
		*putp++ = 0;

	unsigned char* cmd_onep = putp;
	*putp++ = 0x81;
	*putp++ = *getp++;
	bool cmd_one = true;

	while (getp < getend)
	{
		if (getend - getp > 64 && *getp == *(getp + 64))
		{
			const unsigned char* rlemax = (getend - getp) < UINT16_MAX ? getend : getp + UINT16_MAX;
			const unsigned char* rlep;

			for (rlep = getp + 1; *rlep == *getp && rlep < rlemax; ++rlep);

			unsigned short run_length = rlep - getp;
			if (run_length >= 0x41)
			{
				cmd_one = false;
				*putp++ = 0xFE;
				*putp++ = run_length;
				*putp++ = run_length >> 8;
				*putp++ = *getp;
				getp = rlep;
				continue;
			}
		}

		int block_size = 0;
		const unsigned char* offstart;

		if (relative)
			offstart = (getp - getstart) < UINT16_MAX ? getstart : getp - UINT16_MAX;
		else
			offstart = getstart;

		const unsigned char* offchk = offstart;
		const unsigned char* offsetp = getp;
		while (offchk < getp)
		{
			while (offchk < getp && *offchk != *getp)
				++offchk;

			if (offchk >= getp)
				break;

			int i;
			for (i = 1; &getp[i] < getend; ++i) {
				if (offchk[i] != getp[i])
					break;
			}

			if (i >= block_size)
			{
				block_size = i;
				offsetp = offchk;
			}
			++offchk;
		}

		if (block_size <= 2)
		{
			if (cmd_one && *cmd_onep < 0xBF)
			{
				++*cmd_onep;
				*putp++ = *getp++;
			}
			else
			{
				cmd_onep = putp;
				*putp++ = 0x81;
				*putp++ = *getp++;
				cmd_one = true;
			}
		}
		else
		{
			unsigned short offset;
			unsigned short rel_offset = getp - offsetp;
			if (block_size > 0xA || ((rel_offset) > 0xFFF))
			{
				if (block_size > 0x40) {
					*putp++ = 0xFF;
					*putp++ = block_size;
					*putp++ = block_size >> 8;
				}
				else
					*putp++ = (block_size - 3) | 0xC0;
				offset = relative ? rel_offset : offsetp - getstart;
			}
			else
				offset = rel_offset << 8 | (16 * (block_size - 3) + (rel_offset >> 8));
			*putp++ = offset;
			*putp++ = offset >> 8;
			getp += block_size;
			cmd_one = false;
		}
	}

	*putp++ = 0x80;
	return putp - putstart;
}

unsigned int LCW_Uncompress(void* source, void* dest)
{
	unsigned char* source_ptr, * dest_ptr, * copy_ptr, op_code, data;
	unsigned	  count, * word_dest_ptr, word_data;

	source_ptr = (unsigned char*)source;
	dest_ptr = (unsigned char*)dest;

	while (true)
	{
		op_code = *source_ptr++;
		if (!(op_code & 0x80))
		{
			count = (op_code >> 4) + 3;
			copy_ptr = dest_ptr - ((unsigned)*source_ptr++ + (((unsigned)op_code & 0x0f) << 8));
			while (count--) *dest_ptr++ = *copy_ptr++;
		}
		else
		{
			if (!(op_code & 0x40))
			{
				if (op_code == 0x80)
					return ((unsigned long)(dest_ptr - (unsigned char*)dest));
				else
				{
					count = op_code & 0x3f;
					while (count--) *dest_ptr++ = *source_ptr++;
				}
			}
			else
			{
				if (op_code == 0xfe)
				{
					count = *source_ptr + ((size_t)*(source_ptr + 1) << 8);
					word_data = data = *(source_ptr + 2);
					word_data = (word_data << 24) + (word_data << 16) + (word_data << 8) + word_data;
					source_ptr += 3;
					copy_ptr = dest_ptr + 4 - ((size_t)dest_ptr & 0x3);
					count -= (copy_ptr - dest_ptr);
					while (dest_ptr < copy_ptr) *dest_ptr++ = data;
					word_dest_ptr = (unsigned*)dest_ptr;
					dest_ptr += (count & 0xfffffffc);
					while (word_dest_ptr < (unsigned*)dest_ptr)
					{
						*word_dest_ptr = word_data;
						*(word_dest_ptr + 1) = word_data;
						word_dest_ptr += 2;
					}
					copy_ptr = dest_ptr + (count & 0x3);
					while (dest_ptr < copy_ptr) *dest_ptr++ = data;
				}
				else
				{
					if (op_code == 0xff)
					{
						count = *source_ptr + ((size_t)*(source_ptr + 1) << 8);
						copy_ptr = (unsigned char*)dest + *(source_ptr + 2) + ((size_t)*(source_ptr + 3) << 8);
						source_ptr += 4;
						while (count--) *dest_ptr++ = *copy_ptr++;
					}
					else
					{
						count = (op_code & 0x3f) + 3;
						copy_ptr = (unsigned char*)dest + *source_ptr + ((size_t)*(source_ptr + 1) << 8);
						source_ptr += 2;
						while (count--) *dest_ptr++ = *copy_ptr++;
					}
				}
			}
		}
	}
}