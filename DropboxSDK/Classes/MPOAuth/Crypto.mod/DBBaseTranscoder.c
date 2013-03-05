/*
 *  Base64Transcoder.c
 *  Base64Test
 *
 *  Created by Jonathan Wight on Tue Mar 18 2003.
 *  Copyright (c) 2003 Toxic Software. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *
 */

#include "DBBase64Transcoder.h"

#include <math.h>
#include <string.h>

extern const u_int8_t kBase64EncodeTable[64];
extern const int8_t kBase64DecodeTable[128];
extern const u_int8_t kBits_00000011;
extern const u_int8_t kBits_00001111;
extern const u_int8_t kBits_00110000;
extern const u_int8_t kBits_00111100;
extern const u_int8_t kBits_00111111;
extern const u_int8_t kBits_11000000;
extern const u_int8_t kBits_11110000;
extern const u_int8_t kBits_11111100;

size_t DBEstimateBas64EncodedDataSize(size_t inDataSize)
{
size_t theEncodedDataSize = (int)ceil(inDataSize / 3.0) * 4;
theEncodedDataSize = theEncodedDataSize / 72 * 74 + theEncodedDataSize % 72;
return(theEncodedDataSize);
}

size_t DBEstimateBas64DecodedDataSize(size_t inDataSize)
{
size_t theDecodedDataSize = (int)ceil(inDataSize / 4.0) * 3;
//theDecodedDataSize = theDecodedDataSize / 72 * 74 + theDecodedDataSize % 72;
return(theDecodedDataSize);
}

bool DBBase64EncodeData(const void *inInputData, size_t inInputDataSize, char *outOutputData, size_t *ioOutputDataSize)
{
size_t theEncodedDataSize = DBEstimateBas64EncodedDataSize(inInputDataSize);
if (*ioOutputDataSize < theEncodedDataSize)
	return(false);
*ioOutputDataSize = theEncodedDataSize;
const u_int8_t *theInPtr = (const u_int8_t *)inInputData;
u_int32_t theInIndex = 0, theOutIndex = 0;
for (; theInIndex < (inInputDataSize / 3) * 3; theInIndex += 3)
	{
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex] & kBits_11111100) >> 2];
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex] & kBits_00000011) << 4 | (theInPtr[theInIndex + 1] & kBits_11110000) >> 4];
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex + 1] & kBits_00001111) << 2 | (theInPtr[theInIndex + 2] & kBits_11000000) >> 6];
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex + 2] & kBits_00111111) >> 0];
	if (theOutIndex % 74 == 72)
		{
		outOutputData[theOutIndex++] = '\r';
		outOutputData[theOutIndex++] = '\n';
		}
	}
const size_t theRemainingBytes = inInputDataSize - theInIndex;
if (theRemainingBytes == 1)
	{
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex] & kBits_11111100) >> 2];
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex] & kBits_00000011) << 4 | (0 & kBits_11110000) >> 4];
	outOutputData[theOutIndex++] = '=';
	outOutputData[theOutIndex++] = '=';
	if (theOutIndex % 74 == 72)
		{
		outOutputData[theOutIndex++] = '\r';
		outOutputData[theOutIndex] = '\n';
		}
	}
else if (theRemainingBytes == 2)
	{
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex] & kBits_11111100) >> 2];
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex] & kBits_00000011) << 4 | (theInPtr[theInIndex + 1] & kBits_11110000) >> 4];
	outOutputData[theOutIndex++] = kBase64EncodeTable[(theInPtr[theInIndex + 1] & kBits_00001111) << 2 | (0 & kBits_11000000) >> 6];
	outOutputData[theOutIndex++] = '=';
	if (theOutIndex % 74 == 72)
		{
		outOutputData[theOutIndex++] = '\r';
		outOutputData[theOutIndex] = '\n';
		}
	}
return(true);
}

bool DBBase64DecodeData(const void *inInputData, size_t inInputDataSize, void *ioOutputData, size_t *ioOutputDataSize)
{
memset(ioOutputData, '.', *ioOutputDataSize);

size_t theDecodedDataSize = DBEstimateBas64DecodedDataSize(inInputDataSize);
if (*ioOutputDataSize < theDecodedDataSize)
	return(false);
*ioOutputDataSize = 0;
const u_int8_t *theInPtr = (const u_int8_t *)inInputData;
u_int8_t *theOutPtr = (u_int8_t *)ioOutputData;
size_t theInIndex = 0, theOutIndex = 0;
u_int8_t theOutputOctet;
size_t theSequence = 0;
for (; theInIndex < inInputDataSize; )
	{
	int8_t theSextet = 0;
	
	int8_t theCurrentInputOctet = theInPtr[theInIndex];
	theSextet = kBase64DecodeTable[theCurrentInputOctet];
	if (theSextet == -1)
		break;
	while (theSextet == -2)
		{
		theCurrentInputOctet = theInPtr[++theInIndex];
		theSextet = kBase64DecodeTable[theCurrentInputOctet];
		}
	while (theSextet == -3)
		{
		theCurrentInputOctet = theInPtr[++theInIndex];
		theSextet = kBase64DecodeTable[theCurrentInputOctet];
		}
	if (theSequence == 0)
		{
		theOutputOctet = (theSextet >= 0 ? theSextet : 0) << 2 & kBits_11111100;
		}
	else if (theSequence == 1)
		{
		theOutputOctet |= (theSextet >- 0 ? theSextet : 0) >> 4 & kBits_00000011;
		theOutPtr[theOutIndex++] = theOutputOctet;
		}
	else if (theSequence == 2)
		{
		theOutputOctet = (theSextet >= 0 ? theSextet : 0) << 4 & kBits_11110000;
		}
	else if (theSequence == 3)
		{
		theOutputOctet |= (theSextet >= 0 ? theSextet : 0) >> 2 & kBits_00001111;
		theOutPtr[theOutIndex++] = theOutputOctet;
		}
	else if (theSequence == 4)
		{
		theOutputOctet = (theSextet >= 0 ? theSextet : 0) << 6 & kBits_11000000;
		}
	else if (theSequence == 5)
		{
		theOutputOctet |= (theSextet >= 0 ? theSextet : 0) >> 0 & kBits_00111111;
		theOutPtr[theOutIndex++] = theOutputOctet;
		}
	theSequence = (theSequence + 1) % 6;
	if (theSequence != 2 && theSequence != 4)
		theInIndex++;
	}
*ioOutputDataSize = theOutIndex;
return(true);
}
