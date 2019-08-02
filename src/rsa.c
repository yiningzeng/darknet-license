/* RSA.C - RSA routines for RSAREF
 */

 /* Copyright (C) RSA Laboratories, a division of RSA Data Security,
      Inc., created 1991. All rights reserved.
  */

#include "rsa.h" 
#include <string.h>
#include <stdlib.h>
#define NN_DIGIT_BITS 32 
#define R_memset memset
#define R_memcpy memcpy
#define R_memcmp memcmp

static NN_DIGIT NN_AddDigitMult
(NN_DIGIT*, NN_DIGIT*, NN_DIGIT, NN_DIGIT*, unsigned int);
static NN_DIGIT NN_SubDigitMult
(NN_DIGIT*, NN_DIGIT*, NN_DIGIT, NN_DIGIT*, unsigned int);

static unsigned int NN_DigitBits(NN_DIGIT);

/* Decodes character string b into a, where character string is ordered
   from most to least significant.

   Lengths: a[digits], b[len].
   Assumes b[i] = 0 for i < len - digits * NN_DIGIT_LEN. (Otherwise most
   significant bytes are truncated.)
 */
void NN_Decode(NN_DIGIT* a,
    unsigned int digits,
    unsigned char* b,
    unsigned int len)

{
    NN_DIGIT t;
    int j;
    unsigned int i, u;

    for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
        t = 0;
        for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
            t |= ((NN_DIGIT)b[j]) << u;
        a[i] = t;
    }

    for (; i < digits; i++)
        a[i] = 0;
}

/* Encodes b into character string a, where character string is ordered
   from most to least significant.

   Lengths: a[len], b[digits].
   Assumes NN_Bits (b, digits) <= 8 * len. (Otherwise most significant
   digits are truncated.)
 */
void NN_Encode(unsigned char* a,
    unsigned int len,
    NN_DIGIT* b,
    unsigned int digits)

{
    NN_DIGIT t;
    int j;
    unsigned int i, u;

    for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
        t = b[i];
        for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
            a[j] = (unsigned char)(t >> u);
    }

    for (; j >= 0; j--)
        a[j] = 0;
}

/* Assigns a = b.

   Lengths: a[digits], b[digits].
 */
void NN_Assign(NN_DIGIT* a,
    NN_DIGIT* b,
    unsigned int digits)
{
    unsigned int i;

    for (i = 0; i < digits; i++)
        a[i] = b[i];
}

/* Assigns a = 0.

   Lengths: a[digits].
 */
void NN_AssignZero(NN_DIGIT* a,
    unsigned int digits)
{
    unsigned int i;

    for (i = 0; i < digits; i++)
        a[i] = 0;
}

/* Assigns a = 2^b.

   Lengths: a[digits].
   Requires b < digits * NN_DIGIT_BITS.
 */
void NN_Assign2Exp(NN_DIGIT* a,
    unsigned int b,
    unsigned int digits)
{
    NN_AssignZero(a, digits);

    if (b >= digits * NN_DIGIT_BITS)
        return;

    a[b / NN_DIGIT_BITS] = (NN_DIGIT)1 << (b % NN_DIGIT_BITS);
}

/* Computes a = b + c. Returns carry.

   Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Add(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT* c,
    unsigned int digits)
{
    NN_DIGIT ai, carry;
    unsigned int i;

    carry = 0;

    for (i = 0; i < digits; i++) {
        if ((ai = b[i] + carry) < carry)
            ai = c[i];
        else if ((ai += c[i]) < c[i])
            carry = 1;
        else
            carry = 0;
        a[i] = ai;
    }

    return (carry);
}

/* Computes a = b - c. Returns borrow.

   Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Sub(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT* c,
    unsigned int digits)
{
    NN_DIGIT ai, borrow;
    unsigned int i;

    borrow = 0;

    for (i = 0; i < digits; i++) {
        if ((ai = b[i] - borrow) > (MAX_NN_DIGIT - borrow))
            ai = MAX_NN_DIGIT - c[i];
        else if ((ai -= c[i]) > (MAX_NN_DIGIT - c[i]))
            borrow = 1;
        else
            borrow = 0;
        a[i] = ai;
    }

    return (borrow);
}


/* Returns sign of a - b.

   Lengths: a[digits], b[digits].
 */
int NN_Cmp(NN_DIGIT* a,
    NN_DIGIT* b,
    unsigned int digits)

{
    int i;

    for (i = digits - 1; i >= 0; i--) {
        if (a[i] > b[i])
            return (1);
        if (a[i] < b[i])
            return (-1);
    }

    return (0);
}

/* Returns nonzero iff a is zero.

   Lengths: a[digits].
 */
int NN_Zero(NN_DIGIT* a,
    unsigned int digits)
{
    unsigned int i;

    for (i = 0; i < digits; i++)
        if (a[i])
            return (0);

    return (1);
}

/* Returns the significant length of a in digits.

   Lengths: a[digits].
 */
unsigned int NN_Digits(NN_DIGIT* a,
    unsigned int digits)

{
    int i;

    for (i = digits - 1; i >= 0; i--)
        if (a[i])
            break;

    return (i + 1);
}
/* Returns the significant length of a in bits.

   Lengths: a[digits].
 */
unsigned int NN_Bits(NN_DIGIT* a,
    unsigned int digits)
{
    if ((digits = NN_Digits(a, digits)) == 0)
        return (0);

    return ((digits - 1) * NN_DIGIT_BITS + NN_DigitBits(a[digits - 1]));
}



/* Computes a = b * c.

   Lengths: a[2*digits], b[digits], c[digits].
   Assumes digits < MAX_NN_DIGITS.
 */
void NN_Mult(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT* c,
    unsigned int digits)
{
    NN_DIGIT t[2 * MAX_NN_DIGITS];
    unsigned int bDigits, cDigits, i;

    NN_AssignZero(t, 2 * digits);

    bDigits = NN_Digits(b, digits);
    cDigits = NN_Digits(c, digits);

    for (i = 0; i < bDigits; i++)
        t[i + cDigits] += NN_AddDigitMult(&t[i], &t[i], b[i], c, cDigits);

    NN_Assign(a, t, 2 * digits);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)t, 0, sizeof(t));
}

/* Computes a = b * 2^c (i.e., shifts left c bits), returning carry.

   Lengths: a[digits], b[digits].
   Requires c < NN_DIGIT_BITS.
 */
NN_DIGIT NN_LShift(NN_DIGIT* a,
    NN_DIGIT* b,
    unsigned int c,
    unsigned int digits)
{
    NN_DIGIT bi, carry;
    unsigned int i, t;

    if (c >= NN_DIGIT_BITS)
        return (0);

    t = NN_DIGIT_BITS - c;

    carry = 0;

    for (i = 0; i < digits; i++) {
        bi = b[i];
        a[i] = (bi << c) | carry;
        carry = c ? (bi >> t) : 0;
    }

    return (carry);
}

/* Computes a = c div 2^c (i.e., shifts right c bits), returning carry.

   Lengths: a[digits], b[digits].
   Requires: c < NN_DIGIT_BITS.
 */
NN_DIGIT NN_RShift(NN_DIGIT* a,
    NN_DIGIT* b,
    unsigned int c,
    unsigned int digits)

{
    NN_DIGIT bi, carry;
    int i;
    unsigned int t;

    if (c >= NN_DIGIT_BITS)
        return (0);

    t = NN_DIGIT_BITS - c;

    carry = 0;

    for (i = digits - 1; i >= 0; i--) {
        bi = b[i];
        a[i] = (bi >> c) | carry;
        carry = c ? (bi << t) : 0;
    }

    return (carry);
}


/* Computes a = b * c, where b and c are digits.

   Lengths: a[2].
 */
void NN_DigitMult(NN_DIGIT a[2], NN_DIGIT b, NN_DIGIT c)
{
    NN_DIGIT t, u;
    NN_HALF_DIGIT bHigh, bLow, cHigh, cLow;

    bHigh = (NN_HALF_DIGIT)HIGH_HALF(b);
    bLow = (NN_HALF_DIGIT)LOW_HALF(b);
    cHigh = (NN_HALF_DIGIT)HIGH_HALF(c);
    cLow = (NN_HALF_DIGIT)LOW_HALF(c);

    a[0] = (NN_DIGIT)bLow * (NN_DIGIT)cLow;
    t = (NN_DIGIT)bLow * (NN_DIGIT)cHigh;
    u = (NN_DIGIT)bHigh * (NN_DIGIT)cLow;
    a[1] = (NN_DIGIT)bHigh * (NN_DIGIT)cHigh;

    if ((t += u) < u)
        a[1] += TO_HIGH_HALF(1);
    u = TO_HIGH_HALF(t);

    if ((a[0] += u) < u)
        a[1]++;
    a[1] += HIGH_HALF(t);
}

/* Sets a = b / c, where a and c are digits.

   Lengths: b[2].
   Assumes b[1] < c and HIGH_HALF (c) > 0. For efficiency, c should be
   normalized.
 */
void NN_DigitDiv(NN_DIGIT* a, NN_DIGIT b[2], NN_DIGIT c)
{
    NN_DIGIT t[2], u, v;
    NN_HALF_DIGIT aHigh, aLow, cHigh, cLow;

    cHigh = (NN_HALF_DIGIT)HIGH_HALF(c);
    cLow = (NN_HALF_DIGIT)LOW_HALF(c);

    t[0] = b[0];
    t[1] = b[1];

    /* Underestimate high half of quotient and subtract.
     */
    if (cHigh == MAX_NN_HALF_DIGIT)
        aHigh = (NN_HALF_DIGIT)HIGH_HALF(t[1]);
    else
        aHigh = (NN_HALF_DIGIT)(t[1] / (cHigh + 1));
    u = (NN_DIGIT)aHigh * (NN_DIGIT)cLow;
    v = (NN_DIGIT)aHigh * (NN_DIGIT)cHigh;
    if ((t[0] -= TO_HIGH_HALF(u)) > (MAX_NN_DIGIT - TO_HIGH_HALF(u)))
        t[1]--;
    t[1] -= HIGH_HALF(u);
    t[1] -= v;

    /* Correct estimate.
     */
    while ((t[1] > cHigh) ||
        ((t[1] == cHigh) && (t[0] >= TO_HIGH_HALF(cLow)))) {
        if ((t[0] -= TO_HIGH_HALF(cLow)) > MAX_NN_DIGIT - TO_HIGH_HALF(cLow))
            t[1]--;
        t[1] -= cHigh;
        aHigh++;
    }

    /* Underestimate low half of quotient and subtract.
     */
    if (cHigh == MAX_NN_HALF_DIGIT)
        aLow = (NN_HALF_DIGIT)LOW_HALF(t[1]);
    else
        aLow =
        (NN_HALF_DIGIT)((TO_HIGH_HALF(t[1]) + HIGH_HALF(t[0])) / (cHigh + 1));
    u = (NN_DIGIT)aLow * (NN_DIGIT)cLow;
    v = (NN_DIGIT)aLow * (NN_DIGIT)cHigh;
    if ((t[0] -= u) > (MAX_NN_DIGIT - u))
        t[1]--;
    if ((t[0] -= TO_HIGH_HALF(v)) > (MAX_NN_DIGIT - TO_HIGH_HALF(v)))
        t[1]--;
    t[1] -= HIGH_HALF(v);

    /* Correct estimate.
     */
    while ((t[1] > 0) || ((t[1] == 0) && t[0] >= c)) {
        if ((t[0] -= c) > (MAX_NN_DIGIT - c))
            t[1]--;
        aLow++;
    }

    *a = TO_HIGH_HALF(aHigh) + aLow;
}

/* Computes a = c div d and b = c mod d.

   Lengths: a[cDigits], b[dDigits], c[cDigits], d[dDigits].
   Assumes d > 0, cDigits < 2 * MAX_NN_DIGITS,
           dDigits < MAX_NN_DIGITS.
 */
void NN_Div(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT* c,
    unsigned int cDigits,
    NN_DIGIT* d,
    unsigned int dDigits)

{
    NN_DIGIT ai, cc[2 * MAX_NN_DIGITS + 1], dd[MAX_NN_DIGITS], t;
    int i;
    unsigned int ddDigits, shift;

    ddDigits = NN_Digits(d, dDigits);
    if (ddDigits == 0)
        return;

    /* Normalize operands.
     */
    shift = NN_DIGIT_BITS - NN_DigitBits(d[ddDigits - 1]);
    NN_AssignZero(cc, ddDigits);
    cc[cDigits] = NN_LShift(cc, c, shift, cDigits);
    NN_LShift(dd, d, shift, ddDigits);
    t = dd[ddDigits - 1];

    NN_AssignZero(a, cDigits);

    for (i = cDigits - ddDigits; i >= 0; i--) {
        /* Underestimate quotient digit and subtract.
         */
        if (t == MAX_NN_DIGIT)
            ai = cc[i + ddDigits];
        else
            NN_DigitDiv(&ai, &cc[i + ddDigits - 1], t + 1);
        cc[i + ddDigits] -= NN_SubDigitMult(&cc[i], &cc[i], ai, dd, ddDigits);

        /* Correct estimate.
         */
        while (cc[i + ddDigits] || (NN_Cmp(&cc[i], dd, ddDigits) >= 0)) {
            ai++;
            cc[i + ddDigits] -= NN_Sub(&cc[i], &cc[i], dd, ddDigits);
        }

        a[i] = ai;
    }

    /* Restore result.
     */
    NN_AssignZero(b, dDigits);
    NN_RShift(b, cc, shift, ddDigits);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)cc, 0, sizeof(cc));
    R_memset((POINTER)dd, 0, sizeof(dd));
}

/* Computes a = b mod c.

   Lengths: a[cDigits], b[bDigits], c[cDigits].
   Assumes c > 0, bDigits < 2 * MAX_NN_DIGITS, cDigits < MAX_NN_DIGITS.
 */
void NN_Mod(NN_DIGIT* a,
    NN_DIGIT* b,
    unsigned int bDigits,
    NN_DIGIT* c,
    unsigned int cDigits)
{
    NN_DIGIT t[2 * MAX_NN_DIGITS];

    NN_Div(t, a, b, bDigits, c, cDigits);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)t, 0, sizeof(t));
}

/* Computes a = b * c mod d.

   Lengths: a[digits], b[digits], c[digits], d[digits].
   Assumes d > 0, digits < MAX_NN_DIGITS.
 */
void NN_ModMult(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT* c,
    NN_DIGIT* d,
    unsigned int digits)

{
    NN_DIGIT t[2 * MAX_NN_DIGITS];

    NN_Mult(t, b, c, digits);
    NN_Mod(a, t, 2 * digits, d, digits);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)t, 0, sizeof(t));
}

/* Computes a = b^c mod d.

   Lengths: a[dDigits], b[dDigits], c[cDigits], d[dDigits].
   Assumes d > 0, cDigits > 0, dDigits < MAX_NN_DIGITS.
 */

 /* PGP 2.5's mpilib contains a faster modular exponentiation routine, mp_modexp.
    If USEMPILIB is defined, NN_ModExp is replaced in the PGP 2.5 sources with a
    stub call to mp_modexp.  If USEMPILIB is not defined, we'll get a pure (albeit
    slower) RSAREF implementation.

    The RSAREF 2.0 license, clause 1(c), permits "...modify[ing] the Program in any
    manner for porting or performance improvement purposes..." */

#ifndef USEMPILIB   
void NN_ModExp(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT* c,
    unsigned int cDigits,
    NN_DIGIT* d,
    unsigned int dDigits)

{
    NN_DIGIT bPower[3][MAX_NN_DIGITS], ci, t[MAX_NN_DIGITS];
    int i;
    unsigned int ciBits, j, s;

    /* Store b, b^2 mod d, and b^3 mod d.
     */
    NN_Assign(bPower[0], b, dDigits);
    NN_ModMult(bPower[1], bPower[0], b, d, dDigits);
    NN_ModMult(bPower[2], bPower[1], b, d, dDigits);

    NN_ASSIGN_DIGIT(t, 1, dDigits);

    cDigits = NN_Digits(c, cDigits);
    for (i = cDigits - 1; i >= 0; i--) {
        ci = c[i];
        ciBits = NN_DIGIT_BITS;

        /* Scan past leading zero bits of most significant digit.
         */
        if (i == (int)(cDigits - 1)) {
            while (!DIGIT_2MSB(ci)) {
                ci <<= 2;
                ciBits -= 2;
            }
        }

        for (j = 0; j < ciBits; j += 2, ci <<= 2) {
            /* Compute t = t^4 * b^s mod d, where s = two MSB's of ci.
             */
            NN_ModMult(t, t, t, d, dDigits);
            NN_ModMult(t, t, t, d, dDigits);
            if ((s = DIGIT_2MSB(ci)) != 0)
                NN_ModMult(t, t, bPower[s - 1], d, dDigits);
        }
    }

    NN_Assign(a, t, dDigits);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)bPower, 0, sizeof(bPower));
    R_memset((POINTER)t, 0, sizeof(t));
}
#endif   

/* Compute a = 1/b mod c, assuming inverse exists.

   Lengths: a[digits], b[digits], c[digits].
   Assumes gcd (b, c) = 1, digits < MAX_NN_DIGITS.
 */
void NN_ModInv(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT* c,
    unsigned int digits)
{
    NN_DIGIT q[MAX_NN_DIGITS], t1[MAX_NN_DIGITS], t3[MAX_NN_DIGITS],
        u1[MAX_NN_DIGITS], u3[MAX_NN_DIGITS], v1[MAX_NN_DIGITS],
        v3[MAX_NN_DIGITS], w[2 * MAX_NN_DIGITS];
    int u1Sign;

    /* Apply extended Euclidean algorithm, modified to avoid negative
       numbers.
     */
    NN_ASSIGN_DIGIT(u1, 1, digits);
    NN_AssignZero(v1, digits);
    NN_Assign(u3, b, digits);
    NN_Assign(v3, c, digits);
    u1Sign = 1;

    while (!NN_Zero(v3, digits)) {
        NN_Div(q, t3, u3, digits, v3, digits);
        NN_Mult(w, q, v1, digits);
        NN_Add(t1, u1, w, digits);
        NN_Assign(u1, v1, digits);
        NN_Assign(v1, t1, digits);
        NN_Assign(u3, v3, digits);
        NN_Assign(v3, t3, digits);
        u1Sign = -u1Sign;
    }

    /* Negate result if sign is negative.
      */
    if (u1Sign < 0)
        NN_Sub(a, c, u1, digits);
    else
        NN_Assign(a, u1, digits);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)q, 0, sizeof(q));
    R_memset((POINTER)t1, 0, sizeof(t1));
    R_memset((POINTER)t3, 0, sizeof(t3));
    R_memset((POINTER)u1, 0, sizeof(u1));
    R_memset((POINTER)u3, 0, sizeof(u3));
    R_memset((POINTER)v1, 0, sizeof(v1));
    R_memset((POINTER)v3, 0, sizeof(v3));
    R_memset((POINTER)w, 0, sizeof(w));
}

/* Computes a = gcd(b, c).

   Lengths: a[digits], b[digits], c[digits].
   Assumes b > c, digits < MAX_NN_DIGITS.
 */
void NN_Gcd(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT* c,
    unsigned int digits)

{
    NN_DIGIT t[MAX_NN_DIGITS], u[MAX_NN_DIGITS], v[MAX_NN_DIGITS];

    NN_Assign(u, b, digits);
    NN_Assign(v, c, digits);

    while (!NN_Zero(v, digits)) {
        NN_Mod(t, u, digits, v, digits);
        NN_Assign(u, v, digits);
        NN_Assign(v, t, digits);
    }

    NN_Assign(a, u, digits);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)t, 0, sizeof(t));
    R_memset((POINTER)u, 0, sizeof(u));
    R_memset((POINTER)v, 0, sizeof(v));
}

/* Computes a = b + c*d, where c is a digit. Returns carry.

   Lengths: a[digits], b[digits], d[digits].
 */
static NN_DIGIT NN_AddDigitMult(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT c,
    NN_DIGIT* d,
    unsigned int digits)

{
    NN_DIGIT carry, t[2];
    unsigned int i;

    if (c == 0)
        return (0);

    carry = 0;
    for (i = 0; i < digits; i++) {
        NN_DigitMult(t, c, d[i]);
        if ((a[i] = b[i] + carry) < carry)
            carry = 1;
        else
            carry = 0;
        if ((a[i] += t[0]) < t[0])
            carry++;
        carry += t[1];
    }

    return (carry);
}

/* Computes a = b - c*d, where c is a digit. Returns borrow.

   Lengths: a[digits], b[digits], d[digits].
 */
static NN_DIGIT NN_SubDigitMult(NN_DIGIT* a,
    NN_DIGIT* b,
    NN_DIGIT c,
    NN_DIGIT* d,
    unsigned int digits)
{
    NN_DIGIT borrow, t[2];
    unsigned int i;

    if (c == 0)
        return (0);

    borrow = 0;
    for (i = 0; i < digits; i++) {
        NN_DigitMult(t, c, d[i]);
        if ((a[i] = b[i] - borrow) > (MAX_NN_DIGIT - borrow))
            borrow = 1;
        else
            borrow = 0;
        if ((a[i] -= t[0]) > (MAX_NN_DIGIT - t[0]))
            borrow++;
        borrow += t[1];
    }

    return (borrow);
}

/* Returns the significant length of a in bits, where a is a digit.
 */
static unsigned int NN_DigitBits(NN_DIGIT a)
{
    unsigned int i;

    for (i = 0; i < NN_DIGIT_BITS; i++, a >>= 1)
        if (a == 0)
            break;

    return (i);
}


/**
RSA public-key PublicBlock and RSAPrivateBlock.
**/

int RSAPublicBlock
(unsigned char*, unsigned int*, unsigned char*, unsigned int,
    R_RSA_PUBLIC_KEY*);
int RSAPrivateBlock
(unsigned char*, unsigned int*, unsigned char*, unsigned int,
    R_RSA_PRIVATE_KEY*);

/* RSA public-key encryption, according to PKCS #1.
 */
int RSAPublicEncrypt
(unsigned char* output,                                      /* output block */
    unsigned int* outputLen,                          /* length of output block */
    unsigned char* input,                                        /* input block */
    unsigned int inputLen,                             /* length of input block */
    R_RSA_PUBLIC_KEY* publicKey                         /* RSA public key */
)

{
    int status;
    unsigned char byte, pkcsBlock[MAX_RSA_MODULUS_LEN];
    unsigned int i, modulusLen;

    modulusLen = (publicKey->bits + 7) / 8;
    if (inputLen + 11 > modulusLen)
        return (RE_LEN);

    pkcsBlock[0] = 0;
    /* block type 2 */
    pkcsBlock[1] = 2;

    for (i = 2; i < modulusLen - inputLen - 1; i++) {
        /* Find nonzero random byte.
         */
         //do {   
           //R_GenerateBytes (&byte, 1, randomStruct);   
         //} while (byte == 0);   
         //pkcsBlock[i] = byte;   
    }
    /* separator */
    //pkcsBlock[i++] = 0;   

    R_memcpy((POINTER)& pkcsBlock[i], (POINTER)input, inputLen);

    status = RSAPublicBlock
    (output, outputLen, pkcsBlock, modulusLen, publicKey);

    /* Zeroize sensitive information.
     */
    byte = 0;
    R_memset((POINTER)pkcsBlock, 0, sizeof(pkcsBlock));

    return (status);
}
/* wyhadd this function for raw rsa encrypt
output: buf for result, buffer must be >= (publicKey->bits + 7) / 8.
outputlen: result len
input: data to be encrypted
inputlen: input data len
publicKey: n,e,bits len
randomStruct: not used

notice:
data endian: input[0] is the biggest, input[len-1] is the lest
output: the first data is output[(publicKey->bits + 7) / 8-1]
*/
int wyhRSAPublicEncrypt(
    unsigned char* output,                                      /* output block */
    unsigned int* outputLen,                          /* length of output block */
    unsigned char* input,                                        /* input block */
    unsigned int inputLen,                             /* length of input block */
    R_RSA_PUBLIC_KEY* publicKey,                              /* RSA public key */
    R_RANDOM_STRUCT* randomStruct                          /* random structure */
)
{
    int status;
    unsigned char byte, pkcsBlock[MAX_RSA_MODULUS_LEN];
    unsigned int i, modulusLen;

    modulusLen = (publicKey->bits + 7) / 8;
    if (inputLen > modulusLen)
        return (RE_LEN);

    status = RSAPublicBlock
    (output, outputLen, input, inputLen, publicKey);



    return (status);
}


/* RSA public-key decryption, according to PKCS #1.
 */
int RSAPublicDecrypt(
    unsigned char* output,                                      /* output block */
    unsigned int* outputLen,                          /* length of output block */
    unsigned char* input,                                        /* input block */
    unsigned int inputLen,                             /* length of input block */
    R_RSA_PUBLIC_KEY* publicKey                              /* RSA public key */
)
{
    int status;
    unsigned char pkcsBlock[MAX_RSA_MODULUS_LEN];
    unsigned int i, modulusLen, pkcsBlockLen;

    modulusLen = (publicKey->bits + 7) / 8;
    if (inputLen > modulusLen)
        return (RE_LEN);

    if (status = RSAPublicBlock
    (pkcsBlock, &pkcsBlockLen, input, inputLen, publicKey))
        return (status);

    if (pkcsBlockLen != modulusLen)
        return (RE_LEN);

    /* Require block type 1.
     */
    if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 1))
        return (RE_DATA);

    for (i = 2; i < modulusLen - 1; i++)
        if (pkcsBlock[i] != 0xff)
            break;

    /* separator */
    if (pkcsBlock[i++] != 0)
        return (RE_DATA);

    *outputLen = modulusLen - i;

    if (*outputLen + 11 > modulusLen)
        return (RE_DATA);

    R_memcpy((POINTER)output, (POINTER)& pkcsBlock[i], *outputLen);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)pkcsBlock, 0, sizeof(pkcsBlock));

    return (0);
}



/* RSA private-key encryption, according to PKCS #1.
 */
int RSAPrivateEncrypt(
    unsigned char* output,                                      /* output block */
    unsigned int* outputLen,                          /* length of output block */
    unsigned char* input,                                        /* input block */
    unsigned int inputLen,                             /* length of input block */
    R_RSA_PRIVATE_KEY* privateKey                           /* RSA private key */
)
{
    int status;
    unsigned char pkcsBlock[MAX_RSA_MODULUS_LEN];
    unsigned int i, modulusLen;

    modulusLen = (privateKey->bits + 7) / 8;
#if 1 //PCKS1Моід      
    if (inputLen + 11 > modulusLen)
        return (RE_LEN);

    pkcsBlock[0] = 0;
    /* block type 1 */
    pkcsBlock[1] = 1;

    for (i = 2; i < modulusLen - inputLen - 1; i++)
        pkcsBlock[i] = 0xff;

    /* separator */
    pkcsBlock[i++] = 0;

    R_memcpy((POINTER)& pkcsBlock[i], (POINTER)input, inputLen);
#endif
    //   R_memcpy ((POINTER)&pkcsBlock[0], (POINTER)input, inputLen);    


    status = RSAPrivateBlock
    (output, outputLen, pkcsBlock, modulusLen, privateKey);

    /* Zeroize potentially sensitive information.
     */
    R_memset((POINTER)pkcsBlock, 0, sizeof(pkcsBlock));

    return (status);
}

/* RSA private-key decryption, according to PKCS #1.
 */
int RSAPrivateDecrypt(
    unsigned char* output,                                      /* output block */
    unsigned int* outputLen,                          /* length of output block */
    unsigned char* input,                                        /* input block */
    unsigned int inputLen,                             /* length of input block */
    R_RSA_PRIVATE_KEY* privateKey                           /* RSA private key */
)
{
    int status;
    unsigned char pkcsBlock[MAX_RSA_MODULUS_LEN];
    unsigned int i, modulusLen, pkcsBlockLen;

    modulusLen = (privateKey->bits + 7) / 8;
    if (inputLen > modulusLen)
        return (RE_LEN);

    if (status = RSAPrivateBlock
    (pkcsBlock, &pkcsBlockLen, input, inputLen, privateKey))
        return (status);

    if (pkcsBlockLen != modulusLen)
        return (RE_LEN);

    /* Require block type 2.
     */
    if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 2))
        return (RE_DATA);

    for (i = 2; i < modulusLen - 1; i++)
        /* separator */
        if (pkcsBlock[i] == 0)
            break;

    i++;
    if (i >= modulusLen)
        return (RE_DATA);

    *outputLen = modulusLen - i;

    if (*outputLen + 11 > modulusLen)
        return (RE_DATA);

    R_memcpy((POINTER)output, (POINTER)& pkcsBlock[i], *outputLen);

    /* Zeroize sensitive information.
     */
    R_memset((POINTER)pkcsBlock, 0, sizeof(pkcsBlock));

    return (0);
}

/* Raw RSA public-key operation. Output has same length as modulus.

   Assumes inputLen < length of modulus.
   Requires input < modulus.
 */
int RSAPublicBlock(
    unsigned char* output,                                      /* output block */
    unsigned int* outputLen,                          /* length of output block */
    unsigned char* input,                                        /* input block */
    unsigned int inputLen,                             /* length of input block */
    R_RSA_PUBLIC_KEY* publicKey                              /* RSA public key */
)
{
    NN_DIGIT c[MAX_NN_DIGITS], e[MAX_NN_DIGITS], m[MAX_NN_DIGITS],
        n[MAX_NN_DIGITS];
    unsigned int eDigits, nDigits;

    NN_Decode(m, MAX_NN_DIGITS, input, inputLen);
    NN_Decode(n, MAX_NN_DIGITS, publicKey->modulus, MAX_RSA_MODULUS_LEN);
    NN_Decode(e, MAX_NN_DIGITS, publicKey->exponent, MAX_RSA_MODULUS_LEN);
    nDigits = NN_Digits(n, MAX_NN_DIGITS);
    eDigits = NN_Digits(e, MAX_NN_DIGITS);

    if (NN_Cmp(m, n, nDigits) >= 0)
        return (RE_DATA);

    /* Compute c = m^e mod n.
     */
    NN_ModExp(c, m, e, eDigits, n, nDigits);

    *outputLen = (publicKey->bits + 7) / 8;
    NN_Encode(output, *outputLen, c, nDigits);

    /* Zeroize sensitive information.
     */
    R_memset((POINTER)c, 0, sizeof(c));
    R_memset((POINTER)m, 0, sizeof(m));

    return (0);
}

/* Raw RSA private-key operation. Output has same length as modulus.

   Assumes inputLen < length of modulus.
   Requires input < modulus.
 */
int RSAPrivateBlock(
    unsigned char* output,                                      /* output block */
    unsigned int* outputLen,                          /* length of output block */
    unsigned char* input,                                        /* input block */
    unsigned int inputLen,                             /* length of input block */
    R_RSA_PRIVATE_KEY* privateKey                           /* RSA private key */
)
{
    NN_DIGIT c[MAX_NN_DIGITS], cP[MAX_NN_DIGITS], cQ[MAX_NN_DIGITS],
        dP[MAX_NN_DIGITS], dQ[MAX_NN_DIGITS], mP[MAX_NN_DIGITS],
        mQ[MAX_NN_DIGITS], n[MAX_NN_DIGITS], p[MAX_NN_DIGITS], q[MAX_NN_DIGITS],
        qInv[MAX_NN_DIGITS], t[MAX_NN_DIGITS];
    unsigned int cDigits, nDigits, pDigits;

    NN_Decode(c, MAX_NN_DIGITS, input, inputLen);
    NN_Decode(n, MAX_NN_DIGITS, privateKey->modulus, MAX_RSA_MODULUS_LEN);
    NN_Decode(p, MAX_NN_DIGITS, privateKey->prime[0], MAX_RSA_PRIME_LEN);
    NN_Decode(q, MAX_NN_DIGITS, privateKey->prime[1], MAX_RSA_PRIME_LEN);
    NN_Decode
    (dP, MAX_NN_DIGITS, privateKey->primeExponent[0], MAX_RSA_PRIME_LEN);
    NN_Decode
    (dQ, MAX_NN_DIGITS, privateKey->primeExponent[1], MAX_RSA_PRIME_LEN);
    NN_Decode(qInv, MAX_NN_DIGITS, privateKey->coefficient, MAX_RSA_PRIME_LEN);
    cDigits = NN_Digits(c, MAX_NN_DIGITS);
    nDigits = NN_Digits(n, MAX_NN_DIGITS);
    pDigits = NN_Digits(p, MAX_NN_DIGITS);

    if (NN_Cmp(c, n, nDigits) >= 0)
        return (RE_DATA);

    /* Compute mP = cP^dP mod p  and  mQ = cQ^dQ mod q. (Assumes q has
       length at most pDigits, i.e., p > q.)
     */
    NN_Mod(cP, c, cDigits, p, pDigits);
    NN_Mod(cQ, c, cDigits, q, pDigits);
    NN_ModExp(mP, cP, dP, pDigits, p, pDigits);
    NN_AssignZero(mQ, nDigits);
    NN_ModExp(mQ, cQ, dQ, pDigits, q, pDigits);

    /* Chinese Remainder Theorem:
         m = ((((mP - mQ) mod p) * qInv) mod p) * q + mQ.
     */
    if (NN_Cmp(mP, mQ, pDigits) >= 0)
        NN_Sub(t, mP, mQ, pDigits);
    else {
        NN_Sub(t, mQ, mP, pDigits);
        NN_Sub(t, p, t, pDigits);
    }
    NN_ModMult(t, t, qInv, p, pDigits);
    NN_Mult(t, t, q, pDigits);
    NN_Add(t, t, mQ, nDigits);

    *outputLen = (privateKey->bits + 7) / 8;
    NN_Encode(output, *outputLen, t, nDigits);

    /* Zeroize sensitive information.
     */
    R_memset((POINTER)c, 0, sizeof(c));
    R_memset((POINTER)cP, 0, sizeof(cP));
    R_memset((POINTER)cQ, 0, sizeof(cQ));
    R_memset((POINTER)dP, 0, sizeof(dP));
    R_memset((POINTER)dQ, 0, sizeof(dQ));
    R_memset((POINTER)mP, 0, sizeof(mP));
    R_memset((POINTER)mQ, 0, sizeof(mQ));
    R_memset((POINTER)p, 0, sizeof(p));
    R_memset((POINTER)q, 0, sizeof(q));
    R_memset((POINTER)qInv, 0, sizeof(qInv));
    R_memset((POINTER)t, 0, sizeof(t));

    return (0);
}