/*
 * 
 * Copyright 2015 Zamin Iqbal (zam@well.ox.ac.uk)
 *
 */
/*
  test_file_reader.h
*/

#ifndef TEST_FILE_READER_H_
#define TEST_FILE_READER_H_

#include "global.h"

void test_coverage_is_correctly_counted_on_loading_from_file();
void test_dump_load_sv_trio_binary();
void test_load_singlecolour_binary();
void test_load_individual_binaries_into_sv_trio();
void test_getting_sliding_windows_where_you_break_at_kmers_not_in_db_graph();
void test_get_sliding_windows_from_sequence_requiring_entire_seq_and_edges_to_lie_in_graph();
void test_dumping_of_clean_fasta();
void test_loading_of_paired_end_reads_removing_duplicates();
void test_loading_of_single_ended_reads_removing_duplicates();
void test_load_seq_into_array();
void test_align_next_read_to_graph_and_return_node_array();
void test_read_next_variant_from_full_flank_file();
void test_read_next_variant_from_full_flank_file_2();
void test_read_next_variant_from_full_flank_file_3();
void test_read_next_variant_from_full_flank_file_4();
void test_getting_readlength_distribution();
void test_loading_binary_data_iff_it_overlaps_a_fixed_colour();
void test_load_binversion5_binary();

#endif /* TEST_FILE_READER_H_ */
