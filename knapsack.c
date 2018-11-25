/*!***************************************************
	 Mark Sattolo (mhsatto@po.cse)
	-----------------------------------------------
	  File: @(#)knapsack.c
	  Version: 1.23
	  Last Update: 03/03/20 10:59:55

***************************************************!*/
/*
  Best-First Search with Branch-and-Bound Pruning Algorithm
  for the 0-1 Knapsack Problem

  see p.235 of "Foundations of Algorithms: with C++ pseudocode", 2nd Ed. 1998,
					by Richard Neapolitan & Kumarss Naimipour, Jones & Bartlett,
					ISBN 0-7637-0620-5

  Problem: Let n items be given, where each item has a weight and a profit.
		The weights and profits are positive integers.  Furthermore, let a positive
		integer W be given.  Determine a set of items with maximum total profit,
		under the constraint that the sum of their weights cannot exceed W.

  Inputs: positive integers n and W,
	  arrays of positive integers w[] and p[],
		 ( each indexed from 1 to n, and each of which is sorted in non-increasing
			order according to the values of p[i]/w[i] ).

  Outputs: an integer maxprofit that is the sum of the profits of an optimal set.
		?[an array giving the indices of the items comprising the optimal set]
*/

#include "node.h"

#define KNAPSACK_MAIN_DEBUG 1
#define KNAPSACK_BFS_DEBUG 1

// FUNCTION PROTOTYPES
void bound( node*, const nodeArray, int, int );
int bestFirstSearch( const nodeArray, int, int, int*, char** );


/***** MAIN ****************************************************************!*/

int main( int argc, char *argv[] )
{
  int i=0 ,	// loop index
		j ,	// check each input line
		n ,	// total items
		W ,	// maximum allowed weight of items (user-supplied)

		totweight=0 , // of items selected
		maxProfit ;

  char *bestitems = NULL ;
  char temp[ KNAP_MAX_NAME_LEN ] = {0};

  FILE *inputfile ;
  nodeArray pw ;

  // check command line parameters
  if( argc < 2 )
  {
	 printf( "\nUsage: %s 'file name' [max weight] \n\n", argv[0] );
	 exit( 0 );
  }

  if( argc < 3 )
  {
	 printf( "\nPlease enter the maximum weight: " );
	 scanf( "%u", &W );
  }
  else
		W = atoi( argv[2] );

  printf( "\nfilename is %s \n", argv[1] );
  printf( "W == %u \n", W );
#if KNAPSACK_MAIN_DEBUG > 0
  printf( "sizeof(node) == %u \n", sizeof(node) );
#endif

  // open the file
  inputfile = fopen( argv[1], "r" );
  if( !inputfile )
  {
	 fprintf( stderr, "Error occurred opening file '%s' !\n", argv[1] );
	 exit( 1 );
  }

  // first entry in the file should be the # of items
  if( fscanf(inputfile, "%u", &n) != 1 )
  {
	 fprintf( stderr, "Error getting # of items (%d) in file '%s' !\n",
							n, argv[1] );
	 exit( 2 );
  }

  printf( "\nThere should be %d items in file '%s' \n", n, argv[1] );

  // allocate the array
  if( !initNodeArray(&pw, n) )
	 exit( 3 );

  // scan in the input lines
  while( !feof(inputfile) )
  {
	 // get the data
	 j = fscanf( inputfile, "%s %u %u", temp, &(pw[i].profit), &(pw[i].weight) );

#if KNAPSACK_MAIN_DEBUG > 1
	 printf( "\n i == %d \n", i );
	 printf( "temp == %s ; strlen(temp) == %d \n", temp, strlen(temp) );
#endif

	 if( j == 3 ) // got a complete line
	 {
		setName( &pw[i], temp );

		pw[i].level = 0 ;
      pw[i].bound = 0.0 ;

		// calculate the p/w ratio
		pw[i].pw
		  = (pw[i].weight > 0) ? ((float)pw[i].profit / (float)pw[i].weight) : (float)0.0 ;

#if KNAPSACK_MAIN_DEBUG > 1
		displayNode( pw+i );
#endif

		getc( inputfile ) ; // eat the EOL
		i++ ; // index in the loop through the array

	 }// if( j == 3 )

  }// while( !feof(inputfile) )

#if KNAPSACK_MAIN_DEBUG > 0
	 printf( "\nThere were %d items in file '%s' \n", i, argv[1] );
#endif

  fclose( inputfile );

#if KNAPSACK_MAIN_DEBUG > 1
  puts( "\nBEFORE SORTING:" );
  displayNodeArray( pw, n );
#endif

  // sort the pw array
  qsort( pw, n, sizeof(node), compareNode );

  puts( "\nAFTER SORTING:" );
  displayNodeArray( pw, n );

  // run the algorithm and display the results
  maxProfit = bestFirstSearch( pw, n, W, &totweight, &bestitems );
  printf( "\nFor Weight limit %d: Max Profit == %d (actual weight == %d)\n",
		W, maxProfit, totweight );
  printf( "Best items are: %s \n", bestitems ? bestitems : "NOT AVAILABLE !" );

  free( bestitems );
  deleteNodeArray( pw, n );

  printf( "\n PROGRAM ENDED.\n" );

  return 0 ;

}// main()


///// FUNCTIONS ////////////////////////////////////////////////////////////////

//
void bound( node *x, const nodeArray pw, int n, int W )
{
  int j ;
  int totweight ;
  float result = 0.0 ;

#if KNAPSACK_BOUND_DEBUG > 1
  puts( "\nINSIDE bound():" );
  printf( " n == %d \n", n );
  displayNodeArray( pw, n );
#endif

#if KNAPSACK_BOUND_DEBUG > 0
  printf( "\n bound(1): " );
  displayNode( x );
#endif

  // calculate the new bound if the weight is under the limit
  if( x->weight < W )
  {
	 result = (float)x->profit ;

	 j = x->level + 1 ;
#if KNAPSACK_BOUND_DEBUG > 0
	 printf( " bound(2): j == %d \n", j );
#endif

	 totweight = x->weight ;

	 // grab as many items as possible
	 while( j < n  &&  (totweight + pw[j].weight <= W) )
	 {
		totweight += pw[j].weight ;
		result += (float)pw[j].profit ;
		j++ ;
#if KNAPSACK_BOUND_DEBUG > 0
		printf( " bound(3): result == %7.3f \n", result );
		printf( " bound(4): j == %d \n", j );
#endif
	 }

	 if( j < n )
		// grab fraction of jth item
		result += ( ((float)(W - totweight)) * pw[j].pw );

#if KNAPSACK_BOUND_DEBUG > 0
	 printf( " bound(5): node %s has bound == %7.3f \n", x->name, result );
#endif
  }

  x->bound = result ;

}//! bound()


//
int bestFirstSearch( const nodeArray pw, int n, int W, int *tw, char **best )
{
  int i = 0 ; // loop count

  node u, v ; // working nodes

  const char include[] = "&" ;
  const char exclude[] = "-" ;

  PriorityQueue PQ ;
  int maxprofit = 0 ;

  u.name = v.name = NULL ; // initialize the char*'s

  initQueue( &PQ );

  // set the names of u and v to keep track of items properly
  setName( &u, exclude );
  setName( &v, "root"  );

  v.level = -1 ; // start at -1 so the root node gets index == 0 in bound()
  v.pw = u.pw = 0.0 ;
  v.profit = v.weight = 0 ;

  // get the initial bound
  bound( &v, pw, n, W );

#if KNAPSACK_BFS_DEBUG > 0
  puts( "" );
  displayNode( &v );
#endif

  insertNode( &PQ, &v ); // start the state space tree with the root node

#if KNAPSACK_BFS_DEBUG > 0
  displayQueue( &PQ );
  printf( "START WHILE LOOP... \n\n" );
#endif

  while( !isEmptyQueue(&PQ) )// &&  i < limit ) // limit prevents a runaway loop
  {
#if KNAPSACK_BFS_DEBUG > 1
	 printf( "\nPQ.nodes == %p \n", PQ.nodes );
	 printf( "PQ.size == %d \n", PQ.size );
#endif
#if KNAPSACK_BFS_DEBUG > 2
	 displayQueue( &PQ );
#endif

	 removeNode( &PQ, &v ); // remove node with best bound
#if KNAPSACK_BFS_DEBUG > 0
	 printf( "\nBFS( v ): " );
	 displayNode( &v );
#endif

	 if( v.bound > maxprofit ) // check if node is still promising
	 {
#if KNAPSACK_BFS_DEBUG > 0
		printf( "v.bound == %7.3f \n", v.bound );
#endif

		// SET u TO THE CHILD THAT INCLUDES THE NEXT ITEM
		u.level = v.level + 1 ;

		// keep track of all items in this node
		setName( &u, v.name );
		appendName( &u, include );
		appendName( &u, pw[u.level].name );

		u.weight = v.weight + pw[u.level].weight ;
		u.profit = v.profit + pw[u.level].profit ;

#if KNAPSACK_BFS_DEBUG > 0
		printf( "\nBFS( u ): " );
		displayNode( &u );
#endif

		if( u.weight <=  W  &&  u.profit > maxprofit )
		{
		  maxprofit = u.profit ;
	*tw = u.weight ;
	printf( "\nBFS(%d): maxprofit now == %d \n", i, maxprofit );
	printf( "\t current best items are %s \n", u.name );
	printf( "\t current weight of items is %d \n", *tw );

		  // keep track of overall list of best items
		  *best = (char*)realloc( *best, (strlen(u.name)+1) * sizeof(char) );
		  if( *best )
			 strcpy( *best, u.name );
		  else
				fprintf( stderr, "Memory allocation error for *best !\n" );
		}

		bound( &u, pw, n, W );
		if( u.bound > maxprofit )
		  insertNode( &PQ, &u );


		// SET u TO THE CHILD THAT DOES NOT INCLUDE THE NEXT ITEM

		// keep track of all items in this node
		setName( &u, v.name );
		appendName( &u, exclude ); // alter the name here just to monitor backtracking

		// we already incremented the level in the previous section
		u.weight = v.weight ;
		u.profit = v.profit ;

		bound( &u, pw, n, W );
		if( u.bound > maxprofit ) // if this node is still promising
		  insertNode( &PQ, &u );
	 }

	 i++ ;
#if KNAPSACK_BFS_DEBUG > 0
	 printf( "\n i == %d \n\n", i );
#endif

  }// while( !isEmptyQueue(&PQ)  &&  i < capacity )

#if KNAPSACK_BFS_DEBUG == 0
  printf( "\n Final i == %d \n", i );
#endif

  free( u.name );
  free( v.name );
  deleteQueue( &PQ );

  return maxprofit ;

}// bestFirstSearch()
