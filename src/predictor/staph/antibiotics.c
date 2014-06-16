/*
 * Copyright 2014 Zamin Iqbal (zam@well.ox.ac.uk)
 * 
 *
 * **********************************************************************
 *
 * This file is part of myKrobe.
 *
 * myKrobe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * myKrobe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with myKrobe.  If not, see <http://www.gnu.org/licenses/>.
 *
 * **********************************************************************
 */
/*
  antibiotics.c 
*/

#include "antibiotics.h"
#include "file_reader.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "mut_models.h"
#include "gene_presence_models.h"
#include "json.h"

void map_antibiotic_enum_to_str(Antibiotic ab, StrBuf* name)
{
  if (ab==NoDrug)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "NoDrug");
    }
  else if (ab==Gentamicin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Gentamicin");
    }
  else if (ab==Penicillin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Penicillin");
    }
  else if (ab==Trimethoprim)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Trimethoprim");
    }
  else if (ab==Erythromycin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Erythromycin");
    }
  else if (ab==Methicillin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Methicillin");
    }
  else if (ab==Ciprofloxacin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Ciprofloxacin");
    }
  else if (ab==Rifampicin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Rifampicin");
    }
  else if (ab==Tetracycline)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Tetracycline");
    }
  else if (ab==Mupirocin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Mupirocin");
    }
  else if (ab==FusidicAcid)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "FusidicAcid");
    }
  else if (ab==FusidicAcid)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "FusidicAcid");
    }
  else if (ab==Clindamycin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Clindamycin");
    }
  else if (ab==Vancomycin)
    {
      strbuf_reset(name);
      strbuf_append_str(name, "Vancomycin");
    }
  else
    {
      die("Impossible - compiler should not allow this\n");
    }
}
AntibioticInfo* alloc_antibiotic_info()
{
  AntibioticInfo* abi = (AntibioticInfo*) calloc(1, sizeof(AntibioticInfo));
  if (abi==NULL)
    {
      return NULL;
    }
  else
    {
      abi->m_fasta = strbuf_new();
      abi->num_genes = 0;
      abi->mut = (ResVarInfo**) malloc(sizeof(ResVarInfo*)*NUM_KNOWN_MUTATIONS);
      if (abi->mut==NULL)
	{
	  strbuf_free(abi->m_fasta);
	  free(abi);
	  return NULL;
	}
      abi->genes = (GeneInfo**) malloc(sizeof(GeneInfo*)*NUM_GENE_PRESENCE_GENES);
      if (abi->genes==NULL)
	{
	  free(abi->mut);
	  strbuf_free(abi->m_fasta);
	  free(abi);
	  return NULL;
	}
      abi->which_genes = (int*) calloc(MAX_GENES_PER_ANTIBIOTIC, sizeof(int));
      if (abi->which_genes==NULL)
	{
	  free(abi->genes);
	  free(abi->mut);
	  strbuf_free(abi->m_fasta);
	  free(abi);
	  return NULL;
	  
	}
      int i;
      for (i=0; i<NUM_KNOWN_MUTATIONS; i++)
	{
	  abi->mut[i] = alloc_and_init_res_var_info();
	}
      for (i=0; i<NUM_GENE_PRESENCE_GENES; i++)
	{
	  abi->genes[i] = alloc_and_init_gene_info();
	  abi->genes[i]->name = (GenePresenceGene) i;
	}

    }
  return abi;
}
void free_antibiotic_info(AntibioticInfo* abi)
{
  if (abi==NULL)
    {
      return;
    }
  else
    {
      strbuf_free(abi->m_fasta);
      int i;
      for (i=0; i<NUM_KNOWN_MUTATIONS; i++)
	{
	  free_res_var_info(abi->mut[i]);
	}
      for (i=0; i<NUM_GENE_PRESENCE_GENES; i++)
	{
	  free_gene_info(abi->genes[i]);
	}
      free(abi);
    }
}

void reset_antibiotic_info(AntibioticInfo* abi)
{
  abi->ab = NoDrug;
  strbuf_reset(abi->m_fasta);
  abi->num_mutations=0;

  int i;
  for (i=0; i<NUM_KNOWN_MUTATIONS; i++)
    {
      reset_res_var_info(abi->mut[i]);
    }
  for (i=0; i<NUM_GENE_PRESENCE_GENES; i++)
    {
      reset_gene_info(abi->genes[i]);
    }
}

void  load_antibiotic_mutation_info_on_sample(FILE* fp,
					      dBGraph* db_graph,
					      int (*file_reader)(FILE * fp, 
								 Sequence * seq, 
								 int max_read_length, 
								 boolean new_entry, 
								 boolean * full_entry),
					      AntibioticInfo* abi,
					      ReadingUtils* rutils,
					      ResVarInfo* tmp_rvi,	
					      int ignore_first, int ignore_last, int expected_covg)
{
  reset_reading_utils(rutils);
  reset_res_var_info(tmp_rvi);

  StrBuf* tmp1 = strbuf_new();
  StrBuf* tmp2 = strbuf_new();
  StrBuf* tmp3 = strbuf_new();

  
  int i;

  KnownMutation m = NotSpecified;
  for (i=0; i<abi->num_mutations; i++)
    {
      get_next_mutation_allele_info(fp, 
				    db_graph, 
				    tmp_rvi,
				    rutils->seq, 
				    rutils->kmer_window, 
				    file_reader,
				    rutils->array_nodes, 
				    rutils->array_or,
				    rutils->working_ca, 
				    MAX_LEN_MUT_ALLELE,
				    tmp1, tmp2, tmp3,
				    ignore_first, ignore_last, 
				    expected_covg, &m);

      copy_res_var_info(tmp_rvi, abi->mut[tmp_rvi->var_id]);
    }
  
  strbuf_free(tmp1);
  strbuf_free(tmp2);
  strbuf_free(tmp3);

}


//one gene per fasta, so if you have multiple reads,
//these are different exemplars, for divergent versions of the same gene
void load_antibiotic_gene_presence_info_on_sample(FILE* fp,
						  dBGraph* db_graph,
						  int (*file_reader)(FILE * fp, 
								     Sequence * seq, 
								     int max_read_length, 
								     boolean new_entry, 
								     boolean * full_entry),
						  AntibioticInfo* abi,
						  ReadingUtils* rutils,
						  GeneInfo* tmp_gi)



{
  reset_reading_utils(rutils);

  int num=1;

  while (num>0)
    {
      num = get_next_gene_info(fp,
			       db_graph,
			       tmp_gi,
			       rutils->seq,
			       rutils->kmer_window,
			       file_reader,
			       rutils->array_nodes,
			       rutils->array_or,
			       rutils->working_ca,
			       MAX_LEN_GENE);
      /*      printf("Percent >0 %d\n Median on nonzero %d\nMin %d\n, median %d\n",
	     tmp_gi->percent_nonzero,
	     tmp_gi->median_covg_on_nonzero_nodes,
	     tmp_gi->median_covg,
	     tmp_gi->min_covg);
      */

      if (tmp_gi->percent_nonzero>abi->genes[tmp_gi->name]->percent_nonzero)
	{
	  copy_gene_info(tmp_gi, abi->genes[tmp_gi->name]);
	}

    }
  

}



void load_antibiotic_mut_and_gene_info(dBGraph* db_graph,
				       int (*file_reader)(FILE * fp, 
							  Sequence * seq, 
							  int max_read_length, 
							  boolean new_entry, 
							  boolean * full_entry),
				       AntibioticInfo* abi,
				       ReadingUtils* rutils,
				       ResVarInfo* tmp_rvi,
				       GeneInfo* tmp_gi,
				       int ignore_first, int ignore_last, int expected_covg,
				       StrBuf* install_dir)

{

  FILE* fp;

  if (abi->num_mutations>0)
    {
      fp = fopen(abi->m_fasta->buff, "r");
      if (fp==NULL)
	{
	  die("Cannot open %s - should be there as part of the install - did you run out of disk mid-install?\n",
	      abi->m_fasta->buff);
	}
      
      load_antibiotic_mutation_info_on_sample(fp,
					      db_graph,
					      file_reader,
					      abi,
					      rutils, 
					      tmp_rvi,
					      ignore_first, ignore_last, expected_covg);
      fclose(fp);
    }
  if (abi->num_genes>0)
    {
      StrBuf* tmp = strbuf_new();
      int j;
      for (j=0; j<abi->num_genes; j++)
	{
	  strbuf_reset(tmp);
	  GenePresenceGene g = (GenePresenceGene) abi->which_genes[j];
	  map_gene_to_fasta(g, tmp, install_dir);
	  FILE* fp = fopen(tmp->buff, "r");
	  if (fp==NULL)
	    {
	      die("Unable to open %s, which should come as part of the install.\n", tmp->buff);
	    }
	  load_antibiotic_gene_presence_info_on_sample(fp,
						       db_graph,
						       file_reader,
						       abi,
						       rutils,
						       tmp_gi);
	  fclose(fp);
	}
      strbuf_free(tmp);

    }
}






Troolean is_gentamicin_susceptible(dBGraph* db_graph,
				  int (*file_reader)(FILE * fp, 
						     Sequence * seq, 
						     int max_read_length, 
						     boolean new_entry, 
						     boolean * full_entry),
				  ReadingUtils* rutils,
				  ResVarInfo* tmp_rvi,
				  GeneInfo* tmp_gi,
				  AntibioticInfo* abi,
				  StrBuf* install_dir,
				  int ignore_first, int ignore_last, int expected_covg,
				  double lambda_g, double lambda_e, double err_rate
				  )
{
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab = Gentamicin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/gentamicin.fa");
  abi->which_genes[0]=aacAaphD;
  abi->num_genes=1;
  abi->num_mutations = 0;//entirely determined by gene presence
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi, ignore_first, ignore_last, expected_covg,
				    install_dir);

  Model best_model;
  InfectionType I=
    resistotype_gene(abi->genes[aacAaphD], err_rate, db_graph->kmer_size, 
		     lambda_g, epsilon, expected_covg,
		     &best_model, MaxAPosteriori,
		     MIN_PERC_COVG_STANDARD);
  if ( (I==Resistant) || (I==MixedInfection) ) 
    {
      return _False;
    }
  else if (I==Susceptible)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }

}


Troolean is_penicillin_susceptible(dBGraph* db_graph,
				  int (*file_reader)(FILE * fp, 
						     Sequence * seq, 
						     int max_read_length, 
						     boolean new_entry, 
						     boolean * full_entry),
				  ReadingUtils* rutils,
				  ResVarInfo* tmp_rvi,
				  GeneInfo* tmp_gi,
				  AntibioticInfo* abi,
				  StrBuf* install_dir,
				  int ignore_first, int ignore_last, int expected_covg,
				  double lambda_g, double lambda_e, double err_rate
				  )

{

  reset_antibiotic_info(abi);

  //setup antibiotic info object
  abi->ab = Penicillin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/penicillin.fa");
  abi->num_mutations = 0;//entirely determined by gene presence
  abi->which_genes[0]=blaZ;
  abi->num_genes=1;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);

  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, 
				    ignore_last, 
				    expected_covg,
				    install_dir);

  //  printf("Got blaZ percent %d\n", abi->genes[blaZ]->percent_nonzero);

  Model best_model;
  InfectionType I=
    resistotype_gene(abi->genes[blaZ], err_rate, db_graph->kmer_size, 
		     lambda_g, epsilon,expected_covg,
		     &best_model, MaxAPosteriori,
		     MIN_PERC_COVG_BLAZ);

  if ( (I==Resistant) || (I==MixedInfection) ) 
    {
      return _False;
    }
  else if (I==Susceptible)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }


}


Troolean is_trimethoprim_susceptible(dBGraph* db_graph,
				    int (*file_reader)(FILE * fp, 
						       Sequence * seq, 
						       int max_read_length, 
						       boolean new_entry, 
						       boolean * full_entry),
				    ReadingUtils* rutils,
				    ResVarInfo* tmp_rvi,
				    GeneInfo* tmp_gi,
				    AntibioticInfo* abi,
				    StrBuf* install_dir,
				    int ignore_first, int ignore_last, int expected_covg,
				    double lambda_g, double lambda_e, double err_rate
				    )

{

  reset_antibiotic_info(abi);

  //setup antibiotic info object
  abi->ab = Trimethoprim;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/trimethoprim.fa");
  abi->num_mutations = 113;
  abi->which_genes[0]=dfrA;
  abi->which_genes[1]=dfrG;
  abi->num_genes=2;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
				    install_dir);

  int first_trim_mut = dfrB_L21V;
  int last_trim_mut = dfrB_H150R;
  int i;

  //we are going to iterate through various mutations, each
  //on different genetic backgrounds.
  //we will call it susceptible, if the best hit is good enough
  
  double max_sus_conf=0;
  double min_conf=9999999;//min across all sites

  //if you have any of these resistance alleles - call resistant
  boolean any_allele_non_null=false;
  for (i=first_trim_mut; i<=last_trim_mut; i++)
    {
      if (both_alleles_null(abi->mut[i])==true)
	{
	  continue;
	}

      any_allele_non_null=true;
      Model best_model;
      InfectionType I=
	resistotype(abi->mut[i], err_rate, db_graph->kmer_size, 
		    lambda_g, lambda_e, epsilon,
		    &best_model, MaxAPosteriori);


      if ( (I==Susceptible) && (best_model.conf>max_sus_conf) )
	{
	  max_sus_conf = best_model.conf;
	}
      if (best_model.conf<min_conf)
	{
	  min_conf = best_model.conf;
	}
      if ( (I==Resistant) || (I==MixedInfection) ) 
	{

	  if (I==MixedInfection)
	    {
	      printf("MIXED\n");
	    }
	  else
	    {
	      printf("RES\n");
	    }

	  return _False;
	}

    }


  //NOT straightforward how to get  confidence when integrating
  //both SNP and gene presence calls.

  for (i=0; i<=1; i++)
    {

      Model best_model;
      InfectionType I =
	resistotype_gene(abi->genes[abi->which_genes[i]], 
			 err_rate, db_graph->kmer_size, 
			 lambda_g, epsilon, expected_covg,
			 &best_model, MaxAPosteriori,
			 MIN_PERC_COVG_STANDARD);
      if ( (I==Resistant) || (I==MixedInfection) ) 
	{
	  return _False;
	}
    }

  if ( 
      (any_allele_non_null==false) //all alleles have zero covg
      || 
      (min_conf<MIN_CONFIDENCE) //at one site, you're not sure
       )
    {
      return _Inconclusive;
    }
  else if (max_sus_conf>MIN_CONFIDENCE)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }
}

/* reminder of how erythromycin and clindamycin resistance is related:
gene               erythromycin          clindamycin disc    clindamycin D test         note
erm A, B, C, T     R                     S or R              positive                 assume clinda R
msrA               R                     S                   negative                 erythromycin specific efflux pump
vga(A)LC           S                     R                   n/a                      clindamycin specific efflux pump
*/


Troolean is_erythromycin_susceptible(dBGraph* db_graph,
				    int (*file_reader)(FILE * fp, 
						       Sequence * seq, 
						       int max_read_length, 
						       boolean new_entry, 
						       boolean * full_entry),
				    ReadingUtils* rutils,
				    ResVarInfo* tmp_rvi,
				    GeneInfo* tmp_gi,
				    AntibioticInfo* abi,
				    StrBuf* install_dir,
				    int ignore_first, int ignore_last, int expected_covg,
				    double lambda_g, double lambda_e, double err_rate,
				    boolean* any_erm_present)
{
  reset_antibiotic_info(abi);
  *any_erm_present=false;
  //setup antibiotic info object
  abi->ab = Erythromycin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/erythromycin.fa");


  abi->num_mutations = 0;
  abi->which_genes[0]=ermA;
  abi->which_genes[1]=ermB;
  abi->which_genes[2]=ermC;
  abi->which_genes[3]=ermT;
  abi->which_genes[4]=msrA;
  abi->num_genes=5;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
	install_dir);

  int i;

  //we will call it susceptible, if the least
  //confidnece when calling it not resistant, is > a min
  double max_sus_conf=0;
  double min_conf=9999999; //min across sites
  for (i=0; i<=4; i++)
    {

      Model best_model;
      InfectionType I =
	resistotype_gene(abi->genes[abi->which_genes[i]], 
			 err_rate, db_graph->kmer_size, 
			 lambda_g, epsilon, expected_covg,
			 &best_model, MaxAPosteriori,
			 MIN_PERC_COVG_STANDARD);
      
      if ( (I==Susceptible) && (best_model.conf>max_sus_conf) ) 
	{
	  max_sus_conf = best_model.conf;
	}
      if (best_model.conf<min_conf)
	{
	  min_conf = best_model.conf;
	}

      if ( (I==Resistant) || (I==MixedInfection) ) 
	{
	  if (i<4)
	    {
	      *any_erm_present=true;
	    }
	  return _False;
	}
    }
  if (min_conf<MIN_CONFIDENCE)
    {
      return _Inconclusive;
    }
  if (max_sus_conf>MIN_CONFIDENCE)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }

}


Troolean is_methicillin_susceptible(dBGraph* db_graph,
				   int (*file_reader)(FILE * fp, 
						      Sequence * seq, 
						      int max_read_length, 
						      boolean new_entry, 
						      boolean * full_entry),
				   ReadingUtils* rutils,
				   ResVarInfo* tmp_rvi,
				   GeneInfo* tmp_gi,
				   AntibioticInfo* abi,
				   StrBuf* install_dir,
				   int ignore_first, int ignore_last, int expected_covg,
				   double lambda_g, double lambda_e, double err_rate)
{
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab = Methicillin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/methicillin.fa");
  abi->num_mutations = 0;
  abi->which_genes[0]=mecA;
  abi->num_genes=1;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
	install_dir);

  Model best_model;
  InfectionType I =
    resistotype_gene(abi->genes[mecA], 
		     err_rate, db_graph->kmer_size, 
		     lambda_g, epsilon, expected_covg,
		     &best_model, MaxAPosteriori,
		     MIN_PERC_COVG_STANDARD);
  if ( (I==Resistant) || (I==MixedInfection) ) 
    {
      return _False;
    }
  else if (best_model.conf>MIN_CONFIDENCE)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }

}


Troolean is_ciprofloxacin_susceptible(dBGraph* db_graph,
				   int (*file_reader)(FILE * fp, 
						      Sequence * seq, 
						      int max_read_length, 
						      boolean new_entry, 
						      boolean * full_entry),
				     ReadingUtils* rutils,
				     ResVarInfo* tmp_rvi,
				     GeneInfo* tmp_gi,
				     AntibioticInfo* abi,
				     StrBuf* install_dir,
				     int ignore_first, int ignore_last, int expected_covg,
				     double lambda_g, double lambda_e, double err_rate)
{
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab = Ciprofloxacin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);

  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/ciprofloxacin.fa");

  abi->num_mutations = 94;
  abi->num_genes=0;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
	install_dir);




  int first_cip_mut = gyrA_S84A;
  int last_cip_mut = grlA_S80Y;
  int i;

  //if you have any of these resistance alleles - call resistant
  double max_sus_conf=0;
  double min_conf=9999999;
  boolean any_allele_non_null=false;
  for (i=first_cip_mut; i<=last_cip_mut; i++)
    {
      if (both_alleles_null(abi->mut[i])==true)
	{
	  continue;
	}
      any_allele_non_null=true;
      Model best_model;
      InfectionType I=
	resistotype(abi->mut[i],
		   err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		    &best_model, MaxAPosteriori);
      if (max_sus_conf<best_model.conf)
	{
	  max_sus_conf=best_model.conf;
	}
      if (best_model.conf<min_conf)
	{
	  min_conf = best_model.conf;
	}

      if ( (I==Resistant) || (I==MixedInfection) ) 
	{
	  return _False;
	}
    }

  if (
      (any_allele_non_null==false)
      ||
      (min_conf<MIN_CONFIDENCE) //at one site, you're not sure
      )

    {
       return _Inconclusive;
    }
  else if (max_sus_conf>MIN_CONFIDENCE)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }

}


Troolean is_rifampicin_susceptible(dBGraph* db_graph,
				   int (*file_reader)(FILE * fp, 
						      Sequence * seq, 
						      int max_read_length, 
						      boolean new_entry, 
						      boolean * full_entry),
				  ReadingUtils* rutils,
				  ResVarInfo* tmp_rvi,
				  GeneInfo* tmp_gi,
				  AntibioticInfo* abi,
				  StrBuf* install_dir,
				  int ignore_first, int ignore_last, int expected_covg,
				  double lambda_g, double lambda_e, double err_rate)
{
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab = Rifampicin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/rifampicin.fa");


  abi->num_mutations = 453;
  abi->num_genes=0;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
				    install_dir);


  //these aren't the first of ALL rifampicin/rpoB mutations
  //but the first and last of those able to cause resistance on their own
  int first_rif_mut = rpoB_S463P;
  int last_rif_mut = rpoB_D550G;
  int i;

  //if you have any of these resistance alleles - call resistant
  //covers all except the two mutations that have to occur together

  Model best_model;
  double max_sus_conf=0;
  double min_conf=9999999;
  boolean any_allele_non_null=false;
  for (i=first_rif_mut; i<=last_rif_mut; i++)
    {

      if (both_alleles_null(abi->mut[i])==true)
	{
	  continue;
	}
      any_allele_non_null=true;
      InfectionType I=
	resistotype(abi->mut[i],
		    err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		    &best_model, MaxAPosteriori);

      if ( (I==Susceptible) && (best_model.conf>max_sus_conf) )
	{
	  max_sus_conf = best_model.conf;
	}
      if (best_model.conf<min_conf)
	{
	  min_conf = best_model.conf;
	}

      if ( (I==Resistant) || (I==MixedInfection) ) 
	{	 
	  return _False;
	}
    }

  InfectionType I_m470t=
    resistotype(abi->mut[rpoB_M470T],
		err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);

  if ( (I_m470t==Susceptible) && (best_model.conf>max_sus_conf) )
    {
      max_sus_conf = best_model.conf;
    }
  if (best_model.conf<min_conf)
    {
      min_conf = best_model.conf;
    }

  InfectionType I_d471g=
    resistotype(abi->mut[rpoB_D471G],
		err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);

  if ( (I_d471g ==Susceptible) && (best_model.conf>max_sus_conf) )
    {
      max_sus_conf = best_model.conf;
    }
  if (best_model.conf<min_conf)
    {
      min_conf = best_model.conf;
    }

  if (I_m470t==Resistant && I_d471g==Resistant)
    {
      return _False; //ignoring mixed infections for epistatic case
    }

  if (
      (any_allele_non_null==false)
      ||
      (min_conf<MIN_CONFIDENCE) //at one site, you're not sure
      )

    {
       return _Inconclusive;
    }
  else if (max_sus_conf>MIN_CONFIDENCE)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }

}

Troolean is_tetracycline_susceptible(dBGraph* db_graph,
				   int (*file_reader)(FILE * fp, 
						      Sequence * seq, 
						      int max_read_length, 
						      boolean new_entry, 
						      boolean * full_entry),
				    ReadingUtils* rutils,
				    ResVarInfo* tmp_rvi,
				    GeneInfo* tmp_gi,
				    AntibioticInfo* abi,
				    StrBuf* install_dir,
				    int ignore_first, int ignore_last, int expected_covg,
				    double lambda_g, double lambda_e, double err_rate)
{
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab =Tetracycline;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/tetracycline.fa");
  abi->num_mutations = 0;
  abi->which_genes[0]=tetK;
  abi->which_genes[1]=tetL;
  abi->which_genes[2]=tetM;
  abi->num_genes=3;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
	install_dir);


  int i;
  double max_sus_conf=0;
  double min_conf=9999999;
  for (i=0; i<=2; i++)
    {

      Model best_model;
      InfectionType I =
	resistotype_gene(abi->genes[abi->which_genes[i]], 
			 err_rate, db_graph->kmer_size, 
			 lambda_g, epsilon, expected_covg,
			 &best_model, MaxAPosteriori,
			 MIN_PERC_COVG_STANDARD);

      if ( (I==Susceptible) && (best_model.conf>max_sus_conf) ) 
	{
	  max_sus_conf = best_model.conf;
	}
      if (best_model.conf<min_conf)
	{
	  min_conf = best_model.conf;
	}
      if ( (I==Resistant) || (I==MixedInfection) ) 
	{
	  return _False;
	}
    }

  if (min_conf<MIN_CONFIDENCE)
    {
      return _Inconclusive;
    }
  else if (max_sus_conf>MIN_CONFIDENCE)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }

}


Troolean is_mupirocin_susceptible(dBGraph* db_graph,
				 int (*file_reader)(FILE * fp, 
						    Sequence * seq, 
						    int max_read_length, 
						    boolean new_entry, 
						    boolean * full_entry),
				 ReadingUtils* rutils,
				 ResVarInfo* tmp_rvi,
				 GeneInfo* tmp_gi,
				 AntibioticInfo* abi,
				 StrBuf* install_dir,
				 int ignore_first, int ignore_last, int expected_covg,
				 double lambda_g, double lambda_e, double err_rate)
{
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab = Mupirocin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/mupirocin.fa");

  abi->num_mutations = 0;
  abi->which_genes[0]=mupA;
  abi->which_genes[1]=mupB;
  abi->num_genes=2;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
	install_dir);


  int i;
  for (i=0; i<=1; i++)
    {

      Model best_model;
      InfectionType I =
	resistotype_gene(abi->genes[abi->which_genes[i]], 
			 err_rate, db_graph->kmer_size, 
			 lambda_g, epsilon, expected_covg,
			 &best_model, MaxAPosteriori,
			 MIN_PERC_COVG_STANDARD);
      if ( (I==Resistant) || (I==MixedInfection) ) 
	{
	  return _False;
	}
    }

 return _True;
}


Troolean is_fusidic_acid_susceptible(dBGraph* db_graph,
				    int (*file_reader)(FILE * fp, 
						       Sequence * seq, 
						       int max_read_length, 
						       boolean new_entry, 
						       boolean * full_entry),
				    ReadingUtils* rutils,
				    ResVarInfo* tmp_rvi,
				    GeneInfo* tmp_gi,
				    AntibioticInfo* abi,
				    StrBuf* install_dir,
				    int ignore_first, int ignore_last, int expected_covg,
				    double lambda_g, double lambda_e, double err_rate)
{
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab = FusidicAcid;
  strbuf_append_str(abi->m_fasta, install_dir->buff);

  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/fusidic_acid.fa");
  abi->num_mutations = 961;

  abi->which_genes[0]=fusB;
  abi->which_genes[1]=fusC;
  abi->num_genes=2;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);

  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
				    install_dir);
  

  // in terms of avoiding future bugs when people modify the code,
  // I'm not super-keen on what I've done here:
  // for this to work, there is a dependency between the ordering in the KnownMutations enum,
  // and this code.
  // note first first/last mutations are the first/last of the non-epistatic mutations,
  // which each are enough to cause resistance on their own.
  // I deal with the epistatic ones after the following loop
  int first_fus_mut = fusA_V90I;
  int last_fus_mut  = fusA_G664S;
  int i;

  Model best_model;
  double max_sus_conf=0;
  double min_conf=9999999;
  boolean any_allele_non_null=false;
  for (i=first_fus_mut; i<=last_fus_mut; i++)
    {

      if (both_alleles_null(abi->mut[i])==true)
	{
	  continue;
	}
      any_allele_non_null=true;
      InfectionType I=
	resistotype(abi->mut[i],
		    err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		    &best_model, MaxAPosteriori);
      
      if ( (I==Susceptible) && (best_model.conf>max_sus_conf) )
	{
	  max_sus_conf = best_model.conf;
	}
      if (best_model.conf<min_conf)
	{
	  min_conf = best_model.conf;
	}
      if ( (I==Resistant) || (I==MixedInfection) ) 
	{
	  return _False;
	}
    }
  
  

  InfectionType I_f652s=
    resistotype(abi->mut[fusA_F652S],
		err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);

  InfectionType I_y654n=
    resistotype(abi->mut[fusA_Y654N],
	       err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);
  if (I_f652s==Resistant && I_y654n==Resistant)
    {
      return _False;
    }




  InfectionType I_t326i=
    resistotype(abi->mut[fusA_T326I],
		err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);

  InfectionType I_e468v=
    resistotype(abi->mut[fusA_E468V],
	       err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);


  if (I_t326i==Resistant && I_e468v==Resistant)
    {
      return _False;
    }
  



  InfectionType I_l461f=
    resistotype(abi->mut[fusA_L461F],
		err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);

  InfectionType I_a376v=
    resistotype(abi->mut[fusA_A376V],
		err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);

  InfectionType I_a655p=
    resistotype(abi->mut[fusA_A655P],
		err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);

  InfectionType I_d463g=
    resistotype(abi->mut[fusA_D463G],
		err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
		&best_model, MaxAPosteriori);
  
  if ( (I_l461f==Resistant)
       &&
       (I_a376v==Resistant)
       &&
       (I_a655p==Resistant)
       &&
       (I_d463g==Resistant)
       )
    {
      return _False;
    }

  InfectionType I_e444v=Susceptible;
  resistotype(abi->mut[fusA_E444V],
	      err_rate, db_graph->kmer_size, lambda_g, lambda_e, epsilon,
	      &best_model, MaxAPosteriori);
  if ((I_l461f==Resistant)
       &&
      (I_e444v==Resistant) )
    {
      return _False;
    }



  for (i=0; i<=1; i++)
    {
      Model best_model;
      InfectionType I =
	resistotype_gene(abi->genes[abi->which_genes[i]], 
			 err_rate, db_graph->kmer_size, 
			 lambda_g, epsilon, expected_covg,
			 &best_model, MaxAPosteriori,
			 MIN_PERC_COVG_FUSBC);
      if ( (I==Resistant) || (I==MixedInfection) ) 
	{
	  return _False;
	}
    }

  if (
      (any_allele_non_null==false)
      ||
      (min_conf<MIN_CONFIDENCE) //at one site, you're not sure
      )

    {
       return _Inconclusive;
    }
  else if (max_sus_conf>MIN_CONFIDENCE)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }
}


Troolean is_clindamycin_susceptible(dBGraph* db_graph,
				   int (*file_reader)(FILE * fp, 
						      Sequence * seq, 
						      int max_read_length, 
						      boolean new_entry, 
						      boolean * full_entry),
				   ReadingUtils* rutils,
				   ResVarInfo* tmp_rvi,
				   GeneInfo* tmp_gi,
				   AntibioticInfo* abi,
				   StrBuf* install_dir,
				   int ignore_first, int ignore_last, int expected_covg,
				   double lambda_g, double lambda_e, double err_rate)

{
  //constitutuve only. inducible you get by checking erythromycin also,
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab = Clindamycin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/clindamycin.fa");
  abi->num_mutations = 0;
  abi->which_genes[0]=vga_A_LC;
  abi->num_genes=1;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
	install_dir);


  Model best_model;
  InfectionType I =
    resistotype_gene(abi->genes[vga_A_LC], 
		     err_rate, db_graph->kmer_size, 
		     lambda_g, epsilon, expected_covg,
		     &best_model, MaxAPosteriori,
		     MIN_PERC_COVG_STANDARD);
  if ( (I==Resistant) || (I==MixedInfection) ) 
    {
      return _False;
    }
  else if (I==Susceptible)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }

}


Troolean is_vancomycin_susceptible(dBGraph* db_graph,
				   int (*file_reader)(FILE * fp, 
						      Sequence * seq, 
						      int max_read_length, 
						      boolean new_entry, 
						      boolean * full_entry),
				   ReadingUtils* rutils,
				   ResVarInfo* tmp_rvi,
				   GeneInfo* tmp_gi,
				   AntibioticInfo* abi,
				  StrBuf* install_dir,
				  int ignore_first, int ignore_last, int expected_covg,
				  double lambda_g, double lambda_e, double err_rate)
  
{
  //constitutuve only. inducible you get by checking erythromycin also,
  reset_antibiotic_info(abi);
  
  //setup antibiotic info object
  abi->ab = Vancomycin;
  strbuf_append_str(abi->m_fasta, install_dir->buff);
  strbuf_append_str(abi->m_fasta, "data/staph/antibiotics/vancomycin.fa");

  abi->num_mutations = 0;
  abi->which_genes[0]=vanA;
  abi->num_genes=1;
  double epsilon = pow(1-err_rate, db_graph->kmer_size);
  load_antibiotic_mut_and_gene_info(db_graph,
				    file_reader,
				    abi,
				    rutils,
				    tmp_rvi,
				    tmp_gi,
				    ignore_first, ignore_last, expected_covg,
				    install_dir);

  Model best_model;
  InfectionType I =
    resistotype_gene(abi->genes[vanA], 
		     err_rate, db_graph->kmer_size, 
		     lambda_g, epsilon, expected_covg,
		     &best_model, MaxAPosteriori,
		     MIN_PERC_COVG_STANDARD);
  if ( (I==Resistant) || (I==MixedInfection) ) 
    {
      return _False;
    }
  else if (I==Susceptible)
    {
      return _True;
    }
  else
    {
      return _Inconclusive;
    }


}



void print_antibiotic_susceptibility(dBGraph* db_graph,
					int (*file_reader)(FILE * fp, 
							   Sequence * seq, 
							   int max_read_length, 
							   boolean new_entry, 
							   boolean * full_entry),
					ReadingUtils* rutils,
					ResVarInfo* tmp_rvi,
					GeneInfo* tmp_gi,
					AntibioticInfo* abi,
					Troolean (*func)(dBGraph* db_graph,
							int (*file_reader)(FILE * fp, 
									   Sequence * seq, 
									   int max_read_length, 
									   boolean new_entry, 
									   boolean * full_entry),
							ReadingUtils* rutils,
							ResVarInfo* tmp_rvi,
							GeneInfo* tmp_gi,
							AntibioticInfo* abi,
							StrBuf* install_dir,
							int ignore_first, int ignore_last, int expected_covg,
							double lambda_g, double lambda_e, double err_rate),
					StrBuf* tmpbuf,
					StrBuf* install_dir,
					int ignore_first, int ignore_last,
					int expected_covg,
					double lambda_g, double lambda_e, double err_rate,
					OutputFormat format,
					boolean output_last//for JSON
					)
{
  Troolean suc;
  
  suc  = func(db_graph,
	      file_reader,
	      rutils,
	      tmp_rvi,
	      tmp_gi,
	      abi, 
	      install_dir,
	      ignore_first, 
	      ignore_last, 
	      expected_covg,
	      lambda_g,
	      lambda_e,
	      err_rate);

  
  map_antibiotic_enum_to_str(abi->ab, tmpbuf);
  if (format==Stdout)
    {
      printf("%s\t", tmpbuf->buff);
      if (suc==_True)
	{
	  printf("S\n");
	}
      else if (suc==_False)
	{
	  printf("R\n");
	}
      else
	{
	  printf("N\n");
	}
    }
  else
    {
      if (suc==_True)
	{
	    print_json_item(tmpbuf->buff, "S", output_last);
	}
      else if (suc==_False)
	{
	  print_json_item(tmpbuf->buff, "R", output_last);
	}
      else
	{
	  print_json_item(tmpbuf->buff, "Inconclusive", output_last);
	}
    }

}


void print_erythromycin_susceptibility(dBGraph* db_graph,
					  int (*file_reader)(FILE * fp, 
							     Sequence * seq, 
							     int max_read_length, 
							     boolean new_entry, 
							     boolean * full_entry),
					  ReadingUtils* rutils,
					  ResVarInfo* tmp_rvi,
					  GeneInfo* tmp_gi,
					  AntibioticInfo* abi,
					  Troolean (*func)(dBGraph* db_graph,
							 int (*file_reader)(FILE * fp, 
									    Sequence * seq, 
									    int max_read_length, 
									    boolean new_entry, 
									    boolean * full_entry),
							  ReadingUtils* rutils,
							  ResVarInfo* tmp_rvi,
							  GeneInfo* tmp_gi,
							  AntibioticInfo* abi,
							  StrBuf* install_dir,
							  int ignore_first, int ignore_last, int expected_covg,
							  double lambda_g, double lambda_e, double err_rate, 
							  boolean* any_erm_present),
					  StrBuf* tmpbuf,
					  StrBuf* install_dir,
					  int ignore_first, int ignore_last, int expected_covg,
					  double lambda_g, double lambda_e, double err_rate, OutputFormat format, boolean output_last,//for JSON 
					  boolean* any_erm_present
					 )
{
  Troolean suc;
  
  suc  = func(db_graph,
	      file_reader,
	      rutils,
	      tmp_rvi,
	      tmp_gi,
	      abi,
	      install_dir,
	      ignore_first, ignore_last, expected_covg,
	      lambda_g,
	      lambda_e,
	      err_rate,
	      any_erm_present);

  map_antibiotic_enum_to_str(abi->ab, tmpbuf);

  if (format==Stdout)
    {
      printf("%s\t", tmpbuf->buff);
      if (suc==_False)
	{
	  printf("R\n");
	}
      else if (suc==_True)
	{
	  printf("S\n");
	}
      else
	{
	  printf("N\n");
	}
    }
  else
    {
      if (suc==_True)
	{
	  print_json_item(tmpbuf->buff, "S", output_last);
	}
      else if (suc==_False)
	{
	  print_json_item(tmpbuf->buff, "R", output_last);
	}
      else
	{
	  print_json_item(tmpbuf->buff, "Inconclusive", output_last);
	}
    }

}


void print_clindamycin_susceptibility(dBGraph* db_graph,
					 int (*file_reader)(FILE * fp, 
							    Sequence * seq, 
							    int max_read_length, 
							    boolean new_entry, 
							    boolean * full_entry),
					 ReadingUtils* rutils,
					 ResVarInfo* tmp_rvi,
					 GeneInfo* tmp_gi,
					 AntibioticInfo* abi,
					 Troolean (*func)(dBGraph* db_graph,
							 int (*file_reader)(FILE * fp, 
									    Sequence * seq, 
									    int max_read_length, 
									    boolean new_entry, 
									    boolean * full_entry),
							 ReadingUtils* rutils,
							 ResVarInfo* tmp_rvi,
							 GeneInfo* tmp_gi,
							 AntibioticInfo* abi,
							 StrBuf* install_dir,
							 int ignore_first, int ignore_last, int expected_covg,
							 double lambda_g, double lambda_e, double err_rate),
					 StrBuf* tmpbuf,
					 boolean any_erm_present,
					 StrBuf* install_dir,
					 int ignore_first, int ignore_last, int expected_covg,
					 double lambda_g, double lambda_e, double err_rate, OutputFormat format, boolean output_last//for JSON 
					 )
{
  Troolean suc;
  
  suc  = func(db_graph,
	      file_reader,
	      rutils,
	      tmp_rvi,
	      tmp_gi,
	      abi,
	      install_dir,
	      ignore_first, ignore_last, expected_covg,
	      lambda_g,
	      lambda_e,
	      err_rate);


  map_antibiotic_enum_to_str(abi->ab, tmpbuf);

  if (format==Stdout)
    {
      printf("%s\t", tmpbuf->buff);
      if (suc==_False)
	{
	  printf("R(constitutive)\n");
	}
      else if ( (suc==_True) && (any_erm_present==true) )
	{
	  printf("R(inducible)\n");
	}
      else if (suc==_True)
	{
	  printf("S\n");
	}
      else
	{
	  printf("N\n");
	}
    }
  else
    {
      if (suc==_False)
	{
	  print_json_item(tmpbuf->buff, "R(constitutive)", output_last);
	}
      else if ( (suc==_True) && (any_erm_present==true) )
	{
	  print_json_item(tmpbuf->buff, "R(inducible)", output_last);
	}
      else if (suc==_True)
	{
	  print_json_item(tmpbuf->buff, "S", output_last);
	}
      else
	{
	  print_json_item(tmpbuf->buff, "Inconclusive", output_last);
	}
    }

}




///virulence
Troolean is_pvl_positive(dBGraph* db_graph,
			   int (*file_reader)(FILE * fp, 
					      Sequence * seq, 
					      int max_read_length, 
					      boolean new_entry, 
					      boolean * full_entry),
			ReadingUtils* rutils,
			GeneInfo* tmp_gi,
			StrBuf* install_dir)
			   

{
  StrBuf* fa = strbuf_create(install_dir->buff);
  strbuf_append_str(fa, "data/staph/virulence/luks.fa");

  FILE* fp = fopen(fa->buff, "r");
  if (fp==NULL)
    {
      die("Cannot open %s\n", fa->buff);
    }
  int num=1;
  boolean is_pos=false;
  while (num>0)
    {
      num = get_next_gene_info(fp,
			       db_graph,
			       tmp_gi,
			       rutils->seq,
			       rutils->kmer_window,
			       file_reader,
			       rutils->array_nodes,
			       rutils->array_or,
			       rutils->working_ca,
			       MAX_LEN_GENE);

      if (tmp_gi->percent_nonzero > MIN_PERC_COVG_STANDARD)
	{
	  is_pos=true;
	}
    }
  fclose(fp);
  return is_pos;

}


void print_pvl_presence(dBGraph* db_graph,
			int (*file_reader)(FILE * fp, 
					   Sequence * seq, 
					   int max_read_length, 
					   boolean new_entry, 
					   boolean * full_entry),
			ReadingUtils* rutils,
			GeneInfo* tmp_gi,
			Troolean (*func)(dBGraph* db_graph,
					int (*file_reader)(FILE * fp, 
							   Sequence * seq, 
							   int max_read_length, 
							   boolean new_entry, 
							   boolean * full_entry),
					ReadingUtils* rutils,
					GeneInfo* tmp_gi,
					StrBuf* install_dir),
			StrBuf* install_dir, OutputFormat format)
{

  Troolean result = is_pvl_positive(db_graph, file_reader, rutils, tmp_gi, install_dir);
  
  if (format==Stdout)
    {
      printf("PVL\t");
      if (result==true)
	{
	  printf("positive\n");
	}
      else
	{
	  printf("negative\n");
	}
    }
  else
    {
      print_json_virulence_start();
      if (result==true)
	{
	  print_json_item("PVL", "positive", true);
	}
      else
	{
	  print_json_item("PVL", "negative", true);
	}
      print_json_virulence_end();
    }
}
