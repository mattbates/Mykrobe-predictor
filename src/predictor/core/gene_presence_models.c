/*
 * Copyright 2015 Zamin Iqbal (zam@well.ox.ac.uk)
 * 
 *
 *  gene_presence_models.h
*/

#include "math.h"
#include "maths.h"
#include "gene_presence_models.h"

//on divergence between the gene panel
//epsilon =  pow(1-err_rate, cmd_line->kmer_size)
double get_log_posterior_major_resistant(double llk,
					 GeneInfo* gi,
					 double recovery_given_sample_and_errors,
					 double err_rate,
					 int min_expected)//given known diversity of genes

{

  int p = gi->percent_nonzero;


  /*  double freq;


  //depending on err rate, set freq
  if (err_rate<0.02)
    {
      freq=0.05;
    }
  else if (err_rate<0.1)
    {
      freq=0.25;
    }
  else
    {
      freq=0.5;
    }
  */

  if (p>=recovery_given_sample_and_errors* min_expected)
    {
      return log(1)+llk;
    }
  /*  else if (p>((0.75-freq)/2)*recovery_given_sample_and_errors* min_expected)
    {
      return log(0.5)+llk;
      } */
  else
    {
      return -99999999;
    } 
}



double get_log_posterior_minor_resistant(double llk,
					 GeneInfo* gi,
					 double recovery_given_sample_and_errors,
					 int expected_covg,
					 double err_rate,
					 int min_expected)//given known diversity of genes

{

  int p = gi->percent_nonzero;

  double freq;

  if (err_rate<0.01)
    {
      //i don't believe the error rate is that low, so fix
      err_rate=0.01;
    }

  if (err_rate<0.02)
    {
      freq=0.05;
    }
  else if (err_rate<0.1)
    {
      freq=0.25;
    }
  else
    {
      return -99999999;
    }

  //what % of the gene do you expect to see at this frequency?
  double exp_rec = 1-exp(-expected_covg*freq);


  //double step function. Coverage gap as might expect for this low frequency
  if ( (p>=exp_rec*100)//need to see enough of the gene
    &&
       (p>=min_expected) )
    //&&
       //       (gi->median_covg_on_nonzero_nodes<freq*expected_covg) )//but it needs not to be repeats
    {
      return log(1)+llk;
    }
  else
    {
      return -99999999;
    } 
}



double get_log_posterior_truly_susceptible(double llk,
					   GeneInfo* gi,
					   double recovery_given_sample_and_errors,
					   int min_expected)
{

  //  int p = gi->percent_nonzero;
  
  /*  if (p>=recovery_given_sample_and_errors*min_expected)
    {
      return -99999999;
    }
  else
  {*/
      return log(1)+llk;;
      //   }
}



// lambda_g = expected_covg/mean_read_len on real allele
/*
double get_log_lik_minor_resistant(GeneInfo* gi,
				   double lambda_g,
				   double err_rate,
				   int kmer)
{

  double p;
  if (err_rate<0.02)
    {
      p = 5*err_rate;
    }
  else if (err_rate<0.1)
    {
      p=0.25
    }
  else
    {
      return -9999999;
    }
  return get_gene_log_lik(gi->median_covg_on_nonzero_nodes, 
			  p*lambda_g, 
			  kmer);

}
*/


// lambda_g = expected_covg/mean_read_len on real allele
 /*
double get_log_lik_major_resistant(GeneInfo* gi,
				   double lambda_g,
				   int kmer)
{
  //resistant pop is 75% freq

  return get_gene_log_lik(gi->median_covg_on_nonzero_nodes, 
			  0.75*lambda_g, 
			  kmer);

}
 */

// epsilon = (1-e)^k
// delta = e(1-e)^(k-1)
// lambda = expected_covg/mean_read_len
double get_log_lik_truly_susceptible(GeneInfo* gi,
				     double lambda_e,
				     int kmer)
{
  
  return get_log_lik_covg_due_to_errors(gi->median_covg_on_nonzero_nodes,
					gi->percent_nonzero,
					gi->len,
					lambda_e, kmer);
}


//use number of gaps (to see if contiguous), plus median covg
double get_log_lik_resistant(GeneInfo* gi,
			     double lambda_g,
			     double freq,//between 0 and 1
			     int expected_covg,
			     int kmer)
{
  double ret =get_gene_log_lik(gi->median_covg,
			       lambda_g*freq, kmer) +  
    log_prob_gaps(gi, expected_covg);
  return ret;

}

double log_prob_gaps(GeneInfo* gi, int expected_covg)
{
  //  printf("Num gaps is %d\n", gi->num_gaps);
  return -expected_covg*gi->num_gaps;
}




// epsilon = (1-e)^k
// lambda = expected_covg/mean_read_len
double get_gene_log_lik(Covg covg,//median covg on parts of gene that are present
			double lambda_g, 
			int kmer)
{
  //Under this model, covg on th true allele is Poisson distributed
  //with rate at true allele   r_t = (1-e)^k  *  depth/read-length =: lambda_g
  double r_t = lambda_g;
  
  // P(covg) = exp(-r_t) * (r_t)^covg /covg!
  
  double log_lik_true_allele  
    = -r_t 
    + covg*log(r_t) 
    - log_factorial(covg);
    
  return log_lik_true_allele;

}

double get_log_lik_covg_due_to_errors(Covg covg,
				      int percent_nonzero,
				      int gene_len,
				      double lambda_e,
				      int kmer)
{
  //num positive kmers
  int p = (int) (percent_nonzero*gene_len/100);

  //rescale - number of SNP errors
  if (p>kmer)
    {
      p = p/kmer;
    }
  else
    {
      p =1;
    }

    //Covg on the err allele is Poisson distributed with 
  // rate at error allele = e * (1-e)^(k-1) * (D/R) /3 =: lambda_e
  double r_e = lambda_e;
  
  double log_lik_err_allele  
    = -r_e 
    + p*covg*log(r_e) 
    - log_factorial(covg*p);
  
  return log_lik_err_allele;

}



void choose_ml_gene_model(double llk_R, double llk_S, double llk_M,
			  Model* best_model)
{
  Model mR;
  mR.type=Resistant;
  mR.likelihood=llk_R;
  mR.lp = 0;
  mR.conf=0;
  Model mM;
  mM.type=MixedInfection;
  mM.likelihood=llk_M;
  mM.lp = 0;
  mM.conf=0;
  Model mS;
  mS.type=Susceptible;
  mS.likelihood=llk_S;
  mS.lp = 0;
  mS.conf=0;

  Model arr[3]={mR, mS, mM};
  qsort(arr, 3, sizeof(Model), model_cmp_loglik);
  best_model->conf = arr[2].likelihood-arr[1].likelihood;
  best_model->type = arr[2].type;
  best_model->likelihood = arr[2].likelihood;
  best_model->lp =0;
}



//max a posteriori
void choose_map_gene_model(GeneInfo* gi,
			   double llk_R, double llk_S,  double llk_M,
			   Model* best_model, double epsilon, 
			   double err_rate, int expected_covg,
			   int min_expected_kmer_recovery_for_this_gene)
{

  //total length of gaps = length of gene * exp(-expected_covg)
  // so percentage of kmers present = 1-exp(-expected_covg);
  double loss = 1-exp(-expected_covg);

  Model mR;
  mR.type=Resistant;
  mR.likelihood=llk_R;
  mR.lp=0;
  mR.conf=0;
  Model mS;
  mS.type=Susceptible;
  mS.likelihood=llk_S;
  mS.lp=0;
  mS.conf=0;
  Model mM;
  mM.type=MixedInfection;
  mM.likelihood=llk_M;
  mM.lp=0;
  mM.conf=0;

  mR.lp 
    = get_log_posterior_major_resistant(llk_R, 
					gi,
					loss,
					err_rate,
					min_expected_kmer_recovery_for_this_gene);

  mM.lp 
    = get_log_posterior_minor_resistant(llk_M, 
					gi,
					loss,
					expected_covg,
					err_rate,
					min_expected_kmer_recovery_for_this_gene);


  mS.lp 
    = get_log_posterior_truly_susceptible(llk_S, 
					  gi,
					  loss,
					  min_expected_kmer_recovery_for_this_gene);

  Model arr[3]={mR, mS, mM};
  qsort(arr, 3, sizeof(Model), model_cmp_logpost);
  best_model->conf = arr[2].lp-arr[1].lp;
  best_model->type = arr[2].type;
  best_model->likelihood = arr[2].likelihood;
  best_model->lp = arr[2].lp;
}


InfectionType resistotype_gene(GeneInfo* gi, double err_rate, int kmer,
			       double lambda_g,  double lambda_e, double epsilon, int expected_covg,
			       Model* best_model,
			       ModelChoiceMethod choice,
			       int min_expected_kmer_recovery_for_this_gene)
{


  //prevent having too small err rate
  if (err_rate<0.001)
    {
      //i don't believe the error rate is that low, so fix
      err_rate=0.005;
    }


  //depending on err rate, set freq
  double freq;
  if (err_rate<0.02)
    {
      freq=0.05;
    }
  else if (err_rate<0.1)
    {
      freq=0.25;
    }
  else
    {
      freq=0.5;
    }


  double llk_R = get_log_lik_resistant(gi, lambda_g, 1, expected_covg, kmer);
  double llk_M = get_log_lik_resistant(gi, lambda_g, freq, expected_covg, kmer);
  double llk_S = get_log_lik_truly_susceptible(gi, 
					       lambda_e, 
					       kmer);

  //  printf("LLks of S, M, R are %f, %f and %f\n", llk_S, llk_M, llk_R);
  best_model->conf=0;
  if (choice==MaxLikelihood)
    {
      choose_ml_gene_model(llk_R, llk_S, llk_M, best_model);
    }
  else
    {
      choose_map_gene_model(gi, llk_R, llk_S, llk_M,
			    best_model, epsilon, err_rate, expected_covg,
			    min_expected_kmer_recovery_for_this_gene);
    }

  if (best_model->conf > MIN_CONFIDENCE_GENE)
    {
      return best_model->type;
    }
  else
    {
      return Unsure;
    }
}

