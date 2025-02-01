/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <cmath>

int per_line=16;

void print_space_before_number(int number, int totalchars)
{
    totalchars--;
    if (number>=10)
        totalchars--;
    if (number>=100)
        totalchars--;
    for (int i=0;i<totalchars;i++)
        printf(" ");
    printf("%d",number);
}

/*
 * Hamming 24/18 parity table to get precalculated parity of byte N+x
 * Input  = [x][byte value]
 * Output = FEDCBA in binairy representation
 *
 * From : https://www.etsi.org/deliver/etsi_i_ets/300700_300799/300706/01_40_57/ets_300706e01o.pdf
 * A = 1 ⊕ P1 ⊕ D1 ⊕ D2 ⊕ D4 ⊕ D5 ⊕ D7 ⊕ D9 ⊕ D11 ⊕ D12 ⊕ D14 ⊕ D16 ⊕ D18
 * B = 1 ⊕PP2 ⊕ D1 ⊕ D3 ⊕ D4 ⊕ D6 ⊕ D7 ⊕ D10 ⊕ D11 ⊕ D13 ⊕ D14 ⊕ D17 ⊕ D18
 * C = 1 ⊕PP3 ⊕ D2 ⊕ D3 ⊕ D4 ⊕ D8 ⊕ D9 ⊕ D10 ⊕ D11 ⊕ D15 ⊕ D16 ⊕ D17 ⊕ D18
 * D = 1 ⊕PP4 ⊕ D5 ⊕ D6 ⊕ D7 ⊕ D8 ⊕ D9 ⊕ D10 ⊕ D11
 * E = 1 ⊕PP5 ⊕ D12 ⊕ D13 ⊕ D14 ⊕ D15 ⊕ D16 ⊕ D17 ⊕ D18
 * F = 1 ⊕PP6 ⊕ P1 ⊕ P2 ⊕ D1 ⊕ P3 ⊕ D2 ⊕ D3 ⊕ D4 ⊕ P4 ⊕ D5 ⊕ D6 ⊕ D7 ⊕ D8 ⊕ D9 ⊕ D10 ⊕ D11 ⊕ P5 ⊕ D12 ⊕ D13 ⊕ D14 ⊕ D15 ⊕ D16 ⊕ D17 ⊕ D18
 *
 * byte N + 0 = P1  P2  D1  P3  D2  D3  D4  P4
 * byte N + 1 = D5  D6  D7  D8  D9  D10 D11 P5
 * byte N + 2 = D12 D13 D14 D15 D16 D17 D18 P6
 *
*/
void calc_hamm24par()
{
    int index;

    int D1,D2,D3,D4,D5,D6,D7,D8,D9,D10,D11,D12,D13,D14,D15,D16,D17,D18;
    int P1,P2,P3,P4,P5,P6;
    int A,B,C,D,E,F;

    printf("const std::uint8_t hamm24par[3][256] =\n");
    printf("{\n");
    printf("    {\n");
/*
 * byte N + 0 = P1  P2  D1  P3  D2  D3  D4  P4 (bitwise flipped)
 *
 * A = P1 ⊕ D1 ⊕ D2 ⊕ D4
 * B = P2 ⊕ D1 ⊕ D3 ⊕ D4
 * C = P3 ⊕ D2 ⊕ D3 ⊕ D4
 * D = P4
 * E = 0
 * F = P1 ⊕ P2 ⊕ D1 ⊕ P3 ⊕ D2 ⊕ D3 ⊕ D4 ⊕ P4
 *
 *    input         |  output
 * 0x00 = 00000000b -> 000000b = 0
 * 0x01 = 00000001b -> 100001b = 33
 * 0x02 = 00000010b -> 100010b = 34
 * 0x03 = 00000011b -> 000011b = 3
 * 0x04 = 00000100b -> 100011b = 35
 *                  .
 *                  .
 *                  .
 *                  .
 * 0xFE = 11111110b -> 101001b = 41
 * 0xFF = 11111111b -> 001000b = 8
*/
    index=0;
    for (int i=0; i<256; i++){
        if (index%per_line==0)
            printf("   ");

        P1 = (i>>0) & 1;
        P2 = (i>>1) & 1;
        D1 = (i>>2) & 1;
        P3 = (i>>3) & 1;
        D2 = (i>>4) & 1;
        D3 = (i>>5) & 1;
        D4 = (i>>6) & 1;
        P4 = (i>>7) & 1;

        A = P1 ^ D1 ^ D2 ^ D4;
        B = P2 ^ D1 ^ D3 ^ D4;
        C = P3 ^ D2 ^ D3 ^ D4;
        D = P4;
        E = 0;
        F = P1 ^ P2 ^ D1 ^ P3 ^ D2 ^ D3 ^ D4 ^ P4;

        int result= (F<<5) | (E<<4) | (D<<3) | (C<<2) | (B<<1) | (A<<0);
        print_space_before_number(result,3);
        if (i!=255)
            printf(",");

        index++;
        if (index%per_line==0)
            printf("\n");
    }

    printf("    },\n");
    printf("    {\n");
/*
 * byte N + 1 = D5  D6  D7  D8  D9  D10 D11 P5 (bitwise flipped)
 *
 * A = D5 ⊕ D7 ⊕ D9 ⊕ D11
 * B = D6 ⊕ D7 ⊕ D10 ⊕ D11
 * C = D8 ⊕ D9 ⊕ D10 ⊕ D11
 * D = D5 ⊕ D6 ⊕ D7 ⊕ D8 ⊕ D9 ⊕ D10 ⊕ D11
 * E = P5
 * F = D5 ⊕ D6 ⊕ D7 ⊕ D8 ⊕ D9 ⊕ D10 ⊕ D11 ⊕ P5
 *
 *    input         |  output
 * 0x00 = 00000000b -> 000000b = 0
 * 0x01 = 00000001b -> 101001b = 41
 * 0x02 = 00000010b -> 101010b = 42
 * 0x03 = 00000011b -> 000011b = 3
 * 0x04 = 00000100b -> 101011b = 43
 *                  .
 *                  .
 *                  .
 *                  .
 * 0xFE = 11111110b -> 110001b = 49
 * 0xFF = 11111111b -> 011000b = 24
*/
    index=0;
    for (int i=0; i<256; i++){
        if (index%per_line==0)
            printf("   ");

        D5 = (i>>0) & 1;
        D6 = (i>>1) & 1;
        D7 = (i>>2) & 1;
        D8 = (i>>3) & 1;
        D9 = (i>>4) & 1;
        D10 = (i>>5) & 1;
        D11 = (i>>6) & 1;
        P5 = (i>>7) & 1;

        A = D5 ^ D7 ^ D9 ^ D11;
        B = D6 ^ D7 ^ D10 ^ D11;
        C = D8 ^ D9 ^ D10 ^ D11;
        D = D5 ^ D6 ^ D7 ^ D8 ^ D9 ^ D10 ^ D11;
        E = P5;
        F = D5 ^ D6 ^ D7 ^ D8 ^ D9 ^ D10 ^ D11 ^ P5;

        int result= (F<<5) | (E<<4) | (D<<3) | (C<<2) | (B<<1) | (A<<0);
        print_space_before_number(result,3);
        if (i!=255)
            printf(",");

        index++;
        if (index%per_line==0)
            printf("\n");
    }

    printf("    },\n");
    printf("    {\n");
/*
 * byte N + 2 = D12 D13 D14 D15 D16 D17 D18 P6 (bitwise flipped)
 *
 * A = 1 ⊕D12 ⊕ D14 ⊕ D16 ⊕ D18
 * B = 1 ⊕D13 ⊕ D14 ⊕ D17 ⊕ D18
 * C = 1 ⊕D15 ⊕ D16 ⊕ D17 ⊕ D18
 * D = 1
 * E = 1 ⊕D12 ⊕ D13 ⊕ D14 ⊕ D15 ⊕ D16 ⊕ D17 ⊕ D18
 * F = 1 ⊕P6 ⊕ D12 ⊕ D13 ⊕ D14 ⊕ D15 ⊕ D16 ⊕ D17 ⊕ D18
 *
 *    input         |  output
 * 0x00 = 00000000b -> 111111b = 63
 * 0x01 = 00000001b -> 001110b = 14
 * 0x02 = 00000010b -> 001101b = 13
 * 0x03 = 00000011b -> 111100b = 60
 * 0x04 = 00000100b -> 001100b = 12
 *                  .
 *                  .
 *                  .
 *                  .
 * 0xFE = 11111110b -> 011110b = 30
 * 0xFF = 11111111b -> 101111b = 47
*/
    index=0;
    for (int i=0; i<256; i++){
        if (index%per_line==0)
            printf("   ");

        D12 = (i>>0) & 1;
        D13 = (i>>1) & 1;
        D14 = (i>>2) & 1;
        D15 = (i>>3) & 1;
        D16 = (i>>4) & 1;
        D17 = (i>>5) & 1;
        D18 = (i>>6) & 1;
        P6 = (i>>7) & 1;

        A = 1 ^ D12 ^ D14 ^ D16 ^ D18;
        B = 1 ^ D13 ^ D14 ^ D17 ^ D18;
        C = 1 ^ D15 ^ D16 ^ D17 ^ D18;
        D = 1;
        E = 1 ^ D12 ^ D13 ^ D14 ^ D15 ^ D16 ^ D17 ^ D18;
        F = 1 ^ P6 ^ D12 ^ D13 ^ D14 ^ D15 ^ D16 ^ D17 ^ D18;

        int result= (F<<5) | (E<<4) | (D<<3) | (C<<2) | (B<<1) | (A<<0);
        print_space_before_number(result,3);
        if (i!=255)
            printf(",");

        index++;
        if (index%per_line==0)
            printf("\n");
    }

    printf("    }\n");
    printf("};\n");
}

/*
 * Hamming 24/18 helper table
 * Input  = Byte N
 * Output = values D1-D4 extracted from input Byte N
 *
 * From : https://www.etsi.org/deliver/etsi_i_ets/300700_300799/300706/01_40_57/ets_300706e01o.pdf
 * layout of byte N in Hamming 24/18 code is : P4 D4 D3 D2 P3 D1 P2 P1
 * to reconstruct the data we need only the data bits: D4 D3 D2 D1
 * so  output = ((1<<2 & input)?(1<<0):0) | ((1<<4 & input)?(1<<1):0) | ((1<<5 & input)?(1<<2):0) | ((1<<6 & input)?(1<<3):0) ;
 *
 *    input         |  output
 * 0x00 = 00000000b -> 0000b = 0
 * 0x01 = 00000001b -> 0000b = 0
 * 0x02 = 00000010b -> 0000b = 0
 * 0x03 = 00000011b -> 0000b = 0
 * 0x04 = 00000100b -> 0001b = 1
 *                  .
 *                  .
 *                  .
 *                  .
 * 0xFE = 11111110b -> 1111b = 15
 * 0xFF = 11111111b -> 1111b = 15
*/
void calc_hamm24val()
{
    int index=0;
    int D4, D3, D2, D1;
    printf("const std::uint8_t hamm24val[256] =\n");
    printf("{\n");
    for (int i=0; i<256; i++){
        if (index%per_line==0)
            printf("    ");

        D1 = (i>>2) & 1;
        D2 = (i>>4) & 1;
        D3 = (i>>5) & 1;
        D4 = (i>>6) & 1;

        int result= (D4<<3) | (D3<<2) | (D2<<1) | (D1<<0);
        print_space_before_number(result,3);
        if (i!=255)
            printf(",");

        index++;
        if (index%per_line==0)
            printf("\n");
    }

    printf("};\n");
}

/*
 * Hamming 24/18 error table
 * Input  = indexed by FEDCBA binairy representation
 * Output = 0 if no double error, -1 if double error case
 *
 * From : https://www.etsi.org/deliver/etsi_i_ets/300700_300799/300706/01_40_57/ets_300706e01o.pdf
 * The position of the bit in error is calculated from:
 * 2^4 × Test E result + 2^3 × Test D result + 2^2 × Test C result + 2^1 × Test B result + 2^0 × Test A result
 * where a Test result = "1" if the odd parity test is failed.
 *
 *    input         |     results of tests         | output
 * 0x00 = 000000b -> A-E=all correct     F=correct ->  0
 * 0x01 = 000001b -> A-E=not all correct F=correct -> -1
 * 0x02 = 000010b -> A-E=not all correct F=correct -> -1
 * 0x03 = 000011b -> A-E=not all correct F=correct -> -1
 *                  .
 *                  .
 *                  .
 *                  .
 * 0x3E = 111110b -> A-E=not all correct F=not correct -> -1 (correct bit not found)
 * 0x3F = 111111b -> A-E=not all correct F=not correct -> -1 (correct bit not found)
*/
void calc_hamm24err()
{
    int index=0;
    int A, B, C, D, E, F;
    printf("const std::int8_t hamm24err[64] =\n");
    printf("{\n");
    for (int i=0; i<64; i++){
        if (index%per_line==0)
            printf("  ");

        A = (i>>0) & 1;
        B = (i>>1) & 1;
        C = (i>>2) & 1;
        D = (i>>3) & 1;
        E = (i>>4) & 1;
        F = (i>>5) & 1;

        int bit_in_error = (i & 0x1F);
        int result=-1;
        if ( ((A | B | C | D | E) == 0) && (F==0) ) {
            result = 0; // ABCDE all correct and F correct -> no issues 
        } else if ( ((A | B | C | D | E) == 0) && (F==1) ) {
            result = 0; // ABCDE all correct and F NOT correct -> P6 error
        } else if ( (bit_in_error!=0) && (F==0) ) {
            result = -1; // ABCDE not all correct and F is correct -> Double error
        } else {
            if (bit_in_error >= 24)
                result = -1; // bit in error is out of scope
            else
                result = 0;
        }

        if (index%4 == 0)
            printf(" ");
        printf("%s", (result==-1)?" -1":"  0");
        if (i!=255)
            printf(",");

        index++;
        if (index%per_line==0)
            printf("\n");
    }

    printf("};\n");
}

int data_bit_error_index(int fedcba)
{
    switch (fedcba){
        case 0x23 /*100011b*/ : return 1;  // ->  error in bit D1
        case 0x25 /*100101b*/ : return 2;  // ->  error in bit D2
        case 0x26 /*100110b*/ : return 3;  // ->  error in bit D3
        case 0x27 /*100111b*/ : return 4;  // ->  error in bit D4
        case 0x29 /*101001b*/ : return 5;  // ->  error in bit D5
        case 0x2A /*101010b*/ : return 6;  // ->  error in bit D6
        case 0x2B /*101011b*/ : return 7;  // ->  error in bit D7
        case 0x2C /*101100b*/ : return 8;  // ->  error in bit D8
        case 0x2D /*101101b*/ : return 9;  // ->  error in bit D9
        case 0x2E /*101110b*/ : return 10; // ->  error in bit D10
        case 0x2F /*101111b*/ : return 11; // ->  error in bit D11
        case 0x31 /*110001b*/ : return 12; // ->  error in bit D12
        case 0x32 /*110010b*/ : return 13; // ->  error in bit D13
        case 0x33 /*110011b*/ : return 14; // ->  error in bit D14
        case 0x34 /*110100b*/ : return 15; // ->  error in bit D15
        case 0x35 /*110101b*/ : return 16; // ->  error in bit D16
        case 0x36 /*110110b*/ : return 17; // ->  error in bit D17
        case 0x37 /*110111b*/ : return 18; // ->  error in bit D18
    }
    return -1;
}

/*
 * Hamming 24/18 error correction bitmask table
 * Input  = indexed by FEDCBA binairy representation
 * Output = bitmask to toggle the bit in error
 *
 * From : https://www.etsi.org/deliver/etsi_i_ets/300700_300799/300706/01_40_57/ets_300706e01o.pdf
 * The position of the bit in error is calculated from:
 * 2^4 × Test E result + 2^3 × Test D result + 2^2 × Test C result + 2^1 × Test B result + 2^0 × Test A result
 * where a Test result = "1" if the odd parity test is failed.
 *
 *    input         |   results of table in section 8.3  | output
 * 0x23 = 100011b   ->  error in bit D1                  -> 0x00001
 * 0x25 = 100101b   ->  error in bit D2                  -> 0x00002
 * 0x26 = 100110b   ->  error in bit D3                  -> 0x00004
 * 0x27 = 100111b   ->  error in bit D4                  -> 0x00008
 * 0x29 = 101001b   ->  error in bit D5                  -> 0x00010
 * 0x2A = 101010b   ->  error in bit D6                  -> 0x00020
 * 0x2B = 101011b   ->  error in bit D7                  -> 0x00040
 * 0x2C = 101100b   ->  error in bit D8                  -> 0x00080
 * 0x2D = 101101b   ->  error in bit D9                  -> 0x00100
 * 0x2E = 101110b   ->  error in bit D10                 -> 0x00200
 * 0x2F = 101111b   ->  error in bit D11                 -> 0x00400
 * 0x31 = 110001b   ->  error in bit D12                 -> 0x00800
 * 0x32 = 110010b   ->  error in bit D13                 -> 0x01000
 * 0x33 = 110011b   ->  error in bit D14                 -> 0x02000
 * 0x34 = 110100b   ->  error in bit D15                 -> 0x04000
 * 0x35 = 110101b   ->  error in bit D16                 -> 0x08000
 * 0x36 = 110110b   ->  error in bit D17                 -> 0x10000
 * 0x37 = 110111b   ->  error in bit D18                 -> 0x20000
*/
void calc_hamm24cor()
{
    int per_line=8;
    int index=0;
    int A, B, C, D, E, F;
    printf("const std::uint32_t hamm24cor[64] =\n");
    printf("{\n");
    for (int i=0; i<64; i++){
        if (index%per_line==0)
            printf("   ");

        A = (i>>0) & 1;
        B = (i>>1) & 1;
        C = (i>>2) & 1;
        D = (i>>3) & 1;
        E = (i>>4) & 1;
        F = (i>>5) & 1;

        int data_bit_in_error=data_bit_error_index(i);
        int result=0;
        if (data_bit_in_error!=-1)
            result=1<<(data_bit_in_error-1);

        printf(" 0x%05x",result); 
        if (i!=255)
            printf(",");

        index++;
        if (index%per_line==0)
            printf("\n");
    }

    printf("};\n");
}

/*
 * Simple Byte Flip LUT
 * Input = index
 * Output = input byte in reverse bit order
 *
 *    input         |     output
 * 0x00 = 00000000b -> 00000000b = 0x00
 * 0x01 = 00000001b -> 10000000b = 0x80
 * 0x02 = 00000010b -> 01000000b = 0x40
 * 0x03 = 00000011b -> 11000000b = 0xC0
 *                  .
 *                  .
 *                  .
 *                  .
 * 0xFE = 11111110b -> 01111111b = 0x7F
 * 0xFF = 11111111b -> 11111111b = 0xFF
*/
void calc_byteFlipLookupTable()
{
    int per_line=8;
    int index=0;
    int b1,b2,b3,b4,b5,b6,b7,b8;
    printf("const std::uint8_t byteFlipLookupTable[256] =\n");
    printf("{\n");
    for (int i=0; i<256; i++){
        if (index%per_line==0)
            printf("   ");

        b1 = (i>>0) & 1;
        b2 = (i>>1) & 1;
        b3 = (i>>2) & 1;
        b4 = (i>>3) & 1;
        b5 = (i>>4) & 1;
        b6 = (i>>5) & 1;
        b7 = (i>>6) & 1;
        b8 = (i>>7) & 1;

        int result= (b1<<7) | (b2<<6) | (b3<<5) | (b4<<4) | (b5<<3) | (b6<<2) | (b7<<1) | (b8<<0);
        printf(" 0x%02X",result); 
        if (i!=255)
            printf(",");

        index++;
        if (index%per_line==0)
            printf("\n");
    }

    printf("};\n");
}

/*
 * Simple Hamming(8,4) precalculated LUT decode table
 * Input  = encoded 8 bits (4 data bits, 4 hamming parity bits)
 * Output = decoded 4 bits data (or 0xFF in case of error)
 *
 * From : https://www.etsi.org/deliver/etsi_i_ets/300700_300799/300706/01_40_57/ets_300706e01o.pdf
 *
 * layout of the input dataword: P1 D1 P2 D2 P3 D3 P4 D4
 *
 * 0x00 = 00000000b -> D4-1=0000 P4-1=0000 -> ABC=111 D=1 -> 1 bit error in D1 -> 0001b = 0x01
 * 0x01 = 00000001b -> D4-1=1000 P4-1=0000 -> ABC=001 D=0 -> double error      ->         0xFF
 * 0x02 = 00000010b -> D4-1=0000 P4-1=1000 -> ABC=111 D=0 -> double error      ->         0xFF
 * 0x03 = 00000011b -> D4-1=1000 P4-1=1000 -> ABC=001 D=0 -> 1 bit error in P3 -> 1000b = 0x08
 * 0x04 = 00000100b -> D4-1=0100 P4-1=0000 -> ABC=010 D=0 -> double error      ->         0xFF
 * 0x05 = 00000101b -> D4-1=1100 P4-1=0000 -> ABC=100 D=1 -> error in P1       -> 1100b = 0x0C
 * 0x06 = 00000110b -> D4-1=0100 P4-1=1000 -> ABC=010 D=1 -> error in P2       -> 0100b = 0x04
 * 0x07 = 00000111b -> D4-1=1100 P4-1=1000 -> ABC=100 D=0 -> double error      ->         0xFF
 *                  .
 *                  .
 * 0xFE = 11111110b -> D4-1=0111 P4-1=1111 -> ABC=001 D=0 -> double error      ->         0xFF
 * 0xFF = 11111111b -> D4-1=1111 P4-1=1111 -> ABC=111 D=1 -> 1 bit error in D1 -> 1110b = 0x0E
*/
void calc_hamming84LookupTable()
{
    int per_line=8;
    int index=0;
    int D1, D2, D3, D4;
    int P1, P2, P3, P4;
    printf("const std::uint8_t hamming84LookupTable[256] =\n");
    printf("{\n");
    int hamming84LookupTable[256];

    // default value
    for (int i=0;i<256;i++)
        hamming84LookupTable[i]=0xFF;

    // calculate all values
    for (int i=0; i<16; i++){
        D1 = (i>>0) & 1;
        D2 = (i>>1) & 1;
        D3 = (i>>2) & 1;
        D4 = (i>>3) & 1;

        P1 = 1 ^ D1 ^ D3 ^ D4;
        P2 = 1 ^ D1 ^ D2 ^ D4;
        P3 = 1 ^ D1 ^ D2 ^ D3;
        P4 = 1 ^ P1 ^ D1 ^ P2 ^ D2 ^ P3 ^ D3 ^ D4;

        int result = (P1 << 7) | (D1 << 6) | (P2 << 5) | (D2 << 4) | (P3 << 3) | (D3 << 2) | (P4 << 1) | (D4 << 0);
        hamming84LookupTable[result]=i;

        for (int j = 0; j < 8; ++j) {
            int flipbit = (1 << j);
            hamming84LookupTable[result ^ flipbit] = i;
        }
    }

    for (int i=0; i<256; i++){
        if (index%per_line==0)
            printf("   ");

        printf(" 0x%02X",hamming84LookupTable[i]); 
        if (i!=255)
            printf(",");

        index++;
        if (index%per_line==0)
            printf("\n");
    }

    printf("};\n");
}

int main()
{
 calc_hamm24par();
 calc_hamm24val();
 calc_hamm24err();
 calc_hamm24cor();
 calc_byteFlipLookupTable();
 calc_hamming84LookupTable();
 return 0;
}
