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


#include "Hamming.hpp"

namespace ttxdecoder
{

namespace
{

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
const std::uint8_t hamm24par[3][256] =
{
    {
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
     0, 33, 34,  3, 35,  2,  1, 32, 36,  5,  6, 39,  7, 38, 37,  4,
    37,  4,  7, 38,  6, 39, 36,  5,  1, 32, 35,  2, 34,  3,  0, 33,
    38,  7,  4, 37,  5, 36, 39,  6,  2, 35, 32,  1, 33,  0,  3, 34,
     3, 34, 33,  0, 32,  1,  2, 35, 39,  6,  5, 36,  4, 37, 38,  7,
    39,  6,  5, 36,  4, 37, 38,  7,  3, 34, 33,  0, 32,  1,  2, 35,
     2, 35, 32,  1, 33,  0,  3, 34, 38,  7,  4, 37,  5, 36, 39,  6,
     1, 32, 35,  2, 34,  3,  0, 33, 37,  4,  7, 38,  6, 39, 36,  5,
    36,  5,  6, 39,  7, 38, 37,  4,  0, 33, 34,  3, 35,  2,  1, 32,
    40,  9, 10, 43, 11, 42, 41,  8, 12, 45, 46, 15, 47, 14, 13, 44,
    13, 44, 47, 14, 46, 15, 12, 45, 41,  8, 11, 42, 10, 43, 40,  9,
    14, 47, 44, 13, 45, 12, 15, 46, 42, 11,  8, 41,  9, 40, 43, 10,
    43, 10,  9, 40,  8, 41, 42, 11, 15, 46, 45, 12, 44, 13, 14, 47,
    15, 46, 45, 12, 44, 13, 14, 47, 43, 10,  9, 40,  8, 41, 42, 11,
    42, 11,  8, 41,  9, 40, 43, 10, 14, 47, 44, 13, 45, 12, 15, 46,
    41,  8, 11, 42, 10, 43, 40,  9, 13, 44, 47, 14, 46, 15, 12, 45,
    12, 45, 46, 15, 47, 14, 13, 44, 40,  9, 10, 43, 11, 42, 41,  8
    },
    {
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
     0, 41, 42,  3, 43,  2,  1, 40, 44,  5,  6, 47,  7, 46, 45,  4,
    45,  4,  7, 46,  6, 47, 44,  5,  1, 40, 43,  2, 42,  3,  0, 41,
    46,  7,  4, 45,  5, 44, 47,  6,  2, 43, 40,  1, 41,  0,  3, 42,
     3, 42, 41,  0, 40,  1,  2, 43, 47,  6,  5, 44,  4, 45, 46,  7,
    47,  6,  5, 44,  4, 45, 46,  7,  3, 42, 41,  0, 40,  1,  2, 43,
     2, 43, 40,  1, 41,  0,  3, 42, 46,  7,  4, 45,  5, 44, 47,  6,
     1, 40, 43,  2, 42,  3,  0, 41, 45,  4,  7, 46,  6, 47, 44,  5,
    44,  5,  6, 47,  7, 46, 45,  4,  0, 41, 42,  3, 43,  2,  1, 40,
    48, 25, 26, 51, 27, 50, 49, 24, 28, 53, 54, 31, 55, 30, 29, 52,
    29, 52, 55, 30, 54, 31, 28, 53, 49, 24, 27, 50, 26, 51, 48, 25,
    30, 55, 52, 29, 53, 28, 31, 54, 50, 27, 24, 49, 25, 48, 51, 26,
    51, 26, 25, 48, 24, 49, 50, 27, 31, 54, 53, 28, 52, 29, 30, 55,
    31, 54, 53, 28, 52, 29, 30, 55, 51, 26, 25, 48, 24, 49, 50, 27,
    50, 27, 24, 49, 25, 48, 51, 26, 30, 55, 52, 29, 53, 28, 31, 54,
    49, 24, 27, 50, 26, 51, 48, 25, 29, 52, 55, 30, 54, 31, 28, 53,
    28, 53, 54, 31, 55, 30, 29, 52, 48, 25, 26, 51, 27, 50, 49, 24
    },
    {
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
    63, 14, 13, 60, 12, 61, 62, 15, 11, 58, 57,  8, 56,  9, 10, 59,
    10, 59, 56,  9, 57,  8, 11, 58, 62, 15, 12, 61, 13, 60, 63, 14,
     9, 56, 59, 10, 58, 11,  8, 57, 61, 12, 15, 62, 14, 63, 60, 13,
    60, 13, 14, 63, 15, 62, 61, 12,  8, 57, 58, 11, 59, 10,  9, 56,
     8, 57, 58, 11, 59, 10,  9, 56, 60, 13, 14, 63, 15, 62, 61, 12,
    61, 12, 15, 62, 14, 63, 60, 13,  9, 56, 59, 10, 58, 11,  8, 57,
    62, 15, 12, 61, 13, 60, 63, 14, 10, 59, 56,  9, 57,  8, 11, 58,
    11, 58, 57,  8, 56,  9, 10, 59, 63, 14, 13, 60, 12, 61, 62, 15,
    31, 46, 45, 28, 44, 29, 30, 47, 43, 26, 25, 40, 24, 41, 42, 27,
    42, 27, 24, 41, 25, 40, 43, 26, 30, 47, 44, 29, 45, 28, 31, 46,
    41, 24, 27, 42, 26, 43, 40, 25, 29, 44, 47, 30, 46, 31, 28, 45,
    28, 45, 46, 31, 47, 30, 29, 44, 40, 25, 26, 43, 27, 42, 41, 24,
    40, 25, 26, 43, 27, 42, 41, 24, 28, 45, 46, 31, 47, 30, 29, 44,
    29, 44, 47, 30, 46, 31, 28, 45, 41, 24, 27, 42, 26, 43, 40, 25,
    30, 47, 44, 29, 45, 28, 31, 46, 42, 27, 24, 41, 25, 40, 43, 26,
    43, 26, 25, 40, 24, 41, 42, 27, 31, 46, 45, 28, 44, 29, 30, 47
    }
};

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
const std::uint8_t hamm24val[256] =
{
      0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,
      2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,  4,  5,  5,  5,  5,  4,  4,  4,  4,  5,  5,  5,  5,
      6,  6,  6,  6,  7,  7,  7,  7,  6,  6,  6,  6,  7,  7,  7,  7,
      8,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  8,  9,  9,  9,  9,
     10, 10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11,
     12, 12, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13,
     14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14, 15, 15, 15, 15,
      0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,
      2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,  4,  5,  5,  5,  5,  4,  4,  4,  4,  5,  5,  5,  5,
      6,  6,  6,  6,  7,  7,  7,  7,  6,  6,  6,  6,  7,  7,  7,  7,
      8,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  8,  9,  9,  9,  9,
     10, 10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11,
     12, 12, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13,
     14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14, 15, 15, 15, 15
};

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
const std::int8_t hamm24err[64] =
{
     0, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,
    -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,
     0,  0,  0,  0,   0,  0,  0,  0,   0,  0,  0,  0,   0,  0,  0,  0,
     0,  0,  0,  0,   0,  0,  0,  0,  -1, -1, -1, -1,  -1, -1, -1, -1,
};

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
const std::uint32_t hamm24cor[64] =
{
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00001, 0x00000, 0x00002, 0x00004, 0x00008,
    0x00000, 0x00010, 0x00020, 0x00040, 0x00080, 0x00100, 0x00200, 0x00400,
    0x00000, 0x00800, 0x01000, 0x02000, 0x04000, 0x08000, 0x10000, 0x20000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
};

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
const std::uint8_t byteFlipLookupTable[256] =
{
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};


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
const std::uint8_t hamming84LookupTable[256] =
{
    0x01, 0xFF, 0xFF, 0x08, 0xFF, 0x0C, 0x04, 0xFF,
    0xFF, 0x08, 0x08, 0x08, 0x06, 0xFF, 0xFF, 0x08,
    0xFF, 0x0A, 0x02, 0xFF, 0x06, 0xFF, 0xFF, 0x0F,
    0x06, 0xFF, 0xFF, 0x08, 0x06, 0x06, 0x06, 0xFF,
    0xFF, 0x0A, 0x04, 0xFF, 0x04, 0xFF, 0x04, 0x04,
    0x00, 0xFF, 0xFF, 0x08, 0xFF, 0x0D, 0x04, 0xFF,
    0x0A, 0x0A, 0xFF, 0x0A, 0xFF, 0x0A, 0x04, 0xFF,
    0xFF, 0x0A, 0x03, 0xFF, 0x06, 0xFF, 0xFF, 0x0E,
    0x01, 0x01, 0x01, 0xFF, 0x01, 0xFF, 0xFF, 0x0F,
    0x01, 0xFF, 0xFF, 0x08, 0xFF, 0x0D, 0x05, 0xFF,
    0x01, 0xFF, 0xFF, 0x0F, 0xFF, 0x0F, 0x0F, 0x0F,
    0xFF, 0x0B, 0x03, 0xFF, 0x06, 0xFF, 0xFF, 0x0F,
    0x01, 0xFF, 0xFF, 0x09, 0xFF, 0x0D, 0x04, 0xFF,
    0xFF, 0x0D, 0x03, 0xFF, 0x0D, 0x0D, 0xFF, 0x0D,
    0xFF, 0x0A, 0x03, 0xFF, 0x07, 0xFF, 0xFF, 0x0F,
    0x03, 0xFF, 0x03, 0x03, 0xFF, 0x0D, 0x03, 0xFF,
    0xFF, 0x0C, 0x02, 0xFF, 0x0C, 0x0C, 0xFF, 0x0C,
    0x00, 0xFF, 0xFF, 0x08, 0xFF, 0x0C, 0x05, 0xFF,
    0x02, 0xFF, 0x02, 0x02, 0xFF, 0x0C, 0x02, 0xFF,
    0xFF, 0x0B, 0x02, 0xFF, 0x06, 0xFF, 0xFF, 0x0E,
    0x00, 0xFF, 0xFF, 0x09, 0xFF, 0x0C, 0x04, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x0E,
    0xFF, 0x0A, 0x02, 0xFF, 0x07, 0xFF, 0xFF, 0x0E,
    0x00, 0xFF, 0xFF, 0x0E, 0xFF, 0x0E, 0x0E, 0x0E,
    0x01, 0xFF, 0xFF, 0x09, 0xFF, 0x0C, 0x05, 0xFF,
    0xFF, 0x0B, 0x05, 0xFF, 0x05, 0xFF, 0x05, 0x05,
    0xFF, 0x0B, 0x02, 0xFF, 0x07, 0xFF, 0xFF, 0x0F,
    0x0B, 0x0B, 0xFF, 0x0B, 0xFF, 0x0B, 0x05, 0xFF,
    0xFF, 0x09, 0x09, 0x09, 0x07, 0xFF, 0xFF, 0x09,
    0x00, 0xFF, 0xFF, 0x09, 0xFF, 0x0D, 0x05, 0xFF,
    0x07, 0xFF, 0xFF, 0x09, 0x07, 0x07, 0x07, 0xFF,
    0xFF, 0x0B, 0x03, 0xFF, 0x07, 0xFF, 0xFF, 0x0E
};

template <class T>
std::uint8_t doHamming84(T value)
{
    return hamming84LookupTable[value & 0xFF];
}

template <class T>
std::uint8_t doByteFlip(T value)
{
    return byteFlipLookupTable[value & 0xFF];
}

template <class T>
T doXor(T a, T b)
{
    return a ^ b;
}

template <int BIT,class T>
std::uint8_t bit(T value)
{
    return value & (1 << (BIT-1)) ? 1 : 0;
}

bool parityCheck( std::uint8_t byte )
{
    static const uint8_t PARITY_BIT = 8;

    std::uint8_t temp = bit<1>(byte);
    temp = doXor(temp, bit<2>(byte));
    temp = doXor(temp, bit<3>(byte));
    temp = doXor(temp, bit<4>(byte));
    temp = doXor(temp, bit<5>(byte));
    temp = doXor(temp, bit<6>(byte));
    temp = doXor(temp, bit<7>(byte));
    temp = doXor(temp, bit<PARITY_BIT>(byte));

    return temp == 1;
}

} // namespace <anonymous>

std::int32_t Hamming::decode2418(std::uint8_t byte1,
                                 std::uint8_t byte2,
                                 std::uint8_t byte3)
{
    std::uint8_t p[3];

    p[0] = doByteFlip(byte1);
    p[1] = doByteFlip(byte2);
    p[2] = doByteFlip(byte3);

    int e = hamm24par[0][p[0]]
        ^ hamm24par[1][p[1]]
        ^ hamm24par[2][p[2]];

    int x = hamm24val[p[0]]
        + (p[1] & 127) * 16
        + (p[2] & 127) * 2048;

    return (x ^ hamm24cor[e]) | hamm24err[e];
}

std::int8_t Hamming::decode84(std::uint8_t byte1)
{
    return doHamming84(byte1);
}

std::int8_t Hamming::decodeParity(std::uint8_t byte1)
{
    static const uint8_t PARITY_BIT_CLEAR_MASK = 0x7f;

    std::uint8_t dataByte = doByteFlip( byte1 );

    /* the parity check */
    if( parityCheck( dataByte ) )
    {
        /* mask the parity bit and copy the byte to the data struct */
        return static_cast<std::int8_t>( dataByte & PARITY_BIT_CLEAR_MASK );
    }
    else
    {
        return -1;
    }
}

} // namespace ttxdecoder
