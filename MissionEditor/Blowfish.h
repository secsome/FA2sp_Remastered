#pragma once

class BlowfishEngine
{
public:
	explicit BlowfishEngine() noexcept;

	BlowfishEngine(const BlowfishEngine&) = delete;
	BlowfishEngine& operator=(const BlowfishEngine&) = delete;
	BlowfishEngine(BlowfishEngine&&) = default;
	BlowfishEngine& operator=(BlowfishEngine&&) = default;

	~BlowfishEngine();

	void Submit_Key(const void* key, int length);
	int Encrypt(const void* plaintext, int length, void* cyphertext);
	int Decrypt(const void* cyphertext, int length, void* plaintext);

	enum { MAX_KEY_LENGTH = 56 };

private:
	void Sub_Key_Encrypt(unsigned int& left, unsigned int& right);
	void Process_Block(const void* plaintext, void* cyphertext, const unsigned int* ptable);

private:
	enum
	{
		ROUNDS = 16,
		BYTES_PER_BLOCK = 8
	};

	bool IsKeyed;
	unsigned int P_Encrypt[(int)ROUNDS + 2];
	unsigned int P_Decrypt[(int)ROUNDS + 2];
	unsigned int bf_S[4][UCHAR_MAX + 1];

	static unsigned int const P_Init[(int)ROUNDS + 2];
	static unsigned int const S_Init[4][UCHAR_MAX + 1];
};