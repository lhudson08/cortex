#include <simulator.h>
#include "CUnit.h"
#include <math.h>
#include <db_variants.h>
#include <dB_graph.h>
#include <dB_graph_population.h>
#include <maths.h>
#include <gsl_sf_gamma.h>
#include <db_complex_genotyping.h>
#include <string.h>
#include <model_selection.h>
#include <stdio.h>
#include <stdlib.h>
#include <global.h>
#include <file_reader.h>
#include <gsl_rng.h>


void mark_allele_with_all_1s_or_more(dBNode** allele, int len, int colour)
{
  int i;
  for (i=0; i<len; i++)
    {
      db_node_increment_coverage(allele[i], individual_edge_array, colour);
    }
}

// len is number of edges,
void update_allele(dBNode** allele, int len, int colour, int covg, int eff_read_len)
{

  const gsl_rng_type * T;
  gsl_rng * r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc (T);
     
  int i;
  //add covg number of reads, placing them randomly on the allele
  for (i = 0; i < covg; i++) 
    {
      int u=0;
      while (u==0)
	{
	  u = gsl_rng_uniform_int(r,len);
	}
      //now add coverage:
      int j;
      for (j=u; (j<u+eff_read_len) && (j<len); j++)
	{
	  db_node_increment_coverage(allele[j], individual_edge_array, colour);
	}
    }
     
  gsl_rng_free (r);
     

}


void zero_allele(dBNode** allele, int len, int colour_indiv, int colour_allele1, int colour_allele2, int colour_ref_minus_site)
{
  int i;
  for (i=0; i<len; i++)
    {
      db_node_set_coverage(allele[i], individual_edge_array,  colour_indiv ,0);
      db_node_set_coverage(allele[i], individual_edge_array,  colour_allele1 ,0);
      db_node_set_coverage(allele[i], individual_edge_array,  colour_allele2 ,0);
      db_node_set_coverage(allele[i], individual_edge_array,  colour_ref_minus_site,0);
    }
}


void zero_path_except_two_alleles_and_ref(dBNode** allele, int len, int colour_allele1, int colour_allele2, int colour_ref_minus_site)
{
  int i,j;
  for (i=0; i<len; i++)
    {
      for (j=0; j<NUMBER_OF_COLOURS; j++)
	{
	  if (!( (j==colour_allele1) || (j==colour_allele2) || (j==colour_ref_minus_site) ))
	    {
	      db_node_set_coverage(allele[i], individual_edge_array,  j ,0);
	      db_node_set_coverage(allele[i], individual_edge_array,  j ,0);
	      db_node_set_coverage(allele[i], individual_edge_array,  j, 0);
	      db_node_set_coverage(allele[i], individual_edge_array,  j, 0);
	    }
	}
    }
}


//if you want to simulate a true hom, pass in var with both alleles the same
void simulator(int depth, int read_len, int kmer, double seq_err_per_base, int number_repetitions, 
	       int colour_indiv, int colour_allele1, int colour_allele2, int colour_ref_minus_site,
	       VariantBranchesAndFlanks* var, dBNode** genome_minus_site, int len_genome_minus_site,
	       boolean are_the_two_alleles_identical,
	       //MultiplicitiesAndOverlapsOfBiallelicVariant* var_mults, 
	       GraphAndModelInfo* model_info,
	       char* fasta, char* true_ml_gt_name, dBGraph* db_graph, int working_colour1, int working_colour2)

{

  if (NUMBER_OF_COLOURS<4)
    {
      printf("Cannot run the simulator with <4 colours. Recompile.\n");
      exit(1);
    }


  int count_passes=0;
  int count_fails=0;

  ///********* local function **********
  void test(VariantBranchesAndFlanks* var, //MultiplicitiesAndOverlapsOfBiallelicVariant* var_mults, 
	    GraphAndModelInfo* model_info, int colour_allele1, int colour_allele2, 
	    int colour_ref_minus_site, int colour_indiv)
  {

    int max_allele_length=100000;
    if ( (var->len_one_allele>max_allele_length)||(var->len_other_allele > max_allele_length) )
      {
	exit(1);
      }
    int colours_to_genotype[]={colour_indiv};

    char ml_genotype_name[100]="";
    char* ml_genotype_name_array[1];
    ml_genotype_name_array[0]=ml_genotype_name;
    char ml_but_one_genotype_name[100]="";
    char* ml_but_one_genotype_name_array[1];
    ml_but_one_genotype_name_array[0]=ml_but_one_genotype_name;

    int MIN_LLK = -999999999;
    double ml_genotype_lik=MIN_LLK;
    double ml_but_one_genotype_lik=MIN_LLK;

    calculate_max_and_max_but_one_llks_of_specified_set_of_genotypes_of_complex_site(colours_to_genotype, 1,
										     colour_ref_minus_site, 2,
										     1,3,
										     max_allele_length, fasta,
										     AssumeUncleaned,
										     &ml_genotype_lik, &ml_but_one_genotype_lik,
										     ml_genotype_name_array, ml_but_one_genotype_name_array,
										     false, model_info, db_graph, working_colour1, working_colour2);

    //printf("We get max lik gt %s and we expect %s\n", ml_genotype_name, true_ml_gt_name);
    if (strcmp(ml_genotype_name, true_ml_gt_name)==0) 
      {
	count_passes++;
      }
    else
      {
	count_fails++;
      }
  }
  ///********* end of local function **********



     
  const gsl_rng_type * T;
  gsl_rng * r;
  
  // GLS setup:
  /* create a generator chosen by the 
     environment variable GSL_RNG_TYPE */
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc (T);


  //put the alleles  and reference into their colours:
  mark_allele_with_all_1s_or_more(var->one_allele, var->len_one_allele,     colour_allele1);
  mark_allele_with_all_1s_or_more(var->other_allele, var->len_other_allele, colour_allele2);
  
  int i;
  for (i=0; i<number_repetitions; i++)
    {
      zero_path_except_two_alleles_and_ref(var->one_allele, var->len_one_allele, colour_allele1, colour_allele2, colour_ref_minus_site);

      //give the each allele depth which is taken from a Poisson with mean =  (D/2) * (R-k+1)/R  * (1-k*epsilon)
      //printf("Depth %d, var->len one allele - k,er +1 = %d, and 1-kmer * seq_err = %f     \n", depth, (var->len_one_allele)-kmer+1, 1-kmer*seq_err_per_base    )    ;
      double exp_depth_on_allele1 = ((double) depth/2) * ( (double)(read_len+(var->len_one_allele)-kmer+1)/read_len) * (1-kmer*seq_err_per_base);
      //      if (are_the_two_alleles_identical==false)
      //	{
      //  exp_depth_on_allele1 +=  (read_len+(var->len_other_allele)-kmer+1) * ((double)depth/2) *kmer*seq_err_per_base  ;
      //}
      double exp_depth_on_allele2 = ((double) depth/2) * ( (double)(read_len+(var->len_other_allele)-kmer+1)/read_len) * (1-kmer*seq_err_per_base);
      //if (are_the_two_alleles_identical==false)
      //{	
      //  exp_depth_on_allele2 +=  (read_len+(var->len_one_allele)-kmer+1) * ((double)depth/2) *kmer*seq_err_per_base  ;
      //}
      double exp_depth_on_ref_minus_site = (double) depth * ((double)(len_genome_minus_site-kmer+1)/read_len) * (1-kmer*seq_err_per_base);
      //printf("exp ZAMMER %f %f %f\n", exp_depth_on_allele1, exp_depth_on_allele2, exp_depth_on_ref_minus_site);
      unsigned int sampled_covg_allele1 = gsl_ran_poisson (r, exp_depth_on_allele1);
      unsigned int sampled_covg_allele2 = gsl_ran_poisson (r, exp_depth_on_allele2);
      unsigned int sampled_covg_rest_of_genome = gsl_ran_poisson (r, exp_depth_on_ref_minus_site);
      //printf("Sampled covgs on alleles 1,2 and genome are  %d %d %d\n", sampled_covg_allele1, sampled_covg_allele2, sampled_covg_rest_of_genome);
      update_allele(var->one_allele, var->len_one_allele,     colour_indiv, sampled_covg_allele1,read_len-kmer+1);
      update_allele(var->other_allele, var->len_other_allele, colour_indiv, sampled_covg_allele2, read_len-kmer+1);
      //update_allele(genome_minus_site,len_genome_minus_site,  colour_indiv, sampled_covg_rest_of_genome);
      test(var,  model_info, colour_allele1, colour_allele2, colour_ref_minus_site, colour_indiv);

    }

  //cleanup
  zero_allele(var->one_allele, var->len_one_allele,     colour_indiv, colour_allele1, colour_allele2, colour_ref_minus_site);
  zero_allele(var->other_allele, var->len_other_allele, colour_indiv, colour_allele1, colour_allele2, colour_ref_minus_site);

  CU_ASSERT((double)count_passes/(double)(count_passes+count_fails) > 0.9 );//actually, we could set this to ==1
  printf("Number of passes: %d, number of fails %d\n", count_passes, count_fails);
    

  gsl_rng_free (r);

}