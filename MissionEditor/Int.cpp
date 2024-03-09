#include "StdAfx.h"

#include "Int.h"

int BigInt::Error = 0;
bool BigInt::Carry = false;
bool BigInt::Borrow = false;
BigInt BigInt::Remainder;