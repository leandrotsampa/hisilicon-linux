/*
  * Copyright (c) 2016 MediaTek Inc.  All rights reserved.
  *
  * This software is available to you under a choice of one of two
  * licenses.  You may choose to be licensed under the terms of the GNU
  * General Public License (GPL) Version 2, available from the file
  * COPYING in the main directory of this source tree, or the
  * BSD license below:
  *
  *	Redistribution and use in source and binary forms, with or
  *	without modification, are permitted provided that the following
  *	conditions are met:
  *
  *	 - Redistributions of source code must retain the above
  *	   copyright notice, this list of conditions and the following
  *	   disclaimer.
  *
  *	 - Redistributions in binary form must reproduce the above
  *	   copyright notice, this list of conditions and the following
  *	   disclaimer in the documentation and/or other materials
  *	   provided with the distribution.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  */


/****************************************************************************
    Module Name:
    DH

    Abstract:
    RFC 2631: Diffie-Hellman Key Agreement Method

    Revision History:
    Who		When		What
    --------	----------	------------------------------------------
    Eddy	2009/01/21	Create Diffie-Hellman
***************************************************************************/


#include "crypt_dh.h"
#include "crypt_biginteger.h"


/*
========================================================================
Routine Description:
    Diffie-Hellman public key generation

Arguments:
    GValue	     Array in UINT8
    GValueLength     The length of G in bytes
    PValue	     Array in UINT8
    PValueLength     The length of P in bytes
    PrivateKey	     Private key
    PrivateKeyLength The length of Private key in bytes

Return Value:
    PublicKey	    Public key
    PublicKeyLength The length of public key in bytes

Note:
    Reference to RFC2631
    PublicKey = G^PrivateKey (mod P)
========================================================================
*/
void DH_PublicKey_Generate (
    IN UINT8 GValue[],
    IN UINT GValueLength,
    IN UINT8 PValue[],
    IN UINT PValueLength,
    IN UINT8 PrivateKey[],
    IN UINT PrivateKeyLength,
    OUT UINT8 PublicKey[],
    INOUT UINT *PublicKeyLength)
{
    PBIG_INTEGER pBI_G = NULL;
    PBIG_INTEGER pBI_P = NULL;
    PBIG_INTEGER pBI_PrivateKey = NULL;
    PBIG_INTEGER pBI_PublicKey = NULL;

    /*
     * 1. Check the input parameters
     *	  - GValueLength, PValueLength and PrivateLength must be large than zero
     *	  - PublicKeyLength must be large or equal than PValueLength
     *	  - PValue must be odd
     *
     *	  - PValue must be prime number (no implement)
     *	  - GValue must be greater than 0 but less than the PValue (no implement)
     */
    if (GValueLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: G length is (%d)\n", GValueLength));
	return;
    } /* End of if */
    if (PValueLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: P length is (%d)\n", PValueLength));
	return;
    } /* End of if */
    if (PrivateKeyLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: private key length is (%d)\n", PrivateKeyLength));
	return;
    } /* End of if */
    if (*PublicKeyLength < PValueLength) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: public key length(%d) must be large or equal than P length(%d)\n",
	    *PublicKeyLength, PValueLength));
	return;
    } /* End of if */
    if (!(PValue[PValueLength - 1] & 0x1)) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: P value must be odd\n"));
	return;
    } /* End of if */

    /*
     * 2. Transfer parameters to BigInteger structure
     */
    BigInteger_Init(&pBI_G);
    BigInteger_Init(&pBI_P);
    BigInteger_Init(&pBI_PrivateKey);
    BigInteger_Init(&pBI_PublicKey);
    BigInteger_Bin2BI(GValue, GValueLength, &pBI_G);
    BigInteger_Bin2BI(PValue, PValueLength, &pBI_P);
    BigInteger_Bin2BI(PrivateKey, PrivateKeyLength, &pBI_PrivateKey);

    /*
     * 3. Calculate PublicKey = G^PrivateKey (mod P)
     *	  - BigInteger Operation
     *	  - Montgomery reduction
     */
    BigInteger_Montgomery_ExpMod(pBI_G, pBI_PrivateKey, pBI_P, &pBI_PublicKey);

    /*
     * 4. Transfer BigInteger structure to char array
     */
    BigInteger_BI2Bin(pBI_PublicKey, PublicKey, PublicKeyLength);

    BigInteger_Free(&pBI_G);
    BigInteger_Free(&pBI_P);
    BigInteger_Free(&pBI_PrivateKey);
    BigInteger_Free(&pBI_PublicKey);
} /* End of DH_PublicKey_Generate */


/*
========================================================================
Routine Description:
    Diffie-Hellman secret key generation

Arguments:
    PublicKey	     Public key
    PublicKeyLength  The length of Public key in bytes
    PValue	     Array in UINT8
    PValueLength     The length of P in bytes
    PrivateKey	     Private key
    PrivateKeyLength The length of Private key in bytes

Return Value:
    SecretKey	     Secret key
    SecretKeyLength  The length of secret key in bytes

Note:
    Reference to RFC2631
    SecretKey = PublicKey^PrivateKey (mod P)
========================================================================
*/
void DH_SecretKey_Generate (
    IN UINT8 PublicKey[],
    IN UINT PublicKeyLength,
    IN UINT8 PValue[],
    IN UINT PValueLength,
    IN UINT8 PrivateKey[],
    IN UINT PrivateKeyLength,
    OUT UINT8 SecretKey[],
    INOUT UINT *SecretKeyLength)
{
    PBIG_INTEGER pBI_P = NULL;
    PBIG_INTEGER pBI_SecretKey = NULL;
    PBIG_INTEGER pBI_PrivateKey = NULL;
    PBIG_INTEGER pBI_PublicKey = NULL;

    /*
     * 1. Check the input parameters
     *	  - PublicKeyLength, PValueLength and PrivateLength must be large than zero
     *	  - SecretKeyLength must be large or equal than PValueLength
     *	  - PValue must be odd
     *
     *	  - PValue must be prime number (no implement)
     */
    if (PublicKeyLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: public key length is (%d)\n", PublicKeyLength));
	return;
    } /* End of if */
    if (PValueLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: P length is (%d)\n", PValueLength));
	return;
    } /* End of if */
    if (PrivateKeyLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: private key length is (%d)\n", PrivateKeyLength));
	return;
    } /* End of if */
    if (*SecretKeyLength < PValueLength) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: secret key length(%d) must be large or equal than P length(%d)\n",
	    *SecretKeyLength, PValueLength));
	return;
    } /* End of if */
    if (!(PValue[PValueLength - 1] & 0x1)) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: P value must be odd\n"));
	return;
    } /* End of if */

    /*
     * 2. Transfer parameters to BigInteger structure
     */
    BigInteger_Init(&pBI_P);
    BigInteger_Init(&pBI_PrivateKey);
    BigInteger_Init(&pBI_PublicKey);
    BigInteger_Init(&pBI_SecretKey);

    BigInteger_Bin2BI(PublicKey, PublicKeyLength, &pBI_PublicKey);
    BigInteger_Bin2BI(PValue, PValueLength, &pBI_P);
    BigInteger_Bin2BI(PrivateKey, PrivateKeyLength, &pBI_PrivateKey);

    /*
     * 3. Calculate SecretKey = PublicKey^PrivateKey (mod P)
     *	  - BigInteger Operation
     *	  - Montgomery reduction
     */
    BigInteger_Montgomery_ExpMod(pBI_PublicKey, pBI_PrivateKey, pBI_P, &pBI_SecretKey);

    /*
     * 4. Transfer BigInteger structure to char array
     */
    BigInteger_BI2Bin(pBI_SecretKey, SecretKey, SecretKeyLength);

    BigInteger_Free(&pBI_P);
    BigInteger_Free(&pBI_PrivateKey);
    BigInteger_Free(&pBI_PublicKey);
    BigInteger_Free(&pBI_SecretKey);
} /* End of DH_SecretKey_Generate */
