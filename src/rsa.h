/* RSAREF.H - header file for RSAREF cryptographic toolkit
 */

 /* Copyright (C) RSA Laboratories, a division of RSA Data Security,
      Inc., created 1991. All rights reserved.
  */

#ifndef _RSA_H_ 
#define _RSA_H_ 1 


#ifdef __cplusplus 
extern "C" {
#endif 



    /* Length of digit in bits */
#define NN_DIGIT_BITS 32 
#define NN_HALF_DIGIT_BITS 16 
/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS / 8) 
/* Maximum length in digits */
#define MAX_NN_DIGITS   ((MAX_RSA_MODULUS_LEN + NN_DIGIT_LEN - 1) / NN_DIGIT_LEN + 1) 
/* Maximum digits */
#define MAX_NN_DIGIT 0xffffffff 
#define MAX_NN_HALF_DIGIT 0xffff 
/* Macros.
 */
#define LOW_HALF(x) ((x) & MAX_NN_HALF_DIGIT) 
#define HIGH_HALF(x) (((x) >> NN_HALF_DIGIT_BITS) & MAX_NN_HALF_DIGIT) 
#define TO_HIGH_HALF(x) (((NN_DIGIT)(x)) << NN_HALF_DIGIT_BITS) 
#define DIGIT_MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 1)) & 1) 
#define DIGIT_2MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 2)) & 3) 

#define NN_ASSIGN_DIGIT(a, b, digits) {NN_AssignZero (a, digits); a[0] = b;} 
#define NN_EQUAL(a, b, digits) (! NN_Cmp (a, b, digits)) 
#define NN_EVEN(a, digits) (((digits) == 0) || ! (a[0] & 1)) 


 /* RSA key lengths.
  */
#define MIN_RSA_MODULUS_BITS 64 //wyh raw 508
#define MAX_RSA_MODULUS_BITS 2048 // WYH RAW 1024 
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8) 
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2) 
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8) 

  /* Maximum lengths of encoded and encrypted content, as a function of
     content length len. Also, inverse functions.
   */
#define ENCODED_CONTENT_LEN(len) (4*(len)/3 + 3) 
#define ENCRYPTED_CONTENT_LEN(len) ENCODED_CONTENT_LEN ((len)+8) 
#define DECODED_CONTENT_LEN(len) (3*(len)/4 + 1) 
#define DECRYPTED_CONTENT_LEN(len) (DECODED_CONTENT_LEN (len) - 1) 


   /* Maximum length of Diffie-Hellman parameters.
    */
#define DH_PRIME_LEN(bits) (((bits) + 7) / 8) 

    /* Error codes.
     */
#define RE_CONTENT_ENCODING 0x0400 
#define RE_DATA 0x0401 
#define RE_DIGEST_ALGORITHM 0x0402 
#define RE_ENCODING 0x0403 
#define RE_KEY 0x0404 
#define RE_KEY_ENCODING 0x0405 
#define RE_LEN 0x0406 
#define RE_MODULUS_LEN 0x0407 
#define RE_NEED_RANDOM 0x0408 
#define RE_PRIVATE_KEY 0x0409 
#define RE_PUBLIC_KEY 0x040a 
#define RE_SIGNATURE 0x040b 
#define RE_SIGNATURE_ENCODING 0x040c 
#define RE_ENCRYPTION_ALGORITHM 0x040d 

     /* Random structure.
      */
    typedef struct {
        unsigned int bytesNeeded;
        unsigned char state[16];
        unsigned int outputAvailable;
        unsigned char output[16];
    } R_RANDOM_STRUCT;

    /* RSA public and private key.
     */
    typedef struct {
        unsigned int bits;                           /* length in bits of modulus */
        unsigned char modulus[MAX_RSA_MODULUS_LEN];                    /* modulus :N */
        unsigned char exponent[MAX_RSA_MODULUS_LEN];           /* public exponent :E */
    } R_RSA_PUBLIC_KEY;

    typedef struct {
        unsigned int bits;                           /* length in bits of modulus */
        unsigned char modulus[MAX_RSA_MODULUS_LEN];                    /* modulus :N*/
        unsigned char publicExponent[MAX_RSA_MODULUS_LEN];     /* public exponent :E*/
        unsigned char exponent[MAX_RSA_MODULUS_LEN];          /* private exponent :D*/
        unsigned char prime[2][MAX_RSA_PRIME_LEN];               /* prime factors :P,Q*/
        unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];   /* exponents for CRT :dP,dQ*/
        unsigned char coefficient[MAX_RSA_PRIME_LEN];          /* CRT coefficient :qInv*/
    } R_RSA_PRIVATE_KEY;

    /* RSA prototype key.
     */
    typedef struct {
        unsigned int bits;                           /* length in bits of modulus */
        int useFermat4;                        /* public exponent (1 = F4, 0 = 3) */
    } R_RSA_PROTO_KEY;




    /* PROTOTYPES should be set to one if and only if the compiler supports
         function argument prototyping.
       The following makes PROTOTYPES default to 1 if it has not already been
         defined as 0 with C compiler flags.
     */
#ifndef PROTOTYPES 
#define PROTOTYPES 1 
#endif 

     /* POINTER defines a generic pointer type */
    typedef unsigned char* POINTER;

    /* UINT2 defines a two byte word */
    typedef unsigned short int UINT2;

    /* UINT4 defines a four byte word */
    typedef unsigned long int UINT4;

    /* Type definitions.
     */
    typedef UINT4 NN_DIGIT;
    typedef UINT2 NN_HALF_DIGIT;

#ifndef NULL_PTR 
#define NULL_PTR ((POINTER)0) 
#endif 

#ifndef UNUSED_ARG 
#define UNUSED_ARG(x) x = *(&x); 
#endif 

    /* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
       If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
         returns an empty list.
     */
#if PROTOTYPES 
#define PROTO_LIST(list) list 
#else 
#define PROTO_LIST(list) () 
#endif 




    int RSAPublicBlock PROTO_LIST
    ((unsigned char* output, unsigned int* outputLen,
        unsigned char* input, unsigned int inputLen,
        R_RSA_PUBLIC_KEY*));

    int RSAPrivateBlock PROTO_LIST
    ((unsigned char* output, unsigned int* outputLen,
        unsigned char* input, unsigned int inputLen,
        R_RSA_PRIVATE_KEY*));


#ifdef __cplusplus 
}
#endif 

#endif