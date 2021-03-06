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
  db_differentiation.c
*/

#include <string.h>
#include <string_buffer.h>
#include "db_differentiation.h"
#include "build.h"



void align_list_of_fastaq_to_graph_and_print_coverages_in_all_colours(FileFormat format, char* list_of_fastaq, int max_read_length, 
								      int* array_of_colours, char** array_of_names_of_colours,
								      int num_of_colours, dBGraph* db_graph,int fastq_ascii_offset,
								      boolean is_for_testing, char** for_test_array_of_strings, int* for_test_index,
								      boolean mark_nodes_for_dumping)
{

  if ( (format != FASTA) && (format !=FASTQ) )
    {
      die("Calling align_list_of_fastaq_to_graph_and_print_coverages_in_all_colours "
          "with file format not set to fasta or fastq");
    }

  //For each file in list_of_fasta, go through the reads, and for each read,
  // print one  "coverage read" per colour (space separated)
  // e.g. for a read print
  //    >read_id colour 0
  //    coverages of each of the nodes in the ref (colour 0)
  //   >read_id colour 1
  //     ... covgs in colour 1
  //   >read_id colour 2
  //     ... 
	
  
  //----------------------------------
  // allocate the memory used to read the sequences
  //----------------------------------
  Sequence * seq = malloc(sizeof(Sequence));
  if (seq == NULL){
    die("Out of memory trying to allocate Sequence");
  }
  alloc_sequence(seq,max_read_length,LINE_MAX);
  
  //We are going to load all the bases into a single sliding window 
  KmerSlidingWindow* kmer_window = malloc(sizeof(KmerSlidingWindow));
  if (kmer_window==NULL)
    {
      die("Failed to malloc kmer sliding window in "
          "align_list_of_fastaq_to_graph_and_print_coverages_in_all_colours. Exit.");
    }
  

  //  kmer_window->kmer = (BinaryKmer*) malloc(sizeof(BinaryKmer)*(max_read_length-db_graph->kmer_size-1));
  kmer_window->kmer = (BinaryKmer*) malloc(sizeof(BinaryKmer)*(max_read_length-db_graph->kmer_size+1));
  if (kmer_window->kmer==NULL)
    {
      die("Failed to malloc kmer_window->kmer in "
          "align_list_of_fastaq_to_graph_and_print_coverages_in_all_colours. Exit.");
    }
  kmer_window->nkmers=0;
  
  
  //end of intialisation 
	  
	  
  //create file readers
  int file_reader_fasta(FILE * fp, Sequence * seq, int max_read_length, boolean new_entry, boolean * full_entry){
    long long ret;
    int offset = 0;
    if (new_entry == false){
      offset = db_graph->kmer_size;
      //die("new_entry must be true in hsi test function");
    }
    ret =  read_sequence_from_fasta(fp,seq,max_read_length,new_entry,full_entry,offset);
    
    return ret;
  }

  int file_reader_fastq(FILE * fp, Sequence * seq, int max_read_length, boolean new_entry, boolean * full_entry){
    * full_entry = true;

    if (new_entry!= true){
      die("new_entry has to be true for fastq");
    }

    return read_sequence_from_fastq(fp,seq,max_read_length,fastq_ascii_offset);
  }

  

  dBNode** array_nodes = (dBNode**) malloc(sizeof(dBNode*) * (max_read_length+db_graph->kmer_size+1) );
  Orientation*  array_or = (Orientation*) malloc(sizeof(Orientation)*(max_read_length+db_graph->kmer_size+1) );
  if ( (array_nodes==NULL) || (array_or==NULL) )
    {
      die("Unable to malloc arrays for alignment");
    }
  
  //loop through the fasta/q in list_of_fasta/q, and for each, print out a new coverage fasta
  
  FILE* list_fptr = fopen(list_of_fastaq, "r");
  if (list_fptr==NULL)
    {
      die("Cannot open %s\n", list_of_fastaq);
    }
  //printf("List of fasta is %s\n", list_of_fastaq);

  char line[MAX_FILENAME_LENGTH+1];
  
  while(fgets(line,MAX_FILENAME_LENGTH, list_fptr) !=NULL)
    {
      
      //remove newline from end of line - replace with \0
      char* p;
      if ((p = strchr(line, '\n')) != NULL)
	{
	  *p = '\0';
	}
      
      //printf("Looking at %s\n", line);

      char outputfile[200];
      sprintf(outputfile,"%s.colour_covgs",line);
      FILE* out = fopen(outputfile, "w");
      if (out ==NULL)
	{
	  die("Cannot open %s, exiting", outputfile);
	}
      //printf("Output to %s\n", outputfile);
      
      FILE* fp = fopen(line, "r");
      if (fp==NULL)
	{
	  die("Cannot open %s. Exit.\n", line);
	}
      
      int dummy_colour_ignored=0;//this will be ignored, as setting to false - we don't want to demand the read all lies in any colour
      int num_kmers;
      boolean full_entry=true;
      do
	{
	  if (format==FASTA)
	    {
	      num_kmers = align_next_read_to_graph_and_return_node_array(fp, max_read_length, array_nodes, array_or, false, &full_entry, file_reader_fasta,
									 seq, kmer_window, db_graph, dummy_colour_ignored);
	    }
	  else if (format==FASTQ)
	    {
	      num_kmers = align_next_read_to_graph_and_return_node_array(fp, max_read_length, array_nodes, array_or, false, &full_entry, file_reader_fastq,
									 seq, kmer_window, db_graph, dummy_colour_ignored);
	    }

	  
	  if (num_kmers>0)
	    {
	      int k;
	      if (mark_nodes_for_dumping==true)
		{
		  int q;
		  for (q=0; q<num_kmers; q++)//for each kmer in the read
		    {
		      if (array_nodes[q] !=NULL)
			{
			  db_node_set_status(array_nodes[q], to_be_dumped);
			}
		    }
		}
	      //print out the original read
	      if (is_for_testing==false)
		{
		  if (full_entry)
		    {
		      fprintf(out, ">%s\n%s\n", seq->name, seq->seq);
		    }
		  else
		    {
		      fprintf(out, ">%s partial (long read)\n%s\n", seq->name, seq->seq);
		    }
		}
	      else//for testing only
		{
		  for_test_array_of_strings[*for_test_index][0]='\0';
		  strcat(for_test_array_of_strings[*for_test_index], seq->name);
		  *for_test_index = *for_test_index+1;

		  for_test_array_of_strings[*for_test_index][0]='\0';
		  strcat(for_test_array_of_strings[*for_test_index], seq->seq);
		  *for_test_index = *for_test_index+1;
		}

	      //print out multiplicities of nodes in each of the colours in turn
	      int j;
	      for (j=0; j<num_of_colours ; j++)//for each colour
		{
		  char read_id[200];
		  if (full_entry)
		    {
		      sprintf(read_id, "%s_%s_kmer_coverages", seq->name, array_of_names_of_colours[j]);
		    }
		  else
		    {
		      sprintf(read_id, "%s_%s_kmer_coverages partial (long read)", seq->name, array_of_names_of_colours[j]);	      
		    }

		  if (is_for_testing==false)//print out read_id
		    {
		      fprintf(out, ">%s\n", read_id);
		    }
		  else
		    {
		      for_test_array_of_strings[*for_test_index][0]='\0';
		      strcat(for_test_array_of_strings[*for_test_index], read_id);
		      *for_test_index = *for_test_index+1;
		    }

		  for (k=0; k<num_kmers; k++)//for each kmer in the read
		    {
		      if (is_for_testing==false)//print covg
			{
			  if (array_nodes[k] !=NULL)
			    {
			      fprintf(out, "%d ", array_nodes[k]->coverage[array_of_colours[j]]);
			    }
			  else
			    {
			      fprintf(out, "0 ");
			    }
			}
		      else
			{
			  for_test_array_of_strings[*for_test_index][0]='\0';
			  char tmp[100];
			  sprintf(tmp, "%d", array_nodes[k]->coverage[array_of_colours[j]]);
			  strcat(for_test_array_of_strings[*for_test_index],  tmp);
			  *for_test_index = *for_test_index+1;
			}
		    }
		  if (is_for_testing==false)
		    {
			  fprintf(out, "\n");
		    }
		  
		}

	    }
	}while((num_kmers>0)||(!feof(fp)) );
      
      
      fclose(out);
      
    }
  
  fclose(list_fptr);
  free(kmer_window->kmer);
  free(kmer_window);
  free_sequence(&seq);
  free(array_nodes);
  free(array_or);
}


void print_percent_agreement_for_each_colour_for_each_read(char* fasta, int max_read_length, 
							   dBGraph* db_graph, char** list_sample_ids)
{


  //----------------------------------
  // allocate the memory used to read the sequences
  //----------------------------------
  Sequence * seq = malloc(sizeof(Sequence));
  if (seq == NULL){
    die("Out of memory trying to allocate Sequence");
  }
  alloc_sequence(seq,max_read_length,LINE_MAX);
  
  //We are going to load all the bases into a single sliding window 
  KmerSlidingWindow* kmer_window = malloc(sizeof(KmerSlidingWindow));
  if (kmer_window==NULL)
    {
      die("Failed to malloc kmer sliding window in "
          "align_list_of_fastaq_to_graph_and_print_coverages_in_all_colours. Exit.");
    }
  

  //  kmer_window->kmer = (BinaryKmer*) malloc(sizeof(BinaryKmer)*(max_read_length-db_graph->kmer_size-1));
  kmer_window->kmer = (BinaryKmer*) malloc(sizeof(BinaryKmer)*(max_read_length-db_graph->kmer_size+1));
  if (kmer_window->kmer==NULL)
    {
      die("Failed to malloc kmer_window->kmer in "
          "align_list_of_fastaq_to_graph_and_print_coverages_in_all_colours. Exit.");
    }
  kmer_window->nkmers=0;
  
  
  //end of intialisation 
	  
	  
  //create file readers
  int file_reader_fasta(FILE * fp, Sequence * seq, int max_read_length, boolean new_entry, boolean * full_entry){
    long long ret;
    int offset = 0;
    if (new_entry == false){
      offset = db_graph->kmer_size;
      //die("new_entry must be true in hsi test function");
    }
    ret =  read_sequence_from_fasta(fp,seq,max_read_length,new_entry,full_entry,offset);
    
    return ret;
  }

  dBNode** array_nodes = (dBNode**) malloc(sizeof(dBNode*) * (max_read_length+db_graph->kmer_size+1) );
  Orientation*  array_or = (Orientation*) malloc(sizeof(Orientation)*(max_read_length+db_graph->kmer_size+1) );
  if ( (array_nodes==NULL) || (array_or==NULL) )
    {
      die("Unable to malloc arrays for alignment");
    }
  
  //loop through the fasta/q in list_of_fasta/q, and for each, print out a new coverage fasta
  
  
  char outputfile[MAX_FILENAME_LENGTH];
  sprintf(outputfile,"%s.pangenome_matrix",fasta);
  FILE* out = fopen(outputfile, "w");
  if (out ==NULL)
    {
      die("Cannot open %s, exiting", outputfile);
    }

  //start debug
  char debugfile[MAX_FILENAME_LENGTH];
  sprintf(debugfile, "%s.estim_allele_freq", fasta);
  FILE* debug_fout = fopen(debugfile, "w");
  if (debug_fout ==NULL)
    {
      die("Cannot open %s, exiting", debugfile);
    }
  //end debug


  FILE* fp = fopen(fasta, "r");
  if (fp==NULL)
    {
      die("Cannot open %s. Exit.\n", fasta);
    }

  //print header line
  fprintf(out, "GENE/READ_ID\t");
  fprintf(debug_fout, "GENE/READ_ID\t");
  int i;
  for (i=0; i<NUMBER_OF_COLOURS; i++)
    {
      fprintf(out, "%s", list_sample_ids[i]);
      fprintf(debug_fout, "%s", list_sample_ids[i]);
    
      if (i<NUMBER_OF_COLOURS-1)
	{
	  fprintf(out, "\t");
	  fprintf(debug_fout, "\t");
	}
      else
	{
	  fprintf(out, "\n");
	  fprintf(debug_fout, "\n");
	}
    }
  
  int dummy_colour_ignored=0;//this will be ignored, as setting to false - we don't want to demand the read all lies in any colour
  int num_kmers;
  boolean full_entry=true;
  do
    {
      num_kmers = align_next_read_to_graph_and_return_node_array(fp, max_read_length, array_nodes, array_or, false, &full_entry, file_reader_fasta,
								 seq, kmer_window, db_graph, dummy_colour_ignored);
      
      if (num_kmers>0)
	{
	  int k;
	  if (full_entry)
	    {
	      fprintf(out, "%s\t", seq->name);
	      fprintf(debug_fout, "%s\t", seq->name);
	    }
	  else
	    {
	      die("Read longer than max read len\n");
	    }

	  
	  //print out % of nodes which are in each colour
	  int j;
	  for (j=0; j<NUMBER_OF_COLOURS; j++)//for each colour
	    {
	      int count_num_kmers_present=0;
	      Covg debug_min_covg=COVG_MAX;
	      //	      int total_kmers=0;
	      for (k=0; k<num_kmers; k++)//for each kmer in the read
		{

		  if (array_nodes[k] !=NULL)
		    {
		      // total_kmers++;
		      if ( array_nodes[k]->coverage[j]>0)
			{
			  count_num_kmers_present++;
			}
		      //debug
		      if (array_nodes[k]->coverage[j]<debug_min_covg)
			{
			  debug_min_covg = array_nodes[k]->coverage[j];
			}
		    }
		}
	      //if (total_kmers<=0)
	      if (num_kmers<=0)
		{
		  fprintf(out, "0");
		  fprintf(debug_fout, "0");//debug
		}
	      else
		{
		  float percent=(float) count_num_kmers_present/ (float)num_kmers; //total_kmers;
		  fprintf(out, "%0.2f", percent);
		  fprintf(debug_fout, "%" PRIu32 "", debug_min_covg);
		  
		}
	      if (j<NUMBER_OF_COLOURS-1)
		{
		  fprintf(out, "\t");
		  fprintf(debug_fout, "\t");
		}
	      else
		{
		  fprintf(out, "\n");
		  fprintf(debug_fout, "\n");
		}
	    }
	}
      
    }while((num_kmers>0)||(!feof(fp)) );
  
  
  fclose(out);
  fclose(debug_fout);
  
  
  
  
  free(kmer_window->kmer);
  free(kmer_window);
  free_sequence(&seq);
  free(array_nodes);
  free(array_or);
  
}
  
void print_supernodes_touched_by_fasta( char* fasta, int max_read_length, 
					dBGraph* db_graph, char* outfile_stub )
{
    //----------------------------------
  // allocate the memory used to read the sequences
  //----------------------------------
  Sequence * seq = malloc(sizeof(Sequence));
  if (seq == NULL){
    die("Out of memory trying to allocate Sequence");
  }
  alloc_sequence(seq,max_read_length,LINE_MAX);
  
  //We are going to load all the bases into a single sliding window 
  KmerSlidingWindow* kmer_window = malloc(sizeof(KmerSlidingWindow));
  if (kmer_window==NULL)
    {
      die("Failed to malloc kmer sliding window in "
          "align_list_of_fastaq_to_graph_and_print_coverages_in_all_colours. Exit.");
    }
  

  //  kmer_window->kmer = (BinaryKmer*) malloc(sizeof(BinaryKmer)*(max_read_length-db_graph->kmer_size-1));
  kmer_window->kmer = (BinaryKmer*) malloc(sizeof(BinaryKmer)*(max_read_length-db_graph->kmer_size+1));
  if (kmer_window->kmer==NULL)
    {
      die("Failed to malloc kmer_window->kmer in "
          "align_list_of_fastaq_to_graph_and_print_coverages_in_all_colours. Exit.");
    }
  kmer_window->nkmers=0;
  
  
  //end of intialisation 
	  
	  
  //create file reader
  int file_reader_fasta(FILE * fp, Sequence * seq, int max_read_length, boolean new_entry, boolean * full_entry){
    long long ret;
    int offset = 0;
    if (new_entry == false){
      offset = db_graph->kmer_size;
      //die("new_entry must be true in hsi test function");
    }
    ret =  read_sequence_from_fasta(fp,seq,max_read_length,new_entry,full_entry,offset);
    
    return ret;
  }


  dBNode** array_nodes = (dBNode**) malloc(sizeof(dBNode*) * (max_read_length+db_graph->kmer_size+1) );
  Orientation*  array_or = (Orientation*) malloc(sizeof(Orientation)*(max_read_length+db_graph->kmer_size+1) );
  if ( (array_nodes==NULL) || (array_or==NULL) )
    {
      die("Unable to malloc arrays for alignment");
    }
  
  FILE* fp = fopen(fasta, "r");
  if (fp==NULL)
    {
      die("Cannot open %s. Exit.\n", fasta);
    }



  //alloc some stuff for printing
  dBNode** path_nodes        = calloc(max_read_length,sizeof(dBNode*));
  Orientation* path_orientations = calloc(max_read_length,sizeof(Orientation));
  Nucleotide* path_labels       = calloc(max_read_length,sizeof(Nucleotide));
  char* path_seq               = calloc(max_read_length+1+db_graph->kmer_size,sizeof(char));
  
  if ( (path_nodes==NULL) || (path_orientations==NULL) || (path_labels==NULL) || (path_seq==NULL) )
    {
      die("Cannot malloc nodes for sup printing\n");
    }

  FILE* fout;
  int count_nodes=0;

  void print_touched_supernode(dBNode * node){
    
    char name[100];
    Covg max_covg=0;
    Covg min_covg=0;
    double avg_coverage=0;

    if (db_node_check_status(node, to_be_dumped) == true){

      boolean is_cycle=false;
      int length = db_graph_supernode_for_specific_person_or_pop(node,max_read_length,&db_node_action_set_status_visited,
								 path_nodes,path_orientations,path_labels, path_seq,
								 &avg_coverage, &min_covg, &max_covg, &is_cycle,
								 db_graph, 0);
      
      
      
      if (length>0){	
	sprintf(name,"node_%i",count_nodes);

	/*	print_ultra_minimal_fasta_from_path(fout,name,length,
					    path_nodes[0],path_orientations[0],path_seq,
					    db_graph->kmer_size,true); */
	if (length==max_read_length){
	  printf("contig length equals max length [%i] for node_%i\n",max_read_length,count_nodes);
	}
	count_nodes++;
      }
      
    }
  }


      
  int dummy_colour_ignored=0;//this will be ignored, as setting to false - we don't want to demand the read all lies in any colour
  int num_kmers=0;
  boolean full_entry=true;
  do
    {
      count_nodes=0;
      num_kmers = align_next_read_to_graph_and_return_node_array(fp, max_read_length, array_nodes, array_or, false, &full_entry, file_reader_fasta,
								 seq, kmer_window, db_graph, dummy_colour_ignored);
      if (num_kmers>0)
	{
	  int q;
	  for (q=0; q<num_kmers; q++)//for each kmer in the read
	    {
	      if (array_nodes[q] !=NULL)
		{
		  db_node_set_status(array_nodes[q], to_be_dumped);
		}
	    }

	  //ok, we have loaded a new gene. Now let's print out the supernodes that hit it.
	  StrBuf* outfile = strbuf_new();
	  strbuf_append_str(outfile, outfile_stub);
	  strbuf_append_str(outfile,".");
	  strbuf_append_str(outfile,seq->name);
	  strbuf_append_str(outfile,".fasta");
	  printf("We make outfile name %s\n", outfile->buff);
	  fout = fopen(outfile->buff, "w");
	  if (fout==NULL)
	    {
	      die("Cannot open %s for writing\n", outfile->buff);
	    }
    
	  hash_table_traverse(&print_touched_supernode,db_graph); 
	  fclose(fout);
	  hash_table_traverse(&db_node_action_unset_status_visited_or_visited_and_exists_in_reference, db_graph); 
	  strbuf_free(outfile);
	}
    }
  while((num_kmers>0)||(!feof(fp)) );



  
}
