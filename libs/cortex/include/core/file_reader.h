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
  file_reader.h
*/

#ifndef FILE_READER_H_
#define FILE_READER_H_

#include <sys/stat.h>

#include <string_buffer.h>

#include "global.h"
#include "seq.h"
#include "dB_graph.h"
#include "file_format.h"
#include "graph_info.h"
#include "db_variants.h"

extern int MAX_FILENAME_LENGTH;
extern int MAX_READ_LENGTH;
#define PROGRESS_STEP 50000


typedef enum {
  EValid                        = 0,
  ECannotReadMagicNumber        = 1,
  ECanReadMagicNumberButIsWrong = 2,
  ECannotReadBinversion         = 3,
  EValidButOldBinVersion        = 4,
  EInvalidBinversion            = 5,
  ECannotReadKmer               = 6,
  EWrongKmer                    = 7,
  ECannotReadNumBitfields       = 8,
  EWrongNumberBitfields         = 9,
  ECannotReadNumColours         = 10,
  EBadColours                   = 11,
  EFailedToReadReadLensAndCovgs = 12,
  ECannotReadEndOfHeaderMagicNumber = 13,
  EFailedToReadSampleIds        =14,
  EFailedToReadSampleIdsSeemsTooLong = 15,
  EFailedToReadSeqErrRates      =16,
  EFailedToReadErrorCleaningInfo=17,
  ECanReadEndOfHeaderMagicNumberButIsWrong = 18,
  EGarbage = 19,
  EBinaryHasTooManyColoursGivenFirstColour=20, //ie starting at colour 10, loading 100 colours but 110>NUMBER_OF_COLOURS-1
} BinaryHeaderErrorCode;
typedef struct {
  int version;
  int kmer_size;
  int number_of_bitfields;
  int number_of_colours;
  GraphInfo* ginfo;
} BinaryHeaderInfo;

boolean dir_exists(char* dir_to_check);

// mkpath - ensure all directories in path exist
// Returns 1 on success, 0 on failure
// Adapted from Jonathan Leffler http://stackoverflow.com/a/675193/431087
char mkpath(const char *path, mode_t mode);

StrBuf* file_reader_get_strbuf_of_dir_path(char* path);

boolean subsample_null();

boolean check_binary_signature_NEW(FILE * fp,int kmer_size, 
				   BinaryHeaderInfo* binfo, BinaryHeaderErrorCode* ecode,
				   int first_colour_loading_into);

boolean query_binary_NEW(FILE * fp, BinaryHeaderInfo* binfo, BinaryHeaderErrorCode* ecode, int first_colour_loading_into);

boolean get_extra_data_from_header(FILE * fp, BinaryHeaderInfo* binfo, BinaryHeaderErrorCode* ecode, int first_colour_loading_into);

boolean get_read_lengths_and_total_seqs_from_header(FILE * fp, BinaryHeaderInfo* binfo, BinaryHeaderErrorCode* ecode,
						    int first_colour_loading_into);

boolean  get_binversion6_extra_data(FILE * fp, BinaryHeaderInfo* binfo, BinaryHeaderErrorCode* ecode, int first_colour_loading_into);
boolean read_next_error_cleaning_object(FILE* fp, ErrorCleaning* cl);

long long load_multicolour_binary_from_filename_into_graph(char* filename,  dBGraph* db_graph, GraphInfo* ginfo, int* num_cols_in_loaded_binary);

void load_se_seq_data_into_graph_colour(
					const char *file_path,
					char quality_cutoff, int homopolymer_cutoff, boolean remove_dups_se,
					char ascii_fq_offset, int colour_index, dBGraph *db_graph,
					unsigned long long *bad_reads, unsigned long long *dup_reads,
					uint64_t *bases_read, 
					uint64_t *bases_loaded,
					uint64_t *readlen_count_array, 
					uint64_t readlen_count_array_size,
					boolean (*subsample_func)(),
					boolean only_load_pre_existing_kmers,
					boolean print_progress,
					uint64_t* count_so_far,
					uint64_t denom_for_progress);



void load_pe_seq_data_into_graph_colour(
  const char *file_path1, const char *file_path2,
  char quality_cutoff, int homopolymer_cutoff, boolean remove_dups_pe,
  char ascii_fq_offset, int colour_index, dBGraph *db_graph,
  unsigned long long *bad_reads, unsigned long long *dup_reads,
  uint64_t *bases_read, uint64_t *bases_loaded,
  uint64_t *readlen_count_array, uint64_t readlen_count_array_size,
  boolean (*subsample_func)(), boolean only_load_pre_existing_kmers );

void load_se_filelist_into_graph_colour(
					char* se_filelist_path,
					int qual_thresh, int homopol_limit, boolean remove_dups_se,
					char ascii_fq_offset, int colour, dBGraph* db_graph, char is_colour_list,
					unsigned int *total_files_loaded,
					unsigned long long *total_bad_reads, 
					unsigned long long *total_dup_reads,
					uint64_t *total_bases_read, 
					uint64_t *total_bases_loaded,
					uint64_t *readlen_count_array, 
					uint64_t readlen_count_array_size,
					boolean (*subsample_func)(),
					boolean only_load_pre_existing_kmers,
					boolean print_progress,
					uint64_t* count_so_far,
					uint64_t denom_for_progress);


void load_pe_filelists_into_graph_colour(
					 char* pe_filelist_path1, char* pe_filelist_path2,
					 int qual_thresh, int homopol_limit, boolean remove_dups_pe,
					 char ascii_fq_offset, int colour, dBGraph* db_graph, char is_colour_lists,
					 unsigned int *total_file_pairs_loaded,
					 unsigned long long *total_bad_reads, 
					 unsigned long long *total_dup_reads,
					 uint64_t *total_bases_read, 
					 uint64_t *total_bases_loaded,
					 uint64_t *readlen_count_array, 
					 uint64_t readlen_count_array_size,
					 boolean (*subsample_func)(), 
					 boolean only_load_pre_existing_kmers );

// End of loading sequence data

void initialise_binary_header_info(BinaryHeaderInfo* binfo, GraphInfo* ginfo);


void  load_kmers_from_sliding_window_into_graph_marking_read_starts_of_specific_person_or_pop(
  KmerSlidingWindowSet * windows, 
  boolean* prev_full_ent, //boolean* full_ent,
  uint64_t* bases_loaded, 
  boolean mark_read_starts, 
  dBGraph* db_graph,
  int index, 
  uint64_t** read_len_count_array);


//pass in a single kmer sliding window and the Sequence* it was derived from. Will find the nodes correspinding to this seqeunce
//and put them in array. Also will check that edges exist as expected from the Sequence*
void load_kmers_from_sliding_window_into_array(KmerSlidingWindow* kmer_window, dBGraph* db_graph, 
					       dBNode** array_nodes, Orientation* array_orientations, 
					       int max_array_size, 
					       boolean require_nodes_to_lie_in_given_colour, int colour);


//use preallocated sliding window, and get all the kmers from the passed-in sequence. Any kmer that would have contained an N is returned as NULL
int get_single_kmer_sliding_window_from_sequence(char * seq, int length, short kmer_size, KmerSlidingWindow* kmer_window, dBGraph* db_graph);





// gets the next number_of_bases_to_load bases from fasta file, and returns them in the array of nodes.
// assumes this file has already been loaded into the graph.
// returns the number of nodes loaded. If this is less than what you asked for, you know it has hit the end of the file.
// We expect this to be used as follows:
// repeated calls of this function load etc into the LAST number_of_bases_to_load places of the relevant arrays

int load_seq_into_array(FILE* chrom_fptr, int number_of_nodes_to_load, int length_of_arrays, 
			dBNode * * path_nodes, Orientation * path_orientations, Nucleotide * path_labels, char* path_string,
			Sequence* seq, KmerSlidingWindow* kmer_window, boolean expecting_new_fasta_entry, dBGraph * db_graph);





//functions for loading multicolour graphs
long long load_multicolour_binary_from_filename_into_graph(char* filename,  dBGraph* db_graph, GraphInfo* ginfo, int* num_cols_in_loaded_binary); 

// last arg is to load the "union" of graphs. This is not a special case of load_multicolour
long long load_single_colour_binary_data_from_filename_into_graph(char* filename,  dBGraph* db_graph, 
								  GraphInfo* ginfo, 
								  boolean all_entries_are_unique, 
								  int colour_loading_into,
								  boolean only_load_kmers_already_in_hash, int colour_clean,
								  boolean load_all_kmers_but_only_increment_covg_on_new_ones);
								  

long long load_all_binaries_for_given_person_given_filename_of_file_listing_their_binaries(char* filename,  dBGraph* db_graph, GraphInfo* db_graph_info,
											   boolean all_entries_are_unique, int index,
											   boolean only_load_kmers_already_in_hash, int colour_clean,
											   boolean load_all_kmers_but_only_increment_covg_on_new_ones);

long long load_population_as_binaries_from_graph(char* filename, int first_colour,boolean about_to_load_first_binary_into_empty_graph, 
						 dBGraph* db_graph, GraphInfo* db_graph_info,
						 boolean only_load_kmers_already_in_hash, int colour_clean,
						 boolean load_all_kmers_but_only_increment_covg_on_new_ones);

void dump_successive_cleaned_binaries(char* filename, int in_colour, int clean_colour, char* suffix, dBGraph* db_graph, GraphInfo* db_graph_info );



//functions for comparing graph with reference, or comparing reads with the graph

void read_fastq_and_print_reads_that_lie_in_graph(FILE* fp, FILE* fout, int (* file_reader)(FILE * fp, Sequence * seq, int max_read_length, boolean new_entry, boolean * full_entry),
                                                  long long * bad_reads, int max_read_length, dBGraph * db_graph,
                                                  boolean is_for_testing, char** for_test_array_of_clean_reads, int* for_test_index);


/*
// DEV: flagged for removal -- not used, uses old file reader
void read_fastq_and_print_subreads_that_lie_in_graph_breaking_at_edges_or_kmers_not_in_graph(
  FILE* fp, FILE* fout,
  int (* file_reader)(FILE * fp, Sequence * seq, int max_read_length,
                      boolean new_entry, boolean * full_entry), 
  long long * bad_reads, int max_read_length, dBGraph * db_graph, 
  int index, boolean is_for_testing,
  char** for_test_array_of_clean_reads, int* for_test_index);
*/

int get_sliding_windows_from_sequence_breaking_windows_when_sequence_not_in_graph(char * seq,  char * qualities, int length, char quality_cut_off, 
										  KmerSlidingWindowSet * windows, int max_windows, int max_kmers, dBGraph* db_graph);

int get_sliding_windows_from_sequence_requiring_entire_seq_and_edges_to_lie_in_graph(char * seq,  char * qualities, int length, char quality_cut_off, 
										     KmerSlidingWindowSet * windows, int max_windows, int max_kmers, dBGraph* db_graph, int index); 

void read_fastq_and_print_reads_that_lie_in_graph(FILE* fp, FILE* fout, int (* file_reader)(FILE * fp, Sequence * seq, int max_read_length, boolean new_entry, boolean * full_entry), 
						  long long * bad_reads, int max_read_length, dBGraph * db_graph,
						  boolean is_for_testing, char** for_test_array_of_clean_reads, int* for_test_index);




//gets number_of_bases_to_load's worth of kmers, and returns the corresponding nodes, orientations etc in he array passed in.

int load_seq_into_array(FILE* chrom_fptr, int number_of_bases_to_load, int length_of_arrays,
			    dBNode * * path_nodes, Orientation * path_orientations, Nucleotide * path_labels, char* path_string,
			    Sequence* seq, KmerSlidingWindow* kmer_window, boolean expecting_new_fasta_entry,  dBGraph * db_graph);


int align_next_read_to_graph_and_return_node_array(FILE* fp, int max_read_length, dBNode** array_nodes, Orientation* array_orientations, 
						   boolean require_nodes_to_lie_in_given_colour, boolean* full_entry,
						   int (* file_reader)(FILE * fp, Sequence * seq, int max_read_length,boolean new_entry, boolean * full_entry), 
						   Sequence* seq, KmerSlidingWindow* kmer_window,dBGraph * db_graph, int colour);


int given_prev_kmer_align_next_read_to_graph_and_return_node_array_including_overlap(char* prev_kmer, FILE* fp, int max_read_length, 
										     dBNode** array_nodes, Orientation* array_orientations, 
										     boolean require_nodes_to_lie_in_given_colour,
										     boolean* full_entry,
										     int (* file_reader)(FILE * fp, Sequence * seq, 
													 int max_read_length,boolean new_entry, 
													 boolean * full_entry), 
										     Sequence* seq, Sequence* seq_inc_prev_kmer, 
										     KmerSlidingWindow* kmer_window,dBGraph * db_graph, int colour);



//void print_binary_signature(FILE * fp,int kmer_size, int num_cols, int* array_mean_readlens, long long* array_total_seq);
void print_binary_signature_NEW(FILE * fp,int kmer_size, int num_cols, GraphInfo* ginfo, int first_col, int version);

//boolean check_binary_signature(FILE * fp,int kmer_size, int bin_version, int* number_of_colours_in_binary, int** array_mean_readlens, long long** array_total_seqs, int *return_binversion);
boolean check_binary_signature_NEW(FILE * fp,int kmer_size, 
				   BinaryHeaderInfo* binfo, BinaryHeaderErrorCode* ecode,
				   int first_colour_loading_into);

boolean query_binary_NEW(FILE * fp, BinaryHeaderInfo* binfo, BinaryHeaderErrorCode* ecode,
			 int first_colour_loading_into);

void print_error_cleaning_object(FILE* fp, GraphInfo* ginfo, int colour);

boolean get_extra_data_from_header(FILE * fp, BinaryHeaderInfo* binfo, 
				   BinaryHeaderErrorCode* ecode, int first_colour_loading_into);
boolean get_read_lengths_and_total_seqs_from_header(FILE * fp, BinaryHeaderInfo* binfo, 
						    BinaryHeaderErrorCode* ecode, int first_colour_loading_into);

boolean  get_binversion6_extra_data(FILE * fp, BinaryHeaderInfo* binfo, BinaryHeaderErrorCode* ecode, int first_colour_loading_into);
boolean read_next_error_cleaning_object(FILE* fp, ErrorCleaning* cl);


int load_paths_from_filelist(char* filelist_path, char** path_array);

boolean check_colour_list(char* filename, int kmer);
boolean check_ctx_list(char* filename, int kmer);

typedef struct
{
  Sequence* seq;
  KmerSlidingWindow* kmer_window;
  CovgArray* working_ca;
  dBNode** array_nodes;
  Orientation* array_or;
  int max_read_length;
} ReadingUtils;

ReadingUtils* alloc_reading_utils(int max_read_len, int kmer_size);
void free_reading_utils(ReadingUtils* rutils);
void reset_reading_utils(ReadingUtils* rutils);
uint64_t count_reads_in_file(StrBuf* file);
uint64_t count_all_reads(StrBuf* path, //may be file or list of files
			 boolean is_list);


#endif /* FILE_READER_H_ */
