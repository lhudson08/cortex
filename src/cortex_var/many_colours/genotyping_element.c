/*
 * Copyright 2009-2011 Zamin Iqbal and Mario Caccamo 
 * 
 * CORTEX project contacts:  
 * 		M. Caccamo (mario.caccamo@bbsrc.ac.uk) and 
 * 		Z. Iqbal (zam@well.ox.ac.uk)
 *
 * **********************************************************************
 *
 * This file is part of CORTEX.
 *
 * CORTEX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CORTEX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CORTEX.  If not, see <http://www.gnu.org/licenses/>.
 *
 * **********************************************************************
 */



#include <genotyping_element.h>
#include <stdlib.h>
#include <global.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

//currently noone calls this in normal use
// In normal use, the priority queue allocates space to put the eloement directly within,
// and calls genotyping_element_initialise
GenotypingElement* new_genotyping_element()
{


  GenotypingElement* e = malloc(sizeof(GenotypingElement));

  if (e==NULL)
    {
      printf("Unable to allocate a new genotyping_element");
      exit(1);
    }
  
  binary_kmer_initialise_to_zero(&(e->kmer));

  int i;
  for (i=0; i< MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      e->individual_edges[i]=0;
      e->coverage[i]=0;
    }


  e->status=none;
  
  return e;
}


void free_genotyping_element(GenotypingElement** element)
{
  free(*element);
  *element=NULL;
}

void genotyping_element_assign(GenotypingElement* e1, GenotypingElement* e2)
{

  if ((e1==NULL)||(e2==NULL))
    {
      return;
    }

  binary_kmer_assignment_operator( (*e1).kmer, (*e2).kmer);
  int i;
  for (i=0; i< MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      e1->individual_edges[i] = e2->individual_edges[i];
      e1->coverage[i]         = e2->coverage[i];
    }
  e1->status = e2->status;
}


// the GenotypingElement has 2 extra colours on the end for working
// and MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING extra colours before them for the alleles
void genotyping_element_initialise_from_normal_element(GenotypingElement* e1, 
						       Element* e2,
						       boolean set_status_to_none)
						    
{
  if (e2==NULL)
    {
      e1=NULL;
      return;
    }

  binary_kmer_assignment_operator( (*e1).kmer, (*e2).kmer);
  int i;
  for (i=0; i< NUMBER_OF_COLOURS; i++)
    {
      e1->individual_edges[i] = e2->individual_edges[i];
      e1->coverage[i]         = e2->coverage[i];
    }
  if (set_status_to_none)
    {
      e1->status=none;
    }
  else
    {
      e1->status = e2->status;
    }
  for (i=NUMBER_OF_COLOURS; i<MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      e1->individual_edges[i] =0;
      e1->coverage[i]         =0;
    }
}




//gets you a pointer to the edge you are referring to
//Don't use this if you can use get_edge_copy instead
Edges* genotyping_node_get_edge(GenotypingElement e, EdgeArrayType type,int index)
{
  if (type == individual_edge_array)
    {
      if (index>=MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2)
	{
	  printf("Called genotyping_node_get_edge with index %d which is >= MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2 which is %d. Exist\n", index, MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2);
	  exit(1);
	}
      return &e.individual_edges[index];//compiler thinks this is returning a local variable, but assuming e is in the hash table, it has a lifetime beyond that of this function.
    }
 else 
    {
      printf("Coding error. Only expecting enum of edge array types to contain one type - individual_edge_array, but we are getting type %d", type);
      exit(1);
    }

  exit(1);
}


//return a copy of the edge you are referring to
Edges genotyping_node_get_edge_copy(const GenotypingElement e, EdgeArrayType type,int index)
{

  if (type == individual_edge_array)
    {
      if (index>=MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2)
	{
	  printf("Trying to access a colour beyond the compile-time limit. Exit.\n");
	  exit(1);
	}
      return e.individual_edges[index];
    }
  else 
    {
      printf("Coding error. Only expecting enum of edge array types to contain one type - individual_edge_array, but we are getting type %d", type);
      exit(1);
    }

  exit(1);
}



Edges genotyping_node_get_union_of_edges(GenotypingElement e)
{

  int i;
  Edges edges=0;

  for (i=0; i< MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      edges |= e.individual_edges[i];
    }

  return edges;
}

Edges genotyping_element_get_colour_union_of_all_colours(const GenotypingElement* e)
{
  int i;
  Edges edges=0;
  
  for (i=0; i< MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      edges |= e->individual_edges[i];
    }

  return edges;
  
}



Edges genotyping_element_get_last_colour(const GenotypingElement* e)
{
  int i;
  Edges edges =  genotyping_node_get_edge_copy(*e, individual_edge_array, MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2-1);
  return edges;
}


Edges genotyping_element_get_colour0(const GenotypingElement* e)
{
  Edges edges=genotyping_node_get_edge_copy(*e, individual_edge_array,0);
  return edges;
}

Edges genotyping_element_get_colour1(const GenotypingElement* e)
{
  Edges edges=genotyping_node_get_edge_copy(*e, individual_edge_array,1);
  return edges;
}



int genotyping_element_get_covg_union_of_all_covgs(const GenotypingElement* e)
{
  int i;
  int covg=0;
  
  for (i=0; i< MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      covg += e->coverage[i];
    }

  return covg;
  
}


int genotyping_element_get_covg_colour0(const GenotypingElement* e)
{
  return e->coverage[0];
}

int genotyping_element_get_covg_last_colour(const GenotypingElement* e)
{
  return e->coverage[MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2-1];
}

int genotyping_element_get_covg_colour1(const GenotypingElement* e)
{
  return e->coverage[1];
}





//adds edges from edge_char to the appropriate person/population edgeset, without removing existing edges
void genotyping_node_add_edges(GenotypingElement* e, EdgeArrayType type, int index, Edges edge_char)
{
  if (type == individual_edge_array)
    {
      if (index>=MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2)
	{
	  printf("in genotyping_element's add_edges function. index is %d, and should be at most %d", index, MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2-1);
	  exit(1);
	}
      e->individual_edges[index] |= edge_char;
    }

  else
    {
      printf("Coding error. Only expecting enum of edge array types to contain one type - individual_edge_array, but we are getting type %d", type);
      exit(1);
    }
  
}


void genotyping_node_set_edges(GenotypingElement* e, EdgeArrayType type, int index, Edges edge_char)
{
  if (type == individual_edge_array)
    {
      if (index>=MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2)
	{
	  printf("in genotyping_element's set_edges function. index is %d,and should be at most %d", index, MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2);
	  exit(1);
	}
      e->individual_edges[index] = edge_char;
    }

  else
    {
      printf("Coding error. Only expecting enum of edge array types to contain one type - individual_edge_array, but we are getting type %d", type);
      exit(1);
    }
  
}


void db_genotyping_node_reset_all_edges_for_all_people_and_pops_to_zero(GenotypingElement* e)
{
  int i;

    for (i=0; i<MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      e->individual_edges[i]=0;
    }

}

void genotyping_node_reset_one_edge(GenotypingElement* e, Orientation orientation, Nucleotide nucleotide, EdgeArrayType type, int index)
{
  if (type == individual_edge_array)
    {
      if (index>=MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2)
	{
	  printf("in genotyping_element's reset_one_edge function. index is %d,and should be at most %d - 1", index, MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2);
	  exit(1);
	}

      char edge = 1 << nucleotide;      
      if (orientation == reverse){
	edge <<= 4;
      }
      //toggle 1->0 0->1
      edge ^= (unsigned char) 0xFF; //xor with all 1's, ie 00010000 -> 11101111
      
      e->individual_edges[index] &= edge; //reset one edge
      

    }
  else
    {
      printf("Coding error. Only expecting enum of edge array types to contain one type - individual_edge_array, but we are getting type %d", type);
      exit(1);
    }
  
}


int genotyping_element_get_number_of_people_or_pops_containing_this_element(GenotypingElement* e, EdgeArrayType type, int index)
{
  int i;
  int count=0;
  if (type == individual_edge_array)
    {
      for (i=0; i< MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
	{
	  if ( (e->individual_edges)[i] != 0)
	    {
	      count++;
	    }
	}
    }
  else
    {
      printf("Coding error. Only expecting enum of edge array types to contain one type - individual_edge_array, but we are getting type %d", type);
      exit(1);
    }

  return count;
}

boolean genotyping_element_smaller(GenotypingElement  e1, GenotypingElement e2){
  return genotyping_node_get_union_of_edges(e1)  <  genotyping_node_get_union_of_edges(e2);
}



//WARNING - this gives you a pointer to a the binary kmer in the node. You could modify contents of the hash table
BinaryKmer* genotyping_element_get_kmer(GenotypingElement * e){
  return &(e->kmer);
}

boolean genotyping_element_is_key(Key key, GenotypingElement e, short kmer_size){
  //  return key == e.kmer;
  return binary_kmer_comparison_operator(*key, e.kmer);
}

Key genotyping_element_get_key(BinaryKmer* kmer, short kmer_size, Key preallocated_key){
  
  BinaryKmer local_rev_kmer;
  binary_kmer_initialise_to_zero(&local_rev_kmer);

  binary_kmer_reverse_complement(kmer,kmer_size, &local_rev_kmer);
  
  if (binary_kmer_less_than(local_rev_kmer,*kmer, kmer_size))
    {
      binary_kmer_assignment_operator(*((BinaryKmer*)preallocated_key),local_rev_kmer);
    }
  else
    {
      binary_kmer_assignment_operator(*((BinaryKmer*)preallocated_key),*kmer);
    }

  return preallocated_key;

}


void genotyping_element_initialise(GenotypingElement * e, Key kmer, short kmer_size){

  if (e==NULL)
    {
      printf("Called elemtn_initialise on NULL ptr");
      exit(1);
    }

  BinaryKmer tmp_kmer;
  binary_kmer_initialise_to_zero(&tmp_kmer);
  binary_kmer_assignment_operator( e->kmer, *(genotyping_element_get_key(kmer, kmer_size, &tmp_kmer)));

  //hash table has calloc-ed all elements, so elements fromm the hash table are already initialised to zero.
  //however this function is used to reset to 0 GenotypingElements that are reused,
  // - see below in the read_binary functions. Also in tests.
  int i;
  for (i=0; i<MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      e->individual_edges[i]=0;
      e->coverage[i]=0;
    }

  db_genotyping_node_set_status(e, none);

}





void genotyping_element_initialise_kmer_covgs_edges_and_status_to_zero(GenotypingElement * e){

  if (e==NULL)
    {
      printf("Called genotyping_element_initialise_covgs_and_edges_to_zero  on NULL ptr");
      exit(1);
    }

  BinaryKmer tmp_kmer;
  binary_kmer_initialise_to_zero(&(e->kmer));
  //binary_kmer_assignment_operator( e->kmer, &tmp_kmer);

  int i;
  for (i=0; i<MAX_ALLELES_SUPPORTED_FOR_STANDARD_GENOTYPING+NUMBER_OF_COLOURS+2; i++)
    {
      e->individual_edges[i]=0;
      e->coverage[i]=0;
    }

  db_genotyping_node_set_status(e, none);

}




void genotyping_element_set_kmer(GenotypingElement * e, Key kmer, short kmer_size){

  if (e==NULL)
    {
      printf("Called element_set_kmer on NULL ptr");
      exit(1);
    }

  BinaryKmer tmp_kmer;
  binary_kmer_initialise_to_zero(&tmp_kmer);
  binary_kmer_assignment_operator( e->kmer, *(genotyping_element_get_key(kmer, kmer_size, &tmp_kmer)));

}







void db_genotyping_node_increment_coverage(GenotypingElement* e, EdgeArrayType type, int index)
{
  if (e==NULL)
    {
      return;
    }
  e->coverage[index]=e->coverage[index]+1;
}
void db_genotyping_node_decrement_coverage(GenotypingElement* e, EdgeArrayType type, int index)
{
  if (e==NULL)
    {
      return;
    }
  if (e->coverage[index]>0)
    {
      e->coverage[index]=e->coverage[index]-1;
    }
}

void db_genotyping_node_update_coverage(GenotypingElement* e, EdgeArrayType type, int index, int update)
{

  e->coverage[index] += update;

}


int db_genotyping_node_get_coverage(const GenotypingElement* const e, EdgeArrayType type, int index)
{

  if (e==NULL)
    {
      return 0;
    }
  else
    {
      return e->coverage[index];
    }
}


void db_genotyping_node_set_coverage(GenotypingElement* e, EdgeArrayType type, int colour, int covg)
{

  if (e==NULL)
    {
      return;
    }
  else
    {
      e->coverage[colour] = covg;
      return;
    }
}



int db_genotyping_node_get_coverage_in_subgraph_defined_by_func_of_colours(const GenotypingElement* const e, int (*get_covg)(const GenotypingElement*) )
{

  if (e==NULL)
    {
      return 0;
    }
  else
    {
      return get_covg(e);
    }
}





Orientation db_genotyping_node_get_orientation(BinaryKmer* k, GenotypingElement * e, short kmer_size){

  if (binary_kmer_comparison_operator(e->kmer,*k)==true)
    {
      return forward;
    }
  
  BinaryKmer tmp_kmer;

  if (binary_kmer_comparison_operator(e->kmer, *(binary_kmer_reverse_complement(k,kmer_size, &tmp_kmer)))==true)
    {
      return reverse;
    }
  
  printf("programming error - you have called  db_genotyping_node_get_orientation with a kmer that is neither equal to the kmer in this node, nor its rev comp\n");
  char tmpseq1[kmer_size+1];
  char tmpseq2[kmer_size+1];
  printf("Arg 1 Kmer is %s and Arg 2 node kmer is %s\n", binary_kmer_to_seq(k, kmer_size, tmpseq1), binary_kmer_to_seq(&(e->kmer), kmer_size, tmpseq2));
  exit(1);
  
}





//After specifying which individual or population you are talking about, this function
//adds one edge ("arrow") to the appropriate edge in the appropriate array in the element -- basically sets a bit in the correct edges char
void db_genotyping_node_add_labeled_edge(GenotypingElement * e, Orientation o, Nucleotide base, EdgeArrayType edge_type, int edge_index){

  //set edge 
  char edge = 1 << base; // A (0) -> 0001, C (1) -> 0010, G (2) -> 0100, T (3) -> 1000
  
  if (o == reverse){
    edge <<= 4; //move to next nibble 
  }

  //update node
  genotyping_node_add_edges(e, edge_type, edge_index, edge);
  
}


//adding an edge between two nodes implies adding two labeled edges (one in each direction)
//be aware that in the case of self-loops in palindromes the two labeled edges collapse in one

boolean db_genotyping_node_add_edge(GenotypingElement * src_e, GenotypingElement * tgt_e, Orientation src_o, Orientation tgt_o, short kmer_size, EdgeArrayType edge_type, int edge_index){

  BinaryKmer src_k, tgt_k, tmp_kmer; 
  char seq1[kmer_size+1];
  char seq2[kmer_size+1];

  binary_kmer_assignment_operator(src_k, src_e->kmer);
  binary_kmer_assignment_operator(tgt_k, tgt_e->kmer);

  char tmp_seq[kmer_size+1];
 
  if (src_o == reverse){
    binary_kmer_assignment_operator(src_k, *(binary_kmer_reverse_complement(&src_k,kmer_size, &tmp_kmer)));
  }
    
  if (tgt_o == reverse){
    binary_kmer_assignment_operator(tgt_k, *(binary_kmer_reverse_complement(&tgt_k,kmer_size, &tmp_kmer)));
  }
    
   db_genotyping_node_add_labeled_edge(src_e,src_o,binary_kmer_get_last_nucleotide(&tgt_k), 
				       edge_type, edge_index);


  db_genotyping_node_add_labeled_edge(tgt_e,opposite_orientation(tgt_o),binary_kmer_get_last_nucleotide(binary_kmer_reverse_complement(&src_k,kmer_size, &tmp_kmer)), edge_type, edge_index );

  return true;
}



boolean db_genotyping_node_edge_exist(GenotypingElement * element,Nucleotide base,Orientation orientation, EdgeArrayType edge_type, int edge_index){

  //get the edge char for this specific person or pop:
  char edge = genotyping_node_get_edge_copy(*element, edge_type, edge_index);


  edge >>= base;
  if (orientation == reverse){
    edge >>= 4;
  }
  
  edge &= 1;
  
  if (edge == 1){
    return true;
  }
  else{
    return false;
  }
}


//final argument f is a function that returns an Edge that is a function of the different colured edges in a node.
// eg we might be interested in the union of all the coloured edges in the graph, or just the colour/edge for the first person,
//    or the union of all edges except that corresponding to the reference.
boolean db_genotyping_node_edge_exist_within_specified_function_of_coloured_edges(GenotypingElement * element,Nucleotide base,Orientation orientation, Edges (*f)( const GenotypingElement* ))
{

  char edge = f(element);


  edge >>= base;
  if (orientation == reverse){
    edge >>= 4;
  }
  
  edge &= 1;
  
  if (edge == 1){
    return true;
  }
  else{
    return false;
  }
  
}






void db_genotyping_node_reset_edges(GenotypingElement * node,EdgeArrayType edge_type, int edge_index){
  genotyping_node_set_edges(node, edge_type, edge_index, 0);
}

void db_genotyping_node_reset_edge(GenotypingElement * node, Orientation orientation, Nucleotide nucleotide, EdgeArrayType edge_type, int edge_index){
  genotyping_node_reset_one_edge(node, orientation, nucleotide, edge_type, edge_index);
}

//pass in a function which will apply reset_one_edge to whichever of the edges it cares about
void db_genotyping_node_reset_specified_edges(GenotypingElement * node, Orientation orientation, Nucleotide nucleotide,  
				   void (*f)(GenotypingElement*, Orientation, Nucleotide )){
  f(node, orientation, nucleotide);
}




boolean db_genotyping_node_edges_reset(GenotypingElement * node, EdgeArrayType edge_type, int edge_index){
  return genotyping_node_get_edge_copy(*node,edge_type,edge_index) == 0;
}


boolean db_genotyping_node_has_precisely_one_edge(GenotypingElement * node, Orientation orientation, Nucleotide * nucleotide, EdgeArrayType edge_type, int edge_index){
  
  Nucleotide n;
  Edges edges = genotyping_node_get_edge_copy(*node,edge_type,edge_index);
  short edges_count = 0;

  if (orientation == reverse){
    edges >>= 4;
  }
 
  
  for(n=0;n<4;n++){
    
    if ((edges & 1) == 1){
      *nucleotide = n;
      edges_count++;
    }
    
    edges >>= 1;    
  }
  
  return (edges_count == 1);
  
}



boolean db_genotyping_node_has_precisely_one_edge_in_subgraph_defined_by_func_of_colours(GenotypingElement * node, Orientation orientation, Nucleotide * nucleotide, 
									      Edges (*get_colour)(const GenotypingElement*) )
{
  
  Nucleotide n;
  Edges edges = get_colour(node);
  short edges_count = 0;

  if (orientation == reverse){
    edges >>= 4;
  }
 
  
  for(n=0;n<4;n++){
    
    if ((edges & 1) == 1){
      *nucleotide = n;
      edges_count++;
    }
    
    edges >>= 1;    
  }
  
  return (edges_count == 1);
  
}


boolean db_genotyping_node_has_precisely_one_edge_in_union_graph_over_all_people(GenotypingElement * node, Orientation orientation, Nucleotide * nucleotide){
  
  Nucleotide n;
  Edges edges = genotyping_node_get_union_of_edges(*node);
  short edges_count = 0;

  if (orientation == reverse){
    edges >>= 4;
  }
 
  
  for(n=0;n<4;n++){
    
    if ((edges & 1) == 1){
      *nucleotide = n;
      edges_count++;
    }
    
    edges >>= 1;    
  }
  
  return (edges_count == 1);
  
}

//a conflict - bifurcation
boolean db_genotyping_node_has_precisely_two_edges(GenotypingElement * node, Orientation orientation, Nucleotide * nucleotide1, Nucleotide * nucleotide2, EdgeArrayType type, int index){
  
  Nucleotide n;

  Edges edges = genotyping_node_get_edge_copy(*node,type,index);

  short edges_count = 0;

  if (orientation == reverse){
    edges >>= 4;
  }
  
  for(n=0;n<4;n++){
    
    if ((edges & 1) == 1){
      if (edges_count == 0){
	*nucleotide1 = n;
      }
      
      if (edges_count == 1){
	*nucleotide2 = n;
      }
      edges_count++;
    }
    
    edges >>= 1;    
  }
  
  return (edges_count == 2);
  
}



boolean db_genotyping_node_is_blunt_end(GenotypingElement * node, Orientation orientation, EdgeArrayType edge_type, int edge_index){
  
  Edges edges = genotyping_node_get_edge_copy(*node, edge_type, edge_index);


  if (orientation == reverse){
    edges >>= 4;
  }
  
  edges &= 15; // AND with 00001111 so that we only look at the 4 least significant bits
  
  return edges == 0;
}



boolean db_genotyping_node_is_blunt_end_in_subgraph_given_by_func_of_colours(GenotypingElement * node, Orientation orientation,  Edges (*get_colour)(const GenotypingElement*) ){
  
  Edges edges = get_colour(node);


  if (orientation == reverse){
    edges >>= 4;
  }
  
  edges &= 15; // AND with 00001111 so that we only look at the 4 least significant bits
  
  return edges == 0;
}







boolean db_genotyping_node_check_status(GenotypingElement * node, NodeStatus status){
  return node->status == (char) status;
}

boolean db_genotyping_node_check_status_to_be_dumped(GenotypingElement * node){
  if ( db_genotyping_node_check_status(node, to_be_dumped)==true )
    {
      return true;
    }
  else
    {
      return false;
    }
}

boolean db_genotyping_node_check_status_not_pruned(GenotypingElement * node){
  if ( db_genotyping_node_check_status(node, none) || db_genotyping_node_check_status(node,visited))
    {
      return true;
    }
  return false;
}


boolean db_genotyping_node_check_status_not_pruned_or_visited(GenotypingElement * node)
{
  if ( db_genotyping_node_check_status(node,visited) || db_genotyping_node_check_status(node,visited_and_exists_in_reference) ||  db_genotyping_node_check_status(node, pruned)	 ) 
    {
      return false;
    }
  else
    {
      return true;
    }
}


void db_genotyping_node_set_status(GenotypingElement * node,NodeStatus status){
  if (node!=NULL)
    {
      node->status = (char) status;
    }
}

void db_genotyping_node_set_status_to_none(GenotypingElement * node){
  if (node!=NULL)
    {
      node->status = none;
    }
}




boolean db_genotyping_node_is_this_node_in_this_person_or_populations_graph(GenotypingElement* node, EdgeArrayType type, int index)
{

  if (node ==NULL)
    {
      return false;
    }

  if (db_genotyping_node_check_status(node, pruned)==true)
    {
      return false;
    }

  Edges edge_for_this_person_or_pop = genotyping_node_get_edge_copy(*node, type, index);

  if (edge_for_this_person_or_pop == 0)
    {
      return false;
    }
  else
    {
      return true;
    }
 
}

boolean db_genotyping_node_is_this_node_in_subgraph_defined_by_func_of_colours(GenotypingElement* node, Edges (*get_colour)(const GenotypingElement*) )
{

  if (node ==NULL)
    {
      return false;
    }
  if (db_genotyping_node_check_status(node, pruned)==true)
    {
      return false;
    }

  //get_colour will return an edge which is some function of all the edges/colours at that node. eg, will AND all the edges, to consider the union of all colours, etc
  Edges edge = get_colour(node);

  if (edge == 0)
    {
      return false;
    }
  else
    {
      return true;
    }
 
}





void db_genotyping_node_action_set_status_none(GenotypingElement * node){
  db_genotyping_node_set_status(node,none);
}

void db_genotyping_node_action_set_status_of_unpruned_to_none(GenotypingElement * node){

  if (db_genotyping_node_check_status(node, pruned)==false)
    {
      db_genotyping_node_set_status(node,none);
    }
}



void db_genotyping_node_action_set_status_pruned(GenotypingElement * node){
  db_genotyping_node_set_status(node,pruned);
}


void db_genotyping_node_action_set_status_visited(GenotypingElement * node){
  db_genotyping_node_set_status(node,visited);
}

void db_genotyping_node_action_set_status_ignore_this_node(GenotypingElement * node)
{
  db_genotyping_node_set_status(node,ignore_this_node);
}

void db_genotyping_node_action_set_status_visited_or_visited_and_exists_in_reference(GenotypingElement * node){

  if (db_genotyping_node_check_status(node, exists_in_reference))
    {
      db_genotyping_node_set_status(node,visited_and_exists_in_reference);      
    }
  else if (!db_genotyping_node_check_status(node, unassigned)) 
    {
      db_genotyping_node_set_status(node,visited);
    }

}


void db_genotyping_node_action_unset_status_visited_or_visited_and_exists_in_reference(GenotypingElement * node){
  if (db_genotyping_node_check_status_visited_and_exists_in_reference(node))
  {
    db_genotyping_node_set_status(node,exists_in_reference);
  }
  else if (db_genotyping_node_check_status(node, visited))
  {
    db_genotyping_node_set_status(node, none);
  }
      
}

void db_genotyping_node_action_unset_status_visited_or_visited_and_exists_in_reference_or_ignore_this_node(GenotypingElement * node){
  if (db_genotyping_node_check_status_visited_and_exists_in_reference(node))
  {
    db_genotyping_node_set_status(node,exists_in_reference);
  }
  else if (db_genotyping_node_check_status(node, visited))
  {
    db_genotyping_node_set_status(node, none);
  }
  else if (db_genotyping_node_check_status(node, ignore_this_node))
  {
    db_genotyping_node_set_status(node, none);
  }
      
}




void db_genotyping_node_action_do_nothing(GenotypingElement * node){
  
}


boolean db_genotyping_node_check_status_none(GenotypingElement * node){
  return db_genotyping_node_check_status(node,none);
}


//wrapper - in future this will be part of the flags introduced by Mario et al
boolean db_genotyping_node_check_for_flag_ALL_OFF(GenotypingElement * node) {

  if (db_genotyping_node_check_status(node, unassigned)==true)
    {
      return true;
    }
  else
    {
      return false;
    }
}


boolean db_genotyping_node_check_status_visited(GenotypingElement * node){
  return db_genotyping_node_check_status(node,visited);
}

boolean db_genotyping_node_check_status_exists_in_reference(GenotypingElement * node){
  return db_genotyping_node_check_status(node,exists_in_reference);
}

boolean db_genotyping_node_check_status_visited_and_exists_in_reference(GenotypingElement * node){
  return db_genotyping_node_check_status(node,visited_and_exists_in_reference);
}

boolean db_genotyping_node_check_status_is_not_exists_in_reference(GenotypingElement * node){

  if (db_genotyping_node_check_status(node,exists_in_reference)==true )
    {
      return false;
    }
  else
    {
      return true;
    }
}

boolean db_genotyping_node_check_status_is_not_visited(GenotypingElement* node)
{
   if (db_genotyping_node_check_status(node,visited)  )
    {
      return false;
    }
  else
    {
      return true;
    }
  
}

boolean db_genotyping_node_check_status_is_not_visited_or_visited_and_exists_in_reference(GenotypingElement * node){
  if (db_genotyping_node_check_status(node,visited_and_exists_in_reference) || db_genotyping_node_check_status(node,visited)  )
    {
      return false;
    }
  else
    {
      return true;
    }
}


boolean db_genotyping_node_condition_always_true(GenotypingElement* node)
{
  return true;
}

