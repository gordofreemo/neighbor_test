/*BHEADER**********************************************************************
 * (c) 1998   The Regents of the University of California
 *
 * See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
 * notice, contact person, and disclaimer.
 *
 * $Revision$
 *********************************************************************EHEADER*/
/******************************************************************************
 *
 *****************************************************************************/

#include "headers.h"

/*==========================================================================*/
/*==========================================================================*/
/**
  Selects a coarse "grid" based on the graph of a matrix.

  Notes:
  \begin{itemize}
  \item The underlying matrix storage scheme is a hypre_CSR matrix.
  \item The routine returns the following:
  \begin{itemize}
  \item S - a CSR matrix representing the "strength matrix".  This is
  used in the "build interpolation" routine.
  \item CF\_marker - an array indicating both C-pts (value = 1) and
  F-pts (value = -1)
  \end{itemize}
  \item We define the following temporary storage:
  \begin{itemize}
  \item measure\_array - an array containing the "measures" for each
  of the fine-grid points
  \item graph\_array - an array containing the list of points in the
  "current subgraph" being considered in the coarsening process.
  \end{itemize}
  \item The graph of the "strength matrix" for A is a subgraph of the
  graph of A, but requires nonsymmetric storage even if A is
  symmetric.  This is because of the directional nature of the
  "strengh of dependence" notion (see below).  Since we are using
  nonsymmetric storage for A right now, this is not a problem.  If we
  ever add the ability to store A symmetrically, then we could store
  the strength graph as floats instead of doubles to save space.
  \item This routine currently "compresses" the strength matrix.  We
  should consider the possibility of defining this matrix to have the
  same "nonzero structure" as A.  To do this, we could use the same
  A\_i and A\_j arrays, and would need only define the S\_data array.
  There are several pros and cons to discuss.
  \end{itemize}

  Terminology:
  \begin{itemize}
  \item Ruge's terminology: A point is "strongly connected to" $j$, or
  "strongly depends on" $j$, if $-a_ij >= \theta max_{l != j} \{-a_il\}$.
  \item Here, we retain some of this terminology, but with a more
  generalized notion of "strength".  We also retain the "natural"
  graph notation for representing the directed graph of a matrix.
  That is, the nonzero entry $a_ij$ is represented as: i --> j.  In
  the strength matrix, S, the entry $s_ij$ is also graphically denoted
  as above, and means both of the following:
  \begin{itemize}
  \item $i$ "depends on" $j$ with "strength" $s_ij$
  \item $j$ "influences" $i$ with "strength" $s_ij$
  \end{itemize}
  \end{itemize}

  {\bf Input files:}
  headers.h

  @return Error code.
  
  @param A [IN]
  coefficient matrix
  @param strength_threshold [IN]
  threshold parameter used to define strength
  @param S_ptr [OUT]
  strength matrix
  @param CF_marker_ptr [OUT]
  array indicating C/F points
  @param coarse_size_ptr [OUT]
  size of the coarse grid
  
  @see */
/*--------------------------------------------------------------------------*/

#define C_PT  1
#define F_PT -1
#define COMMON_C_PT  2

#define CPOINT 1
#define FPOINT -1
#define UNDECIDED 0




/**************************************************************
 *
 *      Coarsening routine
 *
 **************************************************************/
int
hypre_AMGCoarsenCR( hypre_CSRMatrix    *A,
                  double              strength_threshold,
                  double	      relax_weight,
                  int		      relax_type,
                  int		      num_relax_steps,
                  int               **CF_marker_ptr,
                  int                *coarse_size_ptr     )
{
   int             *A_i           = hypre_CSRMatrixI(A);
   int             *A_j           = hypre_CSRMatrixJ(A);
   double          *A_data        = hypre_CSRMatrixData(A);
   int              num_variables = hypre_CSRMatrixNumRows(A);
                  
   int             *CF_marker;
   int              coarse_size;

   double          *measure_array;
   hypre_Vector    *measure_vector;
   hypre_Vector    *zero_vector;
   hypre_Vector    *tmp_vector;
   int             *graph_array;
   int             *tmp_array;
   int              graph_size;
   int              tmp_size;

   double           diag, row_scale;
   int              i, j, k, jA, jS, kS, ig;

   int              ierr = 0;

#if 0 /* debugging */
   char  filename[256];
   FILE *fp;
   int   iter = 0;
#endif

   /*---------------------------------------------------
    * Initialize the C/F marker array
    *---------------------------------------------------*/

   CF_marker = hypre_CTAlloc(int, num_variables);

   measure_vector = hypre_VectorCreate(num_variables);
   zero_vector = hypre_VectorCreate(num_variables);
   tmp_vector = hypre_VectorCreate(num_variables);
   hypre_VectorInitialize(measure_vector);
   hypre_VectorInitialize(zero_vector);
   hypre_VectorInitialize(tmp_vector);
   hypre_VectorSetConstantValues(measure_vector, 1.0);
   measure_array = hypre_VectorData(measure_vector);

   for (i=0; i < num_relax_steps; i++)
   {
      hypre_AMGRelax (A, zero_vector, CF_marker, relax_type, 0, relax_weight, 
			measure_vector, tmp_vector);
   }

   /*---------------------------------------------------
    * Initialize the graph array
    *---------------------------------------------------*/

   graph_array   = hypre_CTAlloc(int, num_variables);
   coarse_size = 0;
   graph_size = 0;
 
   /* intialize measure array and graph array */

   for (i = 0; i < num_variables; i++)
   {
      if (measure_array[i] < strength_threshold)
      {
	 measure_array[i] = 0;
	 CF_marker[i] = -1;
      }
      else
 	 graph_array[graph_size++] = i;
   }

/*  for second path of coarse point determination if needed */
/*   tmp_array   = hypre_CTAlloc(int, graph_size);  */

   while (graph_size)
   {

      hypre_InitAMGIndepSet(A, measure_array, 0.001);
      hypre_AMGIndepSet (A, measure_array, 0.001,
			graph_array, graph_size, CF_marker);

/*  for second path of coarse point determination if needed */
/*      tmp_size = 0;
      for (ig = 0; ig < graph_size; ig++)
      {
         i = graph_array[ig];
         if (CF_marker[i] == 1)
         {
            tmp_array[tmp_size++] = i;
         }
      }

      hypre_AMGIndepSet (A, measure_array, 0.001,
			tmp_array, tmp_size, CF_marker);
*/

      hypre_VectorSetConstantValues(measure_vector, 0.0);

      for (i = 0; i < num_variables; i++)
      {
         if (CF_marker[i] == 0) 
         {
   	    CF_marker[i] = -2; 
   	    measure_array[i] = 1.0;
         }
      }

      for (i=0; i < num_relax_steps; i++)
      {
         hypre_AMGRelax (A, zero_vector, CF_marker, relax_type, -2, 
			relax_weight, measure_vector, tmp_vector);
      }

      for (ig = 0; ig < graph_size; ig++)
      {
         i = graph_array[ig];
         if (CF_marker[i] == 1)
         {
	    measure_array[i] = 0;
 	    graph_size--;
 	    coarse_size++;
	    graph_array[ig] = graph_array[graph_size];
	    ig--;
         }
         else if (measure_array[i] < strength_threshold)
         {
	    measure_array[i] = 0;
	    CF_marker[i] = -1;
 	    graph_size--;
	    graph_array[ig] = graph_array[graph_size];
   	    ig--;
         }
      }
   }

   /*---------------------------------------------------
    * Clean up and return
    *---------------------------------------------------*/

   hypre_VectorDestroy(measure_vector);
   hypre_VectorDestroy(zero_vector);
   hypre_VectorDestroy(tmp_vector);
   hypre_TFree(graph_array);
/*    hypre_TFree(tmp_array); */

   *CF_marker_ptr   = CF_marker;
   *coarse_size_ptr = coarse_size;

   return (ierr);
}
