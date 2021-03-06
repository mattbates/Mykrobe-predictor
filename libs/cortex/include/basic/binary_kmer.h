/*
 * CORTEX project contacts:  
 * 		M. Caccamo (mario.caccamo@tgac.ac.uk) and 
 * 		Z. Iqbal (zam@well.ox.ac.uk)
 *
 * **********************************************************************
 *
 * The MIT License (MIT)
 * Copyright (c) 2009-2014 <Z. Iqbal and M. Caccamo>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * **********************************************************************
 */
/*
  binary_kmer.h - basic manipulation of binary kmers
*/

#ifndef BINARY_KMER_H_
#define BINARY_KMER_H_

#include <inttypes.h>

#include "global.h"
#include "event_encoding.h"

#define BINVERSION 6

typedef uint64_t bitfield_of_64bits;

// Think of NUMBER_OF_BITFIELDS_IN_BINARY_KMER as the number of uint64_t we
// encode the kmer in
typedef bitfield_of_64bits BinaryKmer[NUMBER_OF_BITFIELDS_IN_BINARY_KMER];

#define BINARY_KMER_BYTES ((NUMBER_OF_BITFIELDS_IN_BINARY_KMER) * sizeof(uint64_t))


typedef struct
{
  int nkmers;
  BinaryKmer * kmer;
} KmerSlidingWindow;


//a set of KmerArrays
typedef struct
{
  int nwindows;
  int max_nwindows;
  KmerSlidingWindow * window; 
} KmerSlidingWindowSet;


// basic BinaryKmer operations
void    binary_kmer_initialise_to_zero(BinaryKmer* bkmer);
void    binary_kmer_assignment_operator(BinaryKmer left, BinaryKmer right);
void    binary_kmer_set_all_bitfields(BinaryKmer assignee, bitfield_of_64bits val);
boolean binary_kmer_comparison_operator(const BinaryKmer const left, const BinaryKmer const right);
boolean binary_kmer_less_than(const BinaryKmer const left, const BinaryKmer const right,short kmer_size);
void    binary_kmer_right_shift_one_base(BinaryKmer kmer);
void    binary_kmer_left_shift_one_base(BinaryKmer kmer, short kmer_size);
void    binary_kmer_left_shift_one_base_and_insert_new_base_at_right_end(BinaryKmer* bkmer, Nucleotide n, short kmer_size);


char * nucleotides_to_string(Nucleotide * nucleotides, int length, char * string);

//get overlapping kmers from sequence
//int get_sliding_windows_from_sequence(char * sequence,char * qualities, int length, char quality_cutoff, short kmer_size, KmerSlidingWindowSet * windows, int max_windows, int max_kmers); 
int get_sliding_windows_from_sequence(char * seq,  char * qualities, int length, char quality_cut_off, short kmer_size, KmerSlidingWindowSet * windows,
                                      int max_windows, int max_kmers, boolean break_homopolymers, int homopolymer_cutoff);


char * binary_kmer_to_seq(BinaryKmer* kmer, short kmer_size, char * seq);

BinaryKmer* seq_to_binary_kmer(char * seq, short kmer_size, BinaryKmer* prealloced_kmer);

char * seq_reverse_complement(char * in, int length, char * out);

BinaryKmer* binary_kmer_reverse_complement(BinaryKmer* kmer, short kmer_size, BinaryKmer* prealloc_reverse_kmer);

Nucleotide binary_kmer_get_first_nucleotide(BinaryKmer* kmer, short kmer_size);
Nucleotide binary_kmer_get_last_nucleotide(BinaryKmer* kmer);

char reverse_char_nucleotide(char c);

void binary_kmer_alloc_kmers_set(KmerSlidingWindowSet * windows, int max_windows, int max_kmers);
void binary_kmer_free_kmers(KmerSlidingWindow * *);
void binary_kmer_free_kmers_set(KmerSlidingWindowSet * *);

void nucleotide_iterator(void (*f)(Nucleotide));
void nucleotide_iterator_orientation(void (*f)(Nucleotide n,Orientation o));

#endif /* BINARY_KMER_H_ */
