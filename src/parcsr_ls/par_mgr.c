/******************************************************************************
 * Copyright (c) 1998 Lawrence Livermore National Security, LLC and other
 * HYPRE Project Developers. See the top-level COPYRIGHT file for details.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR MIT)
 ******************************************************************************/

/******************************************************************************
 *
 * Two-grid system solver
 *
 *****************************************************************************/

#include "_hypre_parcsr_ls.h"
#include "par_amg.h"
#include "par_mgr.h"
#include "_hypre_blas.h"
#include "_hypre_lapack.h"

//#ifdef HYPRE_USING_DSUPERLU
//#include "dsuperlu.h"
//#endif

#if defined(HYPRE_USING_GPU)
void hypre_NoGPUSupport(char *option)
{
   char msg[256];
   hypre_sprintf(msg, "Error: Chosen %s option is not currently supported on GPU\n\n", option);
   hypre_printf("%s ", msg);
   //  hypre_error_w_msg(1, msg);
   hypre_MPI_Abort(hypre_MPI_COMM_WORLD, -1);
}
#endif

/* Need to define these hypre_lapack protos here instead of including _hypre_lapack.h to avoid conflicts with
 * dsuperlu.h on some lapack functions. Alternative is to move superLU related functions to a separate file.
*/
/* dgetrf.c */
//HYPRE_Int hypre_dgetrf ( HYPRE_Int *m, HYPRE_Int *n, HYPRE_Real *a, HYPRE_Int *lda, HYPRE_Int *ipiv,
//                         HYPRE_Int *info );
/* dgetri.c */
//HYPRE_Int hypre_dgetri ( HYPRE_Int *n, HYPRE_Real *a, HYPRE_Int *lda, HYPRE_Int *ipiv,
//                         HYPRE_Real *work, HYPRE_Int *lwork, HYPRE_Int *info);

/* Create */
void *
hypre_MGRCreate(void)
{
   hypre_ParMGRData  *mgr_data;

   mgr_data = hypre_CTAlloc(hypre_ParMGRData,  1, HYPRE_MEMORY_HOST);

   /* block data */
   (mgr_data -> block_size) = 1;
   (mgr_data -> block_num_coarse_indexes) = NULL;
   (mgr_data -> point_marker_array) = NULL;
   (mgr_data -> block_cf_marker) = NULL;

   /* general data */
   (mgr_data -> max_num_coarse_levels) = 10;
   (mgr_data -> A_array) = NULL;
#if defined(HYPRE_USING_GPU)
   (mgr_data -> P_FF_array) = NULL;
#endif
   (mgr_data -> P_array) = NULL;
   (mgr_data -> RT_array) = NULL;
   (mgr_data -> RAP) = NULL;
   (mgr_data -> CF_marker_array) = NULL;
   (mgr_data -> coarse_indices_lvls) = NULL;

   (mgr_data -> A_ff_array) = NULL;
   (mgr_data -> F_fine_array) = NULL;
   (mgr_data -> U_fine_array) = NULL;
   (mgr_data -> aff_solver) = NULL;
   (mgr_data -> fine_grid_solver_setup) = NULL;
   (mgr_data -> fine_grid_solver_solve) = NULL;

   (mgr_data -> F_array) = NULL;
   (mgr_data -> U_array) = NULL;
   (mgr_data -> residual) = NULL;
   (mgr_data -> rel_res_norms) = NULL;
   (mgr_data -> Vtemp) = NULL;
   (mgr_data -> Ztemp) = NULL;
   (mgr_data -> Utemp) = NULL;
   (mgr_data -> Ftemp) = NULL;

   (mgr_data -> num_iterations) = 0;
   (mgr_data -> num_interp_sweeps) = 1;
   (mgr_data -> num_restrict_sweeps) = 1;
   (mgr_data -> trunc_factor) = 0.0;
   (mgr_data -> max_row_sum) = 0.9;
   (mgr_data -> strong_threshold) = 0.25;
   (mgr_data -> P_max_elmts) = 0;

   (mgr_data -> coarse_grid_solver) = NULL;
   (mgr_data -> coarse_grid_solver_setup) = NULL;
   (mgr_data -> coarse_grid_solver_solve) = NULL;

   //(mgr_data -> global_smoother) = NULL;

   (mgr_data -> use_default_cgrid_solver) = 1;
   (mgr_data -> fsolver_mode) = -1; // user or hypre -prescribed F-solver
   (mgr_data -> omega) = 1.;
   (mgr_data -> max_iter) = 20;
   (mgr_data -> tol) = 1.0e-6;
   (mgr_data -> relax_type) = 0;
   (mgr_data -> Frelax_type) = NULL;
   (mgr_data -> relax_order) = 1; // not fully utilized. Only used to compute L1-norms.
   (mgr_data -> num_relax_sweeps) = NULL;
   (mgr_data -> relax_weight) = 1.0;

   (mgr_data -> interp_type) = NULL;
   (mgr_data -> restrict_type) = NULL;
   (mgr_data -> level_smooth_iters) = NULL;
   (mgr_data -> level_smooth_type) = NULL;
   (mgr_data -> level_smoother) = NULL;
   (mgr_data -> global_smooth_cycle) = 1; // Pre = 1 or Post  = 2 global smoothing

   (mgr_data -> logging) = 0;
   (mgr_data -> print_level) = 0;
   (mgr_data -> frelax_print_level) = 0;
   (mgr_data -> cg_print_level) = 0;

   (mgr_data -> l1_norms) = NULL;

   (mgr_data -> reserved_coarse_size) = 0;
   (mgr_data -> reserved_coarse_indexes) = NULL;
   (mgr_data -> reserved_Cpoint_local_indexes) = NULL;

   (mgr_data -> level_diaginv) = NULL;
   (mgr_data -> frelax_diaginv) = NULL;
   //(mgr_data -> global_smooth_iters) = 1;
   //(mgr_data -> global_smooth_type) = 0;

   (mgr_data -> set_non_Cpoints_to_F) = 0;
   (mgr_data -> idx_array) = NULL;

   (mgr_data -> Frelax_method) = NULL;
   (mgr_data -> VcycleRelaxVtemp) = NULL;
   (mgr_data -> VcycleRelaxZtemp) = NULL;
   (mgr_data -> FrelaxVcycleData) = NULL;
   (mgr_data -> Frelax_num_functions) = NULL;
   (mgr_data -> max_local_lvls) = 10;

   (mgr_data -> mgr_coarse_grid_method) = NULL;

   (mgr_data -> print_coarse_system) = 0;

   (mgr_data -> set_c_points_method) = 0;
   (mgr_data -> lvl_to_keep_cpoints) = 0;
   (mgr_data -> cg_convergence_factor) = 0.0;

   (mgr_data -> block_jacobi_bsize) = 0;
   (mgr_data -> blk_size) = NULL;

   (mgr_data -> truncate_coarse_grid_threshold) = 0.0;

   (mgr_data -> GSElimData) = NULL;

   return (void *) mgr_data;
}

/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
/* Destroy */
HYPRE_Int
hypre_MGRDestroy( void *data )
{
   hypre_ParMGRData * mgr_data = (hypre_ParMGRData*) data;

   HYPRE_Int i;
   HYPRE_Int num_coarse_levels = (mgr_data -> num_coarse_levels);

   /* block info data */
   if ((mgr_data -> block_cf_marker))
   {
      for (i = 0; i < (mgr_data -> max_num_coarse_levels); i++)
      {
         hypre_TFree((mgr_data -> block_cf_marker)[i], HYPRE_MEMORY_HOST);
      }
      hypre_TFree((mgr_data -> block_cf_marker), HYPRE_MEMORY_HOST);
   }

   hypre_TFree(mgr_data -> block_num_coarse_indexes, HYPRE_MEMORY_HOST);

   /* final residual vector */
   if ((mgr_data -> residual))
   {
      hypre_ParVectorDestroy( (mgr_data -> residual) );
      (mgr_data -> residual) = NULL;
   }

   hypre_TFree( (mgr_data -> rel_res_norms), HYPRE_MEMORY_HOST);

   /* temp vectors for solve phase */
   if ((mgr_data -> Vtemp))
   {
      hypre_ParVectorDestroy( (mgr_data -> Vtemp) );
      (mgr_data -> Vtemp) = NULL;
   }
   if ((mgr_data -> Ztemp))
   {
      hypre_ParVectorDestroy( (mgr_data -> Ztemp) );
      (mgr_data -> Ztemp) = NULL;
   }
   if ((mgr_data -> Utemp))
   {
      hypre_ParVectorDestroy( (mgr_data -> Utemp) );
      (mgr_data -> Utemp) = NULL;
   }
   if ((mgr_data -> Ftemp))
   {
      hypre_ParVectorDestroy( (mgr_data -> Ftemp) );
      (mgr_data -> Ftemp) = NULL;
   }
   /* coarse grid solver */
   if ((mgr_data -> use_default_cgrid_solver))
   {
      if ((mgr_data -> coarse_grid_solver))
      {
         hypre_BoomerAMGDestroy( (mgr_data -> coarse_grid_solver) );
      }
      (mgr_data -> coarse_grid_solver) = NULL;
   }
   /* l1_norms */
   if ((mgr_data -> l1_norms))
   {
      for (i = 0; i < (num_coarse_levels); i++)
      {
         hypre_SeqVectorDestroy((mgr_data -> l1_norms)[i]);
      }
      hypre_TFree((mgr_data -> l1_norms), HYPRE_MEMORY_HOST);
   }

   /* coarse_indices_lvls */
   if ((mgr_data -> coarse_indices_lvls))
   {
      for (i = 0; i < (num_coarse_levels); i++)
      {
         hypre_TFree((mgr_data -> coarse_indices_lvls)[i], HYPRE_MEMORY_HOST);
      }
      hypre_TFree((mgr_data -> coarse_indices_lvls), HYPRE_MEMORY_HOST);
   }

   /* linear system and cf marker array */
   if (mgr_data -> A_array || mgr_data -> P_array || mgr_data -> RT_array ||
       mgr_data -> CF_marker_array)
   {
      for (i = 1; i < num_coarse_levels + 1; i++)
      {
         hypre_ParVectorDestroy((mgr_data -> F_array)[i]);
         hypre_ParVectorDestroy((mgr_data -> U_array)[i]);

         if ((mgr_data -> P_array)[i - 1])
         {
            hypre_ParCSRMatrixDestroy((mgr_data -> P_array)[i - 1]);
         }

         if ((mgr_data -> RT_array)[i - 1])
         {
            hypre_ParCSRMatrixDestroy((mgr_data -> RT_array)[i - 1]);
         }

         hypre_IntArrayDestroy(mgr_data -> CF_marker_array[i - 1]);
      }
      for (i = 1; i < (num_coarse_levels); i++)
      {
         if ((mgr_data -> A_array)[i])
         {
            hypre_ParCSRMatrixDestroy((mgr_data -> A_array)[i]);
         }
      }
   }

#if defined(HYPRE_USING_GPU)
   if (mgr_data -> P_FF_array)
   {
      for (i = 0; i < num_coarse_levels; i++)
      {
         if ((mgr_data -> P_array)[i])
         {
            hypre_ParCSRMatrixDestroy((mgr_data -> P_FF_array)[i]);
         }
      }
      //hypre_TFree(P_FF_array, hypre_HandleMemoryLocation(hypre_handle()));
      hypre_TFree((mgr_data -> P_FF_array), HYPRE_MEMORY_HOST);
      (mgr_data -> P_FF_array) = NULL;
   }
#endif

   /* AMG for Frelax */
   if (mgr_data -> A_ff_array || mgr_data -> F_fine_array || mgr_data -> U_fine_array)
   {
      for (i = 1; i < num_coarse_levels + 1; i++)
      {
         if (mgr_data -> F_fine_array[i])
         {
            hypre_ParVectorDestroy((mgr_data -> F_fine_array)[i]);
         }
         if (mgr_data -> U_fine_array[i])
         {
            hypre_ParVectorDestroy((mgr_data -> U_fine_array)[i]);
         }
      }
      for (i = 1; i < (num_coarse_levels); i++)
      {
         if ((mgr_data -> A_ff_array)[i])
         {
            hypre_ParCSRMatrixDestroy((mgr_data -> A_ff_array)[i]);
         }
      }
      if (mgr_data -> fsolver_mode != 0)
      {
         if ((mgr_data -> A_ff_array)[0])
         {
            hypre_ParCSRMatrixDestroy((mgr_data -> A_ff_array)[0]);
         }
      }
      hypre_TFree(mgr_data -> F_fine_array, HYPRE_MEMORY_HOST);
      (mgr_data -> F_fine_array) = NULL;
      hypre_TFree(mgr_data -> U_fine_array, HYPRE_MEMORY_HOST);
      (mgr_data -> U_fine_array) = NULL;
      hypre_TFree(mgr_data -> A_ff_array, HYPRE_MEMORY_HOST);
      (mgr_data -> A_ff_array) = NULL;
   }

   if (mgr_data -> aff_solver)
   {
      for (i = 1; i < (num_coarse_levels); i++)
      {
         if ((mgr_data -> aff_solver)[i])
         {
            hypre_BoomerAMGDestroy((mgr_data -> aff_solver)[i]);
         }
      }
      if (mgr_data -> fsolver_mode == 2)
      {
         if ((mgr_data -> aff_solver)[0])
         {
            hypre_BoomerAMGDestroy((mgr_data -> aff_solver)[0]);
         }
      }
      hypre_TFree(mgr_data -> aff_solver, HYPRE_MEMORY_HOST);
      (mgr_data -> aff_solver) = NULL;
   }

   if (mgr_data -> level_diaginv)
   {
      for (i = 0; i < (num_coarse_levels); i++)
      {
         hypre_TFree((mgr_data -> level_diaginv)[i], HYPRE_MEMORY_HOST);
      }
      hypre_TFree(mgr_data -> level_diaginv, HYPRE_MEMORY_HOST);
   }

   if (mgr_data -> frelax_diaginv)
   {
      for (i = 0; i < (num_coarse_levels); i++)
      {
         hypre_TFree((mgr_data -> frelax_diaginv)[i], HYPRE_MEMORY_HOST);
      }
      hypre_TFree(mgr_data -> frelax_diaginv, HYPRE_MEMORY_HOST);
   }
   hypre_TFree((mgr_data -> F_array), HYPRE_MEMORY_HOST);
   hypre_TFree((mgr_data -> U_array), HYPRE_MEMORY_HOST);
   hypre_TFree((mgr_data -> A_array), HYPRE_MEMORY_HOST);
   hypre_TFree((mgr_data -> P_array), HYPRE_MEMORY_HOST);
   hypre_TFree((mgr_data -> RT_array), HYPRE_MEMORY_HOST);
   hypre_TFree((mgr_data -> CF_marker_array), HYPRE_MEMORY_HOST);
   hypre_TFree((mgr_data -> reserved_Cpoint_local_indexes), HYPRE_MEMORY_HOST);
   hypre_TFree(mgr_data -> restrict_type, HYPRE_MEMORY_HOST);
   hypre_TFree(mgr_data -> interp_type, HYPRE_MEMORY_HOST);
   /* Frelax_type */
   hypre_TFree(mgr_data -> Frelax_type, HYPRE_MEMORY_HOST);
   /* Frelax_method */
   hypre_TFree(mgr_data -> Frelax_method, HYPRE_MEMORY_HOST);
   /* Frelax_num_functions */
   hypre_TFree(mgr_data -> Frelax_num_functions, HYPRE_MEMORY_HOST);

   /* data for V-cycle F-relaxation */
   if ((mgr_data -> VcycleRelaxVtemp))
   {
      hypre_ParVectorDestroy( (mgr_data -> VcycleRelaxVtemp) );
      (mgr_data -> VcycleRelaxVtemp) = NULL;
   }
   if ((mgr_data -> VcycleRelaxZtemp))
   {
      hypre_ParVectorDestroy( (mgr_data -> VcycleRelaxZtemp) );
      (mgr_data -> VcycleRelaxZtemp) = NULL;
   }
   if (mgr_data -> FrelaxVcycleData)
   {
      for (i = 0; i < num_coarse_levels; i++)
      {
         hypre_MGRDestroyFrelaxVcycleData((mgr_data -> FrelaxVcycleData)[i]);
      }
      hypre_TFree(mgr_data -> FrelaxVcycleData, HYPRE_MEMORY_HOST);
   }
   /* data for reserved coarse nodes */
   hypre_TFree(mgr_data -> reserved_coarse_indexes, HYPRE_MEMORY_HOST);
   /* index array for setting Cpoints by global block */
   if ((mgr_data -> set_c_points_method) == 1)
   {
      hypre_TFree(mgr_data -> idx_array, HYPRE_MEMORY_HOST);
   }
   /* array for setting option to use non-Galerkin coarse grid */
   hypre_TFree(mgr_data -> mgr_coarse_grid_method, HYPRE_MEMORY_HOST);
   /* coarse level matrix - RAP */
   if ((mgr_data -> RAP))
   {
      hypre_ParCSRMatrixDestroy((mgr_data -> RAP));
   }

   if ((mgr_data -> level_smoother) != NULL)
   {
      for (i = 0; i < num_coarse_levels; i++)
      {
         if ((mgr_data -> level_smooth_iters)[i] > 0)
         {
            if ((mgr_data -> level_smooth_type)[i] == 8)
            {
               HYPRE_EuclidDestroy((mgr_data -> level_smoother)[i]);
            }
            else if ((mgr_data -> level_smooth_type)[i] == 16)
            {
               HYPRE_ILUDestroy((mgr_data -> level_smoother)[i]);
            }
         }
      }
      hypre_TFree(mgr_data -> level_smoother, HYPRE_MEMORY_HOST);
   }

   /* free level data */
   hypre_TFree(mgr_data -> blk_size, HYPRE_MEMORY_HOST);
   hypre_TFree(mgr_data -> level_smooth_type, HYPRE_MEMORY_HOST);
   hypre_TFree(mgr_data -> level_smooth_iters, HYPRE_MEMORY_HOST);
   hypre_TFree(mgr_data -> num_relax_sweeps, HYPRE_MEMORY_HOST);

   if (mgr_data -> GSElimData)
   {
      for (i = 0; i < num_coarse_levels; i++)
      {
         if ((mgr_data -> GSElimData)[i])
         {
            hypre_MGRDestroyGSElimData((mgr_data -> GSElimData)[i]);
            (mgr_data -> GSElimData)[i] = NULL;
         }
      }
      hypre_TFree(mgr_data -> GSElimData, HYPRE_MEMORY_HOST);
   }

   /* mgr data */
   hypre_TFree(mgr_data, HYPRE_MEMORY_HOST);

   return hypre_error_flag;
}

/* create data for Gaussian Elim. for F-relaxation */
void *
hypre_MGRCreateGSElimData( void )
{
   hypre_ParAMGData  *gsdata = hypre_CTAlloc(hypre_ParAMGData,  1, HYPRE_MEMORY_HOST);

   hypre_ParAMGDataGSSetup(gsdata) = 0;
   hypre_ParAMGDataAMat(gsdata) = NULL;
   hypre_ParAMGDataAInv(gsdata) = NULL;
   hypre_ParAMGDataBVec(gsdata) = NULL;
   hypre_ParAMGDataCommInfo(gsdata) = NULL;
   hypre_ParAMGDataNewComm(gsdata) = hypre_MPI_COMM_NULL;

   return (void *) gsdata;
}

/* Destroy data for Gaussian Elim. for F-relaxation */
HYPRE_Int
hypre_MGRDestroyGSElimData( void *data )
{
   hypre_ParAMGData * gsdata = (hypre_ParAMGData*) data;
   MPI_Comm new_comm = hypre_ParAMGDataNewComm(gsdata);

   if (hypre_ParAMGDataAMat(gsdata)) { hypre_TFree(hypre_ParAMGDataAMat(gsdata), HYPRE_MEMORY_HOST); }
   if (hypre_ParAMGDataAInv(gsdata)) { hypre_TFree(hypre_ParAMGDataAInv(gsdata), HYPRE_MEMORY_HOST); }
   if (hypre_ParAMGDataBVec(gsdata)) { hypre_TFree(hypre_ParAMGDataBVec(gsdata), HYPRE_MEMORY_HOST); }
   if (hypre_ParAMGDataCommInfo(gsdata)) { hypre_TFree(hypre_ParAMGDataCommInfo(gsdata), HYPRE_MEMORY_HOST); }

   if (new_comm != hypre_MPI_COMM_NULL)
   {
      hypre_MPI_Comm_free (&new_comm);
   }
   
   hypre_TFree(gsdata, HYPRE_MEMORY_HOST);
   return hypre_error_flag;
}

/* Create data for V-cycle F-relaxtion */
void *
hypre_MGRCreateFrelaxVcycleData( void )
{
   hypre_ParAMGData  *vdata = hypre_CTAlloc(hypre_ParAMGData,  1, HYPRE_MEMORY_HOST);

   hypre_ParAMGDataAArray(vdata) = NULL;
   hypre_ParAMGDataPArray(vdata) = NULL;
   hypre_ParAMGDataFArray(vdata) = NULL;
   hypre_ParAMGDataCFMarkerArray(vdata) = NULL;
   hypre_ParAMGDataVtemp(vdata)  = NULL;
   //   hypre_ParAMGDataAMat(vdata)  = NULL;
   //   hypre_ParAMGDataBVec(vdata)  = NULL;
   hypre_ParAMGDataZtemp(vdata)  = NULL;
   //   hypre_ParAMGDataCommInfo(vdata) = NULL;
   hypre_ParAMGDataUArray(vdata) = NULL;
   hypre_ParAMGDataNewComm(vdata) = hypre_MPI_COMM_NULL;
   hypre_ParAMGDataNumLevels(vdata) = 0;
   hypre_ParAMGDataMaxLevels(vdata) = 10;
   hypre_ParAMGDataNumFunctions(vdata) = 1;
   hypre_ParAMGDataSCommPkgSwitch(vdata) = 1.0;
   hypre_ParAMGDataRelaxOrder(vdata) = 1;
   hypre_ParAMGDataMaxCoarseSize(vdata) = 9;
   hypre_ParAMGDataMinCoarseSize(vdata) = 0;
   hypre_ParAMGDataUserCoarseRelaxType(vdata) = 9;

   /* Gaussian Elim data */
   hypre_ParAMGDataGSSetup(vdata) = 0;
   hypre_ParAMGDataAMat(vdata) = NULL;
   hypre_ParAMGDataAInv(vdata) = NULL;
   hypre_ParAMGDataBVec(vdata) = NULL;
   hypre_ParAMGDataCommInfo(vdata) = NULL;

   return (void *) vdata;
}

/* Destroy data for V-cycle F-relaxation */
HYPRE_Int
hypre_MGRDestroyFrelaxVcycleData( void *data )
{
   hypre_ParAMGData * vdata = (hypre_ParAMGData*) data;
   HYPRE_Int i;
   HYPRE_Int num_levels = hypre_ParAMGDataNumLevels(vdata);
   MPI_Comm new_comm = hypre_ParAMGDataNewComm(vdata);

   hypre_TFree(hypre_ParAMGDataDofFuncArray(vdata)[0], HYPRE_MEMORY_HOST);
   for (i = 1; i < num_levels + 1; i++)
   {
      if (hypre_ParAMGDataAArray(vdata)[i])
      {
         hypre_ParCSRMatrixDestroy(hypre_ParAMGDataAArray(vdata)[i]);
      }

      if (hypre_ParAMGDataPArray(vdata)[i - 1])
      {
         hypre_ParCSRMatrixDestroy(hypre_ParAMGDataPArray(vdata)[i - 1]);
      }

      hypre_IntArrayDestroy(hypre_ParAMGDataCFMarkerArray(vdata)[i - 1]);
      hypre_ParVectorDestroy(hypre_ParAMGDataFArray(vdata)[i]);
      hypre_ParVectorDestroy(hypre_ParAMGDataUArray(vdata)[i]);
      hypre_TFree(hypre_ParAMGDataDofFuncArray(vdata)[i], HYPRE_MEMORY_HOST);
   }

   if (num_levels < 1)
   {
      hypre_IntArrayDestroy(hypre_ParAMGDataCFMarkerArray(vdata)[0]);
   }

   /* Points to VcycleRelaxVtemp of mgr_data, which is already destroyed */
   //hypre_ParVectorDestroy(hypre_ParAMGDataVtemp(vdata));
   hypre_TFree(hypre_ParAMGDataFArray(vdata), HYPRE_MEMORY_HOST);
   hypre_TFree(hypre_ParAMGDataUArray(vdata), HYPRE_MEMORY_HOST);
   hypre_TFree(hypre_ParAMGDataAArray(vdata), HYPRE_MEMORY_HOST);
   hypre_TFree(hypre_ParAMGDataPArray(vdata), HYPRE_MEMORY_HOST);
   hypre_TFree(hypre_ParAMGDataCFMarkerArray(vdata), HYPRE_MEMORY_HOST);
   //hypre_TFree(hypre_ParAMGDataGridRelaxType(vdata), HYPRE_MEMORY_HOST);
   hypre_TFree(hypre_ParAMGDataDofFuncArray(vdata), HYPRE_MEMORY_HOST);

   /* Points to VcycleRelaxZtemp of mgr_data, which is already destroyed */
   /*
     if (hypre_ParAMGDataZtemp(vdata))
         hypre_ParVectorDestroy(hypre_ParAMGDataZtemp(vdata));
   */

   if (hypre_ParAMGDataAMat(vdata)) { hypre_TFree(hypre_ParAMGDataAMat(vdata), HYPRE_MEMORY_HOST); }
   if (hypre_ParAMGDataAInv(vdata)) { hypre_TFree(hypre_ParAMGDataAInv(vdata), HYPRE_MEMORY_HOST); }
   if (hypre_ParAMGDataBVec(vdata)) { hypre_TFree(hypre_ParAMGDataBVec(vdata), HYPRE_MEMORY_HOST); }
   if (hypre_ParAMGDataCommInfo(vdata)) { hypre_TFree(hypre_ParAMGDataCommInfo(vdata), HYPRE_MEMORY_HOST); }

   if (new_comm != hypre_MPI_COMM_NULL)
   {
      hypre_MPI_Comm_free (&new_comm);
   }
   hypre_TFree(vdata, HYPRE_MEMORY_HOST);

   return hypre_error_flag;
}

/* Set C-point variables for each reduction level */
/* Currently not implemented */
HYPRE_Int
hypre_MGRSetReductionLevelCpoints( void      *mgr_vdata,
                                   HYPRE_Int  nlevels,
                                   HYPRE_Int *num_coarse_points,
                                   HYPRE_Int  **level_coarse_indexes)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> num_coarse_levels) = nlevels;
   (mgr_data -> num_coarse_per_level) = num_coarse_points;
   (mgr_data -> level_coarse_indexes) = level_coarse_indexes;
   return hypre_error_flag;
}

/* Initialize some data */
/* Set whether non-coarse points on each level should be explicitly tagged as F-points */
HYPRE_Int
hypre_MGRSetNonCpointsToFpoints( void      *mgr_vdata, HYPRE_Int nonCptToFptFlag)
{
   hypre_ParMGRData *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> set_non_Cpoints_to_F) = nonCptToFptFlag;

   return hypre_error_flag;
}

/* Set whether the reserved C points are reduced before the coarse grid solve */
HYPRE_Int
hypre_MGRSetReservedCpointsLevelToKeep(void *mgr_vdata, HYPRE_Int level)
{
   hypre_ParMGRData *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> lvl_to_keep_cpoints) = level;

   return hypre_error_flag;
}

/* Set Cpoints by contiguous blocks, i.e. p1, p2, ..., pn, s1, s2, ..., sn, ... */
HYPRE_Int
hypre_MGRSetCpointsByContiguousBlock( void  *mgr_vdata,
                                      HYPRE_Int  block_size,
                                      HYPRE_Int  max_num_levels,
                                      HYPRE_BigInt  *begin_idx_array,
                                      HYPRE_Int  *block_num_coarse_points,
                                      HYPRE_Int  **block_coarse_indexes)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   if ((mgr_data -> idx_array) != NULL)
   {
      hypre_TFree(mgr_data -> idx_array, HYPRE_MEMORY_HOST);
      (mgr_data -> idx_array) = NULL;
   }
   HYPRE_BigInt *index_array = hypre_CTAlloc(HYPRE_BigInt, block_size, HYPRE_MEMORY_HOST);
   if (begin_idx_array != NULL)
   {
      for (i = 0; i < block_size; i++)
      {
         index_array[i] = *(begin_idx_array + i);
      }
   }
   hypre_MGRSetCpointsByBlock(mgr_data, block_size, max_num_levels, block_num_coarse_points,
                              block_coarse_indexes);
   (mgr_data -> idx_array) = index_array;
   (mgr_data -> set_c_points_method) = 1;
   return hypre_error_flag;
}

/* Initialize/ set local block data information */
HYPRE_Int
hypre_MGRSetCpointsByBlock( void      *mgr_vdata,
                            HYPRE_Int  block_size,
                            HYPRE_Int  max_num_levels,
                            HYPRE_Int  *block_num_coarse_points,
                            HYPRE_Int  **block_coarse_indexes)
{
   HYPRE_Int  i, j;
   HYPRE_Int  **block_cf_marker = NULL;
   HYPRE_Int *block_num_coarse_indexes = NULL;

   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   /* free block cf_marker data if not previously destroyed */
   if ((mgr_data -> block_cf_marker) != NULL)
   {
      for (i = 0; i < (mgr_data -> max_num_coarse_levels); i++)
      {
         if ((mgr_data -> block_cf_marker)[i])
         {
            hypre_TFree((mgr_data -> block_cf_marker)[i], HYPRE_MEMORY_HOST);
            (mgr_data -> block_cf_marker)[i] = NULL;
         }
      }
      hypre_TFree(mgr_data -> block_cf_marker, HYPRE_MEMORY_HOST);
      (mgr_data -> block_cf_marker) = NULL;
   }
   if ((mgr_data -> block_num_coarse_indexes))
   {
      hypre_TFree((mgr_data -> block_num_coarse_indexes), HYPRE_MEMORY_HOST);
      (mgr_data -> block_num_coarse_indexes) = NULL;
   }

   /* store block cf_marker */
   block_cf_marker = hypre_CTAlloc(HYPRE_Int *, max_num_levels, HYPRE_MEMORY_HOST);
   for (i = 0; i < max_num_levels; i++)
   {
      block_cf_marker[i] = hypre_CTAlloc(HYPRE_Int, block_size, HYPRE_MEMORY_HOST);
      memset(block_cf_marker[i], FMRK, block_size * sizeof(HYPRE_Int));
   }
   for (i = 0; i < max_num_levels; i++)
   {
      for (j = 0; j < block_num_coarse_points[i]; j++)
      {
         (block_cf_marker[i])[block_coarse_indexes[i][j]] = CMRK;
      }
   }

   /* store block_num_coarse_points */
   if (max_num_levels > 0)
   {
      block_num_coarse_indexes = hypre_CTAlloc(HYPRE_Int,  max_num_levels, HYPRE_MEMORY_HOST);
      for (i = 0; i < max_num_levels; i++)
      {
         block_num_coarse_indexes[i] = block_num_coarse_points[i];
      }
   }
   /* set block data */
   (mgr_data -> max_num_coarse_levels) = max_num_levels;
   (mgr_data -> block_size) = block_size;
   (mgr_data -> block_num_coarse_indexes) = block_num_coarse_indexes;
   (mgr_data -> block_cf_marker) = block_cf_marker;
   (mgr_data -> set_c_points_method) = 0;

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRSetCpointsByPointMarkerArray( void      *mgr_vdata,
                                       HYPRE_Int  block_size,
                                       HYPRE_Int  max_num_levels,
                                       HYPRE_Int  *lvl_num_coarse_points,
                                       HYPRE_Int  **lvl_coarse_indexes,
                                       HYPRE_Int  *point_marker_array)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int  i, j;
   HYPRE_Int  **block_cf_marker = NULL;
   HYPRE_Int *block_num_coarse_indexes = NULL;

   /* free block cf_marker data if not previously destroyed */
   if ((mgr_data -> block_cf_marker) != NULL)
   {
      for (i = 0; i < (mgr_data -> max_num_coarse_levels); i++)
      {
         if ((mgr_data -> block_cf_marker)[i])
         {
            hypre_TFree((mgr_data -> block_cf_marker)[i], HYPRE_MEMORY_HOST);
            (mgr_data -> block_cf_marker)[i] = NULL;
         }
      }
      hypre_TFree(mgr_data -> block_cf_marker, HYPRE_MEMORY_HOST);
      (mgr_data -> block_cf_marker) = NULL;
   }
   if ((mgr_data -> block_num_coarse_indexes))
   {
      hypre_TFree((mgr_data -> block_num_coarse_indexes), HYPRE_MEMORY_HOST);
      (mgr_data -> block_num_coarse_indexes) = NULL;
   }

   /* store block cf_marker */
   block_cf_marker = hypre_CTAlloc(HYPRE_Int *, max_num_levels, HYPRE_MEMORY_HOST);
   for (i = 0; i < max_num_levels; i++)
   {
      block_cf_marker[i] = hypre_CTAlloc(HYPRE_Int, block_size, HYPRE_MEMORY_HOST);
      memset(block_cf_marker[i], FMRK, block_size * sizeof(HYPRE_Int));
   }
   for (i = 0; i < max_num_levels; i++)
   {
      for (j = 0; j < lvl_num_coarse_points[i]; j++)
      {
         block_cf_marker[i][j] = lvl_coarse_indexes[i][j];
      }
   }

   /* store block_num_coarse_points */
   if (max_num_levels > 0)
   {
      block_num_coarse_indexes = hypre_CTAlloc(HYPRE_Int,  max_num_levels, HYPRE_MEMORY_HOST);
      for (i = 0; i < max_num_levels; i++)
      {
         block_num_coarse_indexes[i] = lvl_num_coarse_points[i];
      }
   }
   /* set block data */
   (mgr_data -> max_num_coarse_levels) = max_num_levels;
   (mgr_data -> block_size) = block_size;
   (mgr_data -> block_num_coarse_indexes) = block_num_coarse_indexes;
   (mgr_data -> block_cf_marker) = block_cf_marker;
   (mgr_data -> point_marker_array) = point_marker_array;
   (mgr_data -> set_c_points_method) = 2;

   return hypre_error_flag;
}

/*Set number of points that remain part of the coarse grid throughout the hierarchy */
HYPRE_Int
hypre_MGRSetReservedCoarseNodes(void      *mgr_vdata,
                                HYPRE_Int reserved_coarse_size,
                                HYPRE_BigInt *reserved_cpt_index)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_BigInt *reserved_coarse_indexes = NULL;
   HYPRE_Int i;

   if (!mgr_data)
   {
      hypre_error_w_msg(HYPRE_ERROR_GENERIC, "Warning! MGR object empty!\n");
      return hypre_error_flag;
   }

   if (reserved_coarse_size < 0)
   {
      hypre_error_in_arg(2);
      return hypre_error_flag;
   }
   /* free data not previously destroyed */
   if ((mgr_data -> reserved_coarse_indexes))
   {
      hypre_TFree((mgr_data -> reserved_coarse_indexes), HYPRE_MEMORY_HOST);
      (mgr_data -> reserved_coarse_indexes) = NULL;
   }

   /* set reserved coarse nodes */
   if (reserved_coarse_size > 0)
   {
      reserved_coarse_indexes = hypre_CTAlloc(HYPRE_BigInt,  reserved_coarse_size, HYPRE_MEMORY_HOST);
      for (i = 0; i < reserved_coarse_size; i++)
      {
         reserved_coarse_indexes[i] = reserved_cpt_index[i];
      }
   }
   (mgr_data -> reserved_coarse_size) = reserved_coarse_size;
   (mgr_data -> reserved_coarse_indexes) = reserved_coarse_indexes;

   return hypre_error_flag;
}

/* Set CF marker array */
HYPRE_Int
hypre_MGRCoarsen(hypre_ParCSRMatrix *S,
                 hypre_ParCSRMatrix *A,
                 HYPRE_Int fixed_coarse_size,
                 HYPRE_Int *fixed_coarse_indexes,
                 HYPRE_Int debug_flag,
                 hypre_IntArray **CF_marker_ptr,
                 HYPRE_Int cflag)
{
   HYPRE_Int   *CF_marker = NULL;
   HYPRE_Int *cindexes = fixed_coarse_indexes;
   HYPRE_Int    i, row, nc;
   HYPRE_Int nloc =  hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));

   /* If this is the last level, coarsen onto fixed coarse set */
   if (cflag)
   {
      if (*CF_marker_ptr != NULL)
      {
         hypre_IntArrayDestroy(*CF_marker_ptr);
      }
      *CF_marker_ptr = hypre_IntArrayCreate(nloc);
      hypre_IntArrayInitialize(*CF_marker_ptr);
      hypre_IntArraySetConstantValues(*CF_marker_ptr, FMRK);
      CF_marker = hypre_IntArrayData(*CF_marker_ptr);

      /* first mark fixed coarse set */
      nc = fixed_coarse_size;
      for (i = 0; i < nc; i++)
      {
         CF_marker[cindexes[i]] = CMRK;
      }
   }
   else
   {
      /* First coarsen to get initial CF splitting.
       * This is then followed by updating the CF marker to pass
       * coarse information to the next levels. NOTE: It may be
       * convenient to implement this way (allows the use of multiple
       * coarsening strategies without changing too much code),
       * but not necessarily the best option, compared to initializing
       * CF_marker first and then coarsening on subgraph which excludes
       * the initialized coarse nodes.
      */
      hypre_BoomerAMGCoarsen(S, A, 0, debug_flag, CF_marker_ptr);
      CF_marker = hypre_IntArrayData(*CF_marker_ptr);

      /* Update CF_marker to correct Cpoints marked as Fpoints. */
      nc = fixed_coarse_size;
      for (i = 0; i < nc; i++)
      {
         CF_marker[cindexes[i]] = CMRK;
      }
      /* set F-points to FMRK. This is necessary since the different coarsening schemes differentiate
       * between type of F-points (example Ruge coarsening). We do not need that distinction here.
      */
      for (row = 0; row < nloc; row++)
      {
         if (CF_marker[row] == CMRK) { continue; }
         CF_marker[row] = FMRK;
      }
#if 0
      /* IMPORTANT: Update coarse_indexes array to define the positions of the fixed coarse points
       * in the next level.
       */
      nc = 0;
      index_i = 0;
      for (row = 0; row < nloc; row++)
      {
         /* loop through new c-points */
         if (CF_marker[row] == CMRK) { nc++; }
         else if (CF_marker[row] == S_CMRK)
         {
            /* previously marked c-point is part of fixed coarse set. Track its current local index */
            cindexes[index_i++] = nc;
            /* reset c-point from S_CMRK to CMRK */
            cf_marker[row] = CMRK;
            nc++;
         }
         /* set F-points to FMRK. This is necessary since the different coarsening schemes differentiate
          * between type of F-points (example Ruge coarsening). We do not need that distinction here.
          */
         else
         {
            CF_marker[row] = FMRK;
         }
      }
      /* check if this should be last level */
      if ( nc == fixed_coarse_size)
      {
         last_level = 1;
      }
      //printf(" nc = %d and fixed coarse size = %d \n", nc, fixed_coarse_size);
#endif
   }

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRBuildPFromWp( hypre_ParCSRMatrix   *A,
                       hypre_ParCSRMatrix   *Wp,
                       HYPRE_Int            *CF_marker,
                       HYPRE_Int            debug_flag,
                       hypre_ParCSRMatrix   **P_ptr)
{
   MPI_Comm          comm = hypre_ParCSRMatrixComm(A);
   HYPRE_MemoryLocation memory_location_P = hypre_ParCSRMatrixMemoryLocation(A);

   hypre_ParCSRMatrix    *P;

   hypre_CSRMatrix *P_diag = NULL;
   hypre_CSRMatrix *P_offd = NULL;
   hypre_CSRMatrix *Wp_diag, *Wp_offd;

   HYPRE_Real      *P_diag_data, *Wp_diag_data;
   HYPRE_Int       *P_diag_i, *Wp_diag_i;
   HYPRE_Int       *P_diag_j, *Wp_diag_j;
   HYPRE_Real      *P_offd_data, *Wp_offd_data;
   HYPRE_Int       *P_offd_i, *Wp_offd_i;
   HYPRE_Int       *P_offd_j, *Wp_offd_j;

   HYPRE_Int        P_num_rows, P_diag_size, P_offd_size;

   HYPRE_Int        jj_counter, jj_counter_offd;

   HYPRE_Int        start_indexing = 0; /* start indexing for P_data at 0 */

   HYPRE_Int        i, jj;
   HYPRE_Int        row_Wp, coarse_counter;

   HYPRE_Real       one  = 1.0;

   HYPRE_Int        my_id;
   HYPRE_Int        num_procs;

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);
   //num_threads = hypre_NumThreads();
   // Temporary fix, disable threading
   // TODO: enable threading
   P_num_rows = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));

   Wp_diag = hypre_ParCSRMatrixDiag(Wp);
   Wp_offd = hypre_ParCSRMatrixOffd(Wp);
   Wp_diag_i = hypre_CSRMatrixI(Wp_diag);
   Wp_diag_j = hypre_CSRMatrixJ(Wp_diag);
   Wp_diag_data = hypre_CSRMatrixData(Wp_diag);
   Wp_offd_i = hypre_CSRMatrixI(Wp_offd);
   Wp_offd_j = hypre_CSRMatrixJ(Wp_offd);
   Wp_offd_data = hypre_CSRMatrixData(Wp_offd);

   /*-----------------------------------------------------------------------
   *  Intialize counters and allocate mapping vector.
   *-----------------------------------------------------------------------*/
   P_diag_size = hypre_CSRMatrixNumNonzeros(Wp_diag) + hypre_CSRMatrixNumCols(Wp_diag);

   P_diag_i    = hypre_CTAlloc(HYPRE_Int,  P_num_rows + 1, memory_location_P);
   P_diag_j    = hypre_CTAlloc(HYPRE_Int,  P_diag_size, memory_location_P);
   P_diag_data = hypre_CTAlloc(HYPRE_Real,  P_diag_size, memory_location_P);
   P_diag_i[P_num_rows] = P_diag_size;

   P_offd_size = hypre_CSRMatrixNumNonzeros(Wp_offd);

   P_offd_i    = hypre_CTAlloc(HYPRE_Int,  P_num_rows + 1, memory_location_P);
   P_offd_j    = hypre_CTAlloc(HYPRE_Int,  P_offd_size, memory_location_P);
   P_offd_data = hypre_CTAlloc(HYPRE_Real,  P_offd_size, memory_location_P);
   P_offd_i[P_num_rows] = P_offd_size;

   /*-----------------------------------------------------------------------
   *  Intialize some stuff.
   *-----------------------------------------------------------------------*/
   jj_counter = start_indexing;
   jj_counter_offd = start_indexing;

   row_Wp = 0;
   coarse_counter = 0;
   for (i = 0; i < P_num_rows; i++)
   {
      /*--------------------------------------------------------------------
      *  If i is a c-point, interpolation is the identity.
      *--------------------------------------------------------------------*/
      if (CF_marker[i] >= 0)
      {
         P_diag_i[i] = jj_counter;
         P_diag_j[jj_counter]    = coarse_counter;
         P_diag_data[jj_counter] = one;
         coarse_counter++;
         jj_counter++;
      }
      /*--------------------------------------------------------------------
      *  If i is an F-point, build interpolation.
      *--------------------------------------------------------------------*/
      else
      {
         /* Diagonal part of P */
         P_diag_i[i] = jj_counter;
         for (jj = Wp_diag_i[row_Wp]; jj < Wp_diag_i[row_Wp + 1]; jj++)
         {
            P_diag_j[jj_counter]    = Wp_diag_j[jj];
            P_diag_data[jj_counter] = - Wp_diag_data[jj];
            jj_counter++;
         }

         /* Off-Diagonal part of P */
         P_offd_i[i] = jj_counter_offd;
         if (num_procs > 1)
         {
            for (jj = Wp_offd_i[row_Wp]; jj < Wp_offd_i[row_Wp + 1]; jj++)
            {
               P_offd_j[jj_counter_offd]    = Wp_offd_j[jj];
               P_offd_data[jj_counter_offd] = - Wp_offd_data[jj];
               jj_counter_offd++;
            }
         }
         row_Wp++;
      }
      P_offd_i[i + 1] = jj_counter_offd;
   }
   P = hypre_ParCSRMatrixCreate(comm,
                                hypre_ParCSRMatrixGlobalNumRows(A),
                                hypre_ParCSRMatrixGlobalNumCols(Wp),
                                hypre_ParCSRMatrixColStarts(A),
                                hypre_ParCSRMatrixColStarts(Wp),
                                hypre_CSRMatrixNumCols(hypre_ParCSRMatrixOffd(Wp)),
                                P_diag_size,
                                P_offd_size);

   P_diag = hypre_ParCSRMatrixDiag(P);
   hypre_CSRMatrixData(P_diag) = P_diag_data;
   hypre_CSRMatrixI(P_diag) = P_diag_i;
   hypre_CSRMatrixJ(P_diag) = P_diag_j;

   P_offd = hypre_ParCSRMatrixOffd(P);
   hypre_CSRMatrixData(P_offd) = P_offd_data;
   hypre_CSRMatrixI(P_offd) = P_offd_i;
   hypre_CSRMatrixJ(P_offd) = P_offd_j;
   //hypre_ParCSRMatrixOwnsRowStarts(P) = 0;
   //hypre_ParCSRMatrixOwnsColStarts(Wp) = 0;
   //hypre_ParCSRMatrixOwnsColStarts(P) = 1;

   hypre_ParCSRMatrixDeviceColMapOffd(P) = hypre_ParCSRMatrixDeviceColMapOffd(Wp);
   hypre_ParCSRMatrixColMapOffd(P)       = hypre_ParCSRMatrixColMapOffd(Wp);
   //hypre_ParCSRMatrixDeviceColMapOffd(Wp) = NULL;
   //hypre_ParCSRMatrixColMapOffd(Wp)       = NULL;

   hypre_ParCSRMatrixNumNonzeros(P)  = hypre_CSRMatrixNumNonzeros(hypre_ParCSRMatrixDiag(P)) +
                                       hypre_CSRMatrixNumNonzeros(hypre_ParCSRMatrixOffd(P));
   hypre_ParCSRMatrixDNumNonzeros(P) = (HYPRE_Real) hypre_ParCSRMatrixNumNonzeros(P);

   hypre_MatvecCommPkgCreate(P);
   *P_ptr = P;

   return hypre_error_flag;
}


HYPRE_Int
hypre_MGRBuildBlockJacobiWp( hypre_ParCSRMatrix   *A,
                             HYPRE_Int            blk_size,
                             HYPRE_Int            *CF_marker,
                             HYPRE_BigInt         *cpts_starts,
                             hypre_ParCSRMatrix   **Wp_ptr)
{
   hypre_ParCSRMatrix *Wp;
   hypre_ParCSRMatrix *A_FF_inv, *A_FC, *A_FF;

   // Extract submatrices and build A_FF_inv
   hypre_ParCSRMatrixGenerateFFFC(A, CF_marker, cpts_starts, NULL, &A_FC, &A_FF);
   hypre_ParCSRMatrixBlockDiagMatrix(A_FF, blk_size, -1, NULL, &A_FF_inv, 1);

   // Compute Wp = A_FF_inv * A_FC
   Wp = hypre_ParCSRMatMat(A_FF_inv, A_FC);
   *Wp_ptr = Wp;

   hypre_ParCSRMatrixDestroy(A_FF_inv);
   hypre_ParCSRMatrixDestroy(A_FC);
   hypre_ParCSRMatrixDestroy(A_FF);

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRBuildPBlockJacobi( hypre_ParCSRMatrix   *A,
                            hypre_ParCSRMatrix   *Wp,
                            HYPRE_Int            blk_size,
                            HYPRE_Int            *CF_marker,
                            HYPRE_BigInt         *cpts_starts,
                            HYPRE_Int            debug_flag,
                            hypre_ParCSRMatrix   **P_ptr)
{
   MPI_Comm comm = hypre_ParCSRMatrixComm(A);
   HYPRE_Int my_id;

   hypre_MPI_Comm_rank(comm, &my_id);
   if (Wp == NULL)
   {
      HYPRE_Real wall_time = time_getWallclockSeconds();
      hypre_ParCSRMatrix *Wp_tmp;
      hypre_MGRBuildBlockJacobiWp(A, blk_size, CF_marker, cpts_starts, &Wp_tmp);
      hypre_MGRBuildPFromWp(A, Wp_tmp, CF_marker, debug_flag, P_ptr);

      hypre_ParCSRMatrixDeviceColMapOffd(Wp_tmp) = NULL;
      hypre_ParCSRMatrixColMapOffd(Wp_tmp)       = NULL;
      hypre_ParCSRMatrixDestroy(Wp_tmp);
      wall_time = time_getWallclockSeconds() - wall_time;
   }
   else
   {
      hypre_MGRBuildPFromWp(A, Wp, CF_marker, debug_flag, P_ptr);
   }
   return hypre_error_flag;
}

HYPRE_Int
hypre_ExtendWtoPHost(HYPRE_Int      P_nr_of_rows,
                     HYPRE_Int     *CF_marker,
                     HYPRE_Int     *W_diag_i,
                     HYPRE_Int     *W_diag_j,
                     HYPRE_Complex *W_diag_data,
                     HYPRE_Int     *P_diag_i,
                     HYPRE_Int     *P_diag_j,
                     HYPRE_Complex *P_diag_data,
                     HYPRE_Int     *W_offd_i,
                     HYPRE_Int     *P_offd_i )
{
   HYPRE_Int              jj_counter, jj_counter_offd;

   HYPRE_Int              start_indexing = 0; /* start indexing for P_data at 0 */

   HYPRE_Int             *fine_to_coarse = NULL;
   HYPRE_Int              coarse_counter;

   HYPRE_Int              i, jj;

   HYPRE_Real       one  = 1.0;

   /*-----------------------------------------------------------------------
    *  Intialize counters and allocate mapping vector.
    *-----------------------------------------------------------------------*/

   fine_to_coarse = hypre_CTAlloc(HYPRE_Int,  P_nr_of_rows, HYPRE_MEMORY_HOST);

   for (i = 0; i < P_nr_of_rows; i++) { fine_to_coarse[i] = -1; }

   /*-----------------------------------------------------------------------
    *  Loop over fine grid.
    *-----------------------------------------------------------------------*/

   HYPRE_Int row_counter = 0;
   coarse_counter = 0;
   for (i = 0; i < P_nr_of_rows; i++)
   {
      /*--------------------------------------------------------------------
       *  If i is a C-point, interpolation is the identity. Also set up
       *  mapping vector.
       *--------------------------------------------------------------------*/

      if (CF_marker[i] > 0)
      {
         fine_to_coarse[i] = coarse_counter;
         coarse_counter++;
      }
   }

   /*-----------------------------------------------------------------------
    *  Intialize some stuff.
    *-----------------------------------------------------------------------*/

   jj_counter = start_indexing;
   jj_counter_offd = start_indexing;

   row_counter = 0;
   for (i = 0; i < P_nr_of_rows; i++)
   {
      /*--------------------------------------------------------------------
       *  If i is a c-point, interpolation is the identity.
       *--------------------------------------------------------------------*/
      if (CF_marker[i] >= 0)
      {
         P_diag_i[i] = jj_counter;
         P_diag_j[jj_counter]    = fine_to_coarse[i];
         P_diag_data[jj_counter] = one;
         jj_counter++;
      }
      /*--------------------------------------------------------------------
       *  If i is an F-point, build interpolation.
       *--------------------------------------------------------------------*/
      else
      {
         /* Diagonal part of P */
         P_diag_i[i] = jj_counter;
         for (jj = W_diag_i[row_counter]; jj < W_diag_i[row_counter + 1]; jj++)
         {
            //P_marker[row_counter] = jj_counter;
            P_diag_j[jj_counter]    = W_diag_j[jj];
            P_diag_data[jj_counter] = W_diag_data[jj];
            jj_counter++;
         }

         /* Off-Diagonal part of P */
         P_offd_i[i] = jj_counter_offd;
         jj_counter_offd += W_offd_i[row_counter + 1] - W_offd_i[row_counter];

         row_counter++;
      }
      /* update off-diagonal row pointer */
      P_offd_i[i + 1] = jj_counter_offd;
   }
   P_diag_i[P_nr_of_rows] = jj_counter;

   hypre_TFree(fine_to_coarse, HYPRE_MEMORY_HOST);
   return 0;
}

/* Interpolation for MGR - Adapted from BoomerAMGBuildInterp */
HYPRE_Int
hypre_MGRBuildPHost( hypre_ParCSRMatrix   *A,
                     HYPRE_Int            *CF_marker,
                     HYPRE_BigInt         *num_cpts_global,
                     HYPRE_Int             method,
                     hypre_ParCSRMatrix  **P_ptr)
{
   MPI_Comm            comm = hypre_ParCSRMatrixComm(A);
   HYPRE_Int           num_procs, my_id;
   HYPRE_Int           A_nr_of_rows = hypre_ParCSRMatrixNumRows(A);

   hypre_ParCSRMatrix *A_FF = NULL, *A_FC = NULL, *P = NULL;
   hypre_CSRMatrix    *W_diag = NULL, *W_offd = NULL;
   HYPRE_Int           P_diag_nnz, nfpoints;
   HYPRE_Int          *P_diag_i = NULL, *P_diag_j = NULL, *P_offd_i = NULL;
   HYPRE_Complex      *P_diag_data = NULL, *diag = NULL, *diag1 = NULL;
   HYPRE_BigInt        nC_global;
   HYPRE_Int       i;

   HYPRE_MemoryLocation memory_location_P = hypre_ParCSRMatrixMemoryLocation(A);

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);

   nfpoints = 0;
   for (i = 0; i < A_nr_of_rows; i++)
   {
      if (CF_marker[i] == -1)
      {
         nfpoints++;
      }
   }

   if (method > 0)
   {
      hypre_ParCSRMatrixGenerateFFFCHost(A, CF_marker, num_cpts_global, NULL, &A_FC, &A_FF);
      diag = hypre_CTAlloc(HYPRE_Complex, nfpoints, memory_location_P);
      if (method == 1)
      {
         // extract diag inverse sqrt
         //        hypre_CSRMatrixExtractDiagonalHost(hypre_ParCSRMatrixDiag(A_FF), diag, 3);

         // L1-Jacobi-type interpolation
         HYPRE_Complex scal = 1.0;
         diag1 = hypre_CTAlloc(HYPRE_Complex, nfpoints, memory_location_P);
         hypre_CSRMatrixExtractDiagonalHost(hypre_ParCSRMatrixDiag(A_FF), diag, 0);
         hypre_CSRMatrixComputeRowSumHost(hypre_ParCSRMatrixDiag(A_FF), NULL, NULL, diag1, 1, 1.0, "set");
         hypre_CSRMatrixComputeRowSumHost(hypre_ParCSRMatrixDiag(A_FC), NULL, NULL, diag1, 1, 1.0, "add");
         hypre_CSRMatrixComputeRowSumHost(hypre_ParCSRMatrixOffd(A_FF), NULL, NULL, diag1, 1, 1.0, "add");
         hypre_CSRMatrixComputeRowSumHost(hypre_ParCSRMatrixOffd(A_FC), NULL, NULL, diag1, 1, 1.0, "add");

         for (i = 0; i < nfpoints; i++)
         {
            HYPRE_Complex dsum = diag[i] + scal * (diag1[i] - hypre_cabs(diag[i]));
            diag[i] = 1. / dsum;
         }
         hypre_TFree(diag1, memory_location_P);
      }
      else if (method == 2)
      {
         // extract diag inverse
         hypre_CSRMatrixExtractDiagonalHost(hypre_ParCSRMatrixDiag(A_FF), diag, 2);
      }

      for (i = 0; i < nfpoints; i++)
      {
         diag[i] = -diag[i];
      }

      hypre_Vector *D_FF_inv = hypre_SeqVectorCreate(nfpoints);
      hypre_VectorData(D_FF_inv) = diag;
      hypre_SeqVectorInitialize_v2(D_FF_inv, memory_location_P);
      hypre_CSRMatrixDiagScale(hypre_ParCSRMatrixDiag(A_FC), D_FF_inv, NULL);
      hypre_CSRMatrixDiagScale(hypre_ParCSRMatrixOffd(A_FC), D_FF_inv, NULL);
      hypre_SeqVectorDestroy(D_FF_inv);
      W_diag = hypre_ParCSRMatrixDiag(A_FC);
      W_offd = hypre_ParCSRMatrixOffd(A_FC);
      nC_global = hypre_ParCSRMatrixGlobalNumCols(A_FC);
   }
   else
   {
      W_diag = hypre_CSRMatrixCreate(nfpoints, A_nr_of_rows - nfpoints, 0);
      W_offd = hypre_CSRMatrixCreate(nfpoints, 0, 0);
      hypre_CSRMatrixInitialize_v2(W_diag, 0, memory_location_P);
      hypre_CSRMatrixInitialize_v2(W_offd, 0, memory_location_P);

      if (my_id == (num_procs - 1))
      {
         nC_global = num_cpts_global[1];
      }
      hypre_MPI_Bcast(&nC_global, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);
   }

   /* Construct P from matrix product W_diag */
   P_diag_nnz  = hypre_CSRMatrixNumNonzeros(W_diag) + hypre_CSRMatrixNumCols(W_diag);
   P_diag_i    = hypre_CTAlloc(HYPRE_Int,     A_nr_of_rows + 1, memory_location_P);
   P_diag_j    = hypre_CTAlloc(HYPRE_Int,     P_diag_nnz,     memory_location_P);
   P_diag_data = hypre_CTAlloc(HYPRE_Complex, P_diag_nnz,     memory_location_P);
   P_offd_i    = hypre_CTAlloc(HYPRE_Int,     A_nr_of_rows + 1, memory_location_P);

   /* Extend W data to P data */
   hypre_ExtendWtoPHost( A_nr_of_rows,
                         CF_marker,
                         hypre_CSRMatrixI(W_diag),
                         hypre_CSRMatrixJ(W_diag),
                         hypre_CSRMatrixData(W_diag),
                         P_diag_i,
                         P_diag_j,
                         P_diag_data,
                         hypre_CSRMatrixI(W_offd),
                         P_offd_i );

   // finalize P
   P = hypre_ParCSRMatrixCreate(hypre_ParCSRMatrixComm(A),
                                hypre_ParCSRMatrixGlobalNumRows(A),
                                nC_global,
                                hypre_ParCSRMatrixColStarts(A),
                                num_cpts_global,
                                hypre_CSRMatrixNumCols(W_offd),
                                P_diag_nnz,
                                hypre_CSRMatrixNumNonzeros(W_offd) );

   hypre_CSRMatrixMemoryLocation(hypre_ParCSRMatrixDiag(P)) = memory_location_P;
   hypre_CSRMatrixMemoryLocation(hypre_ParCSRMatrixOffd(P)) = memory_location_P;

   hypre_CSRMatrixI(hypre_ParCSRMatrixDiag(P))    = P_diag_i;
   hypre_CSRMatrixJ(hypre_ParCSRMatrixDiag(P))    = P_diag_j;
   hypre_CSRMatrixData(hypre_ParCSRMatrixDiag(P)) = P_diag_data;

   hypre_CSRMatrixI(hypre_ParCSRMatrixOffd(P))    = P_offd_i;
   hypre_CSRMatrixJ(hypre_ParCSRMatrixOffd(P))    = hypre_CSRMatrixJ(W_offd);
   hypre_CSRMatrixData(hypre_ParCSRMatrixOffd(P)) = hypre_CSRMatrixData(W_offd);
   hypre_CSRMatrixJ(W_offd)    = NULL;
   hypre_CSRMatrixData(W_offd) = NULL;

   if (method > 0)
   {
      hypre_ParCSRMatrixColMapOffd(P)    = hypre_ParCSRMatrixColMapOffd(A_FC);
      hypre_ParCSRMatrixColMapOffd(P)          = hypre_ParCSRMatrixColMapOffd(A_FC);
      hypre_ParCSRMatrixColMapOffd(A_FC) = NULL;
      hypre_ParCSRMatrixColMapOffd(A_FC)       = NULL;
      hypre_ParCSRMatrixNumNonzeros(P)         = hypre_ParCSRMatrixNumNonzeros(
                                                    A_FC) + hypre_ParCSRMatrixGlobalNumCols(A_FC);
   }
   else
   {
      hypre_ParCSRMatrixNumNonzeros(P) = nC_global;
   }
   hypre_ParCSRMatrixDNumNonzeros(P) = (HYPRE_Real) hypre_ParCSRMatrixNumNonzeros(P);

   hypre_MatvecCommPkgCreate(P);

   *P_ptr = P;

   if (A_FF)
   {
      hypre_ParCSRMatrixDestroy(A_FF);
   }
   if (A_FC)
   {
      hypre_ParCSRMatrixDestroy(A_FC);
   }

   if (method <= 0)
   {
      hypre_CSRMatrixDestroy(W_diag);
      hypre_CSRMatrixDestroy(W_offd);
   }

   return hypre_error_flag;
}
/* Interpolation for MGR - Adapted from BoomerAMGBuildInterp */
HYPRE_Int
hypre_MGRBuildP( hypre_ParCSRMatrix   *A,
                 HYPRE_Int            *CF_marker,
                 HYPRE_BigInt         *num_cpts_global,
                 HYPRE_Int             method,
                 HYPRE_Int             debug_flag,
                 hypre_ParCSRMatrix  **P_ptr)
{
   MPI_Comm          comm = hypre_ParCSRMatrixComm(A);
   hypre_ParCSRCommPkg     *comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   hypre_ParCSRCommHandle  *comm_handle;
   HYPRE_MemoryLocation memory_location_P = hypre_ParCSRMatrixMemoryLocation(A);

   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real      *A_diag_data = hypre_CSRMatrixData(A_diag);
   HYPRE_Int       *A_diag_i = hypre_CSRMatrixI(A_diag);
   HYPRE_Int       *A_diag_j = hypre_CSRMatrixJ(A_diag);

   hypre_CSRMatrix *A_offd         = hypre_ParCSRMatrixOffd(A);
   HYPRE_Real      *A_offd_data    = hypre_CSRMatrixData(A_offd);
   HYPRE_Int       *A_offd_i = hypre_CSRMatrixI(A_offd);
   HYPRE_Int       *A_offd_j = hypre_CSRMatrixJ(A_offd);
   HYPRE_Int        num_cols_A_offd = hypre_CSRMatrixNumCols(A_offd);
   HYPRE_Real      *a_diag;

   hypre_ParCSRMatrix    *P;
   HYPRE_BigInt    *col_map_offd_P;
   HYPRE_Int       *tmp_map_offd = NULL;

   HYPRE_Int       *CF_marker_offd = NULL;

   hypre_CSRMatrix *P_diag;
   hypre_CSRMatrix *P_offd;

   HYPRE_Real      *P_diag_data;
   HYPRE_Int       *P_diag_i;
   HYPRE_Int       *P_diag_j;
   HYPRE_Real      *P_offd_data;
   HYPRE_Int       *P_offd_i;
   HYPRE_Int       *P_offd_j;

   HYPRE_Int        P_diag_size, P_offd_size;

   HYPRE_Int       *P_marker, *P_marker_offd;

   HYPRE_Int        jj_counter, jj_counter_offd;
   HYPRE_Int       *jj_count, *jj_count_offd;
   //   HYPRE_Int              jj_begin_row,jj_begin_row_offd;
   //   HYPRE_Int              jj_end_row,jj_end_row_offd;

   HYPRE_Int        start_indexing = 0; /* start indexing for P_data at 0 */

   HYPRE_Int        n_fine = hypre_CSRMatrixNumRows(A_diag);

   HYPRE_Int       *fine_to_coarse;
   //HYPRE_BigInt    *fine_to_coarse_offd;
   HYPRE_Int       *coarse_counter;
   HYPRE_Int        coarse_shift;
   HYPRE_BigInt     total_global_cpts;
   //HYPRE_BigInt     my_first_cpt;
   HYPRE_Int        num_cols_P_offd;

   HYPRE_Int        i, i1;
   HYPRE_Int        j, jl, jj;
   HYPRE_Int        start;

   HYPRE_Real       one  = 1.0;

   HYPRE_Int        my_id;
   HYPRE_Int        num_procs;
   HYPRE_Int        num_threads;
   HYPRE_Int        num_sends;
   HYPRE_Int        index;
   HYPRE_Int        ns, ne, size, rest;

   HYPRE_Int       *int_buf_data;

   HYPRE_Real       wall_time;  /* for debugging instrumentation  */

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);
   //num_threads = hypre_NumThreads();
   // Temporary fix, disable threading
   // TODO: enable threading
   num_threads = 1;

   //my_first_cpt = num_cpts_global[0];
   if (my_id == (num_procs - 1)) { total_global_cpts = num_cpts_global[1]; }
   hypre_MPI_Bcast(&total_global_cpts, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);

   /*-------------------------------------------------------------------
   * Get the CF_marker data for the off-processor columns
   *-------------------------------------------------------------------*/

   if (debug_flag < 0)
   {
      debug_flag = -debug_flag;
   }

   if (debug_flag == 4) { wall_time = time_getWallclockSeconds(); }

   CF_marker_offd = hypre_CTAlloc(HYPRE_Int,  num_cols_A_offd, HYPRE_MEMORY_HOST);

   if (!comm_pkg)
   {
      hypre_MatvecCommPkgCreate(A);
      comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   }

   num_sends = hypre_ParCSRCommPkgNumSends(comm_pkg);
   int_buf_data = hypre_CTAlloc(HYPRE_Int,  hypre_ParCSRCommPkgSendMapStart(comm_pkg,
                                                                            num_sends), HYPRE_MEMORY_HOST);

   index = 0;
   for (i = 0; i < num_sends; i++)
   {
      start = hypre_ParCSRCommPkgSendMapStart(comm_pkg, i);
      for (j = start; j < hypre_ParCSRCommPkgSendMapStart(comm_pkg, i + 1); j++)
      {
         int_buf_data[index++] = CF_marker[hypre_ParCSRCommPkgSendMapElmt(comm_pkg, j)];
      }
   }

   comm_handle = hypre_ParCSRCommHandleCreate( 11, comm_pkg, int_buf_data, CF_marker_offd);
   hypre_ParCSRCommHandleDestroy(comm_handle);

   if (debug_flag == 4)
   {
      wall_time = time_getWallclockSeconds() - wall_time;
      hypre_printf("Proc = %d     Interp: Comm 1 CF_marker =    %f\n",
                   my_id, wall_time);
      fflush(NULL);
   }

   /*-----------------------------------------------------------------------
   *  First Pass: Determine size of P and fill in fine_to_coarse mapping.
   *-----------------------------------------------------------------------*/

   /*-----------------------------------------------------------------------
   *  Intialize counters and allocate mapping vector.
   *-----------------------------------------------------------------------*/

   coarse_counter = hypre_CTAlloc(HYPRE_Int,  num_threads, HYPRE_MEMORY_HOST);
   jj_count = hypre_CTAlloc(HYPRE_Int,  num_threads, HYPRE_MEMORY_HOST);
   jj_count_offd = hypre_CTAlloc(HYPRE_Int,  num_threads, HYPRE_MEMORY_HOST);

   fine_to_coarse = hypre_CTAlloc(HYPRE_Int,  n_fine, HYPRE_MEMORY_HOST);
#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (i = 0; i < n_fine; i++) { fine_to_coarse[i] = -1; }

   jj_counter = start_indexing;
   jj_counter_offd = start_indexing;

   /*-----------------------------------------------------------------------
   *  Loop over fine grid.
   *-----------------------------------------------------------------------*/

   /* RDF: this looks a little tricky, but doable */
#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,j,i1,jj,ns,ne,size,rest) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (j = 0; j < num_threads; j++)
   {
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;

      if (j < rest)
      {
         ns = j * size + j;
         ne = (j + 1) * size + j + 1;
      }
      else
      {
         ns = j * size + rest;
         ne = (j + 1) * size + rest;
      }
      for (i = ns; i < ne; i++)
      {
         /*--------------------------------------------------------------------
          *  If i is a C-point, interpolation is the identity. Also set up
          *  mapping vector.
          *--------------------------------------------------------------------*/

         if (CF_marker[i] >= 0)
         {
            jj_count[j]++;
            fine_to_coarse[i] = coarse_counter[j];
            coarse_counter[j]++;
         }
         /*--------------------------------------------------------------------
          *  If i is an F-point, interpolation is the approximation of A_{ff}^{-1}A_{fc}
          *--------------------------------------------------------------------*/
         else
         {
            for (jj = A_diag_i[i]; jj < A_diag_i[i + 1]; jj++)
            {
               i1 = A_diag_j[jj];
               if ((CF_marker[i1] >= 0) && (method > 0))
               {
                  jj_count[j]++;
               }
            }

            if (num_procs > 1)
            {
               for (jj = A_offd_i[i]; jj < A_offd_i[i + 1]; jj++)
               {
                  i1 = A_offd_j[jj];
                  if ((CF_marker_offd[i1] >= 0) && (method > 0))
                  {
                     jj_count_offd[j]++;
                  }
               }
            }
         }
      }
   }

   /*-----------------------------------------------------------------------
    *  Allocate  arrays.
    *-----------------------------------------------------------------------*/
   for (i = 0; i < num_threads - 1; i++)
   {
      coarse_counter[i + 1] += coarse_counter[i];
      jj_count[i + 1] += jj_count[i];
      jj_count_offd[i + 1] += jj_count_offd[i];
   }
   i = num_threads - 1;
   jj_counter = jj_count[i];
   jj_counter_offd = jj_count_offd[i];

   P_diag_size = jj_counter;

   P_diag_i    = hypre_CTAlloc(HYPRE_Int,  n_fine + 1, memory_location_P);
   P_diag_j    = hypre_CTAlloc(HYPRE_Int,  P_diag_size, memory_location_P);
   P_diag_data = hypre_CTAlloc(HYPRE_Real,  P_diag_size, memory_location_P);

   P_diag_i[n_fine] = jj_counter;

   P_offd_size = jj_counter_offd;

   P_offd_i    = hypre_CTAlloc(HYPRE_Int,  n_fine + 1, memory_location_P);
   P_offd_j    = hypre_CTAlloc(HYPRE_Int,  P_offd_size, memory_location_P);
   P_offd_data = hypre_CTAlloc(HYPRE_Real,  P_offd_size, memory_location_P);

   /*-----------------------------------------------------------------------
   *  Intialize some stuff.
   *-----------------------------------------------------------------------*/

   jj_counter = start_indexing;
   jj_counter_offd = start_indexing;

   if (debug_flag == 4)
   {
      wall_time = time_getWallclockSeconds() - wall_time;
      hypre_printf("Proc = %d     Interp: Internal work 1 =     %f\n",
                   my_id, wall_time);
      fflush(NULL);
   }

   /*-----------------------------------------------------------------------
   *  Send and receive fine_to_coarse info.
   *-----------------------------------------------------------------------*/

   if (debug_flag == 4) { wall_time = time_getWallclockSeconds(); }

   //fine_to_coarse_offd = hypre_CTAlloc(HYPRE_BigInt, num_cols_A_offd, HYPRE_MEMORY_HOST);

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,j,ns,ne,size,rest,coarse_shift) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (j = 0; j < num_threads; j++)
   {
      coarse_shift = 0;
      if (j > 0) { coarse_shift = coarse_counter[j - 1]; }
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;
      if (j < rest)
      {
         ns = j * size + j;
         ne = (j + 1) * size + j + 1;
      }
      else
      {
         ns = j * size + rest;
         ne = (j + 1) * size + rest;
      }
      for (i = ns; i < ne; i++)
      {
         fine_to_coarse[i] += coarse_shift;
      }
   }

   /*   index = 0;
      for (i = 0; i < num_sends; i++)
      {
         start = hypre_ParCSRCommPkgSendMapStart(comm_pkg, i);
         for (j = start; j < hypre_ParCSRCommPkgSendMapStart(comm_pkg, i+1); j++)
            big_buf_data[index++]
               = fine_to_coarse[hypre_ParCSRCommPkgSendMapElmt(comm_pkg,j)]+ my_first_cpt;
      }

      comm_handle = hypre_ParCSRCommHandleCreate( 21, comm_pkg, big_buf_data,
                                       fine_to_coarse_offd);

   hypre_ParCSRCommHandleDestroy(comm_handle);
   */
   if (debug_flag == 4)
   {
      wall_time = time_getWallclockSeconds() - wall_time;
      hypre_printf("Proc = %d     Interp: Comm 4 FineToCoarse = %f\n",
                   my_id, wall_time);
      fflush(NULL);
   }

   if (debug_flag == 4) { wall_time = time_getWallclockSeconds(); }

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
   //for (i = 0; i < n_fine; i++) fine_to_coarse[i] -= my_first_cpt;

   /*-----------------------------------------------------------------------
   *  Loop over fine grid points.
   *-----------------------------------------------------------------------*/
   a_diag = hypre_CTAlloc(HYPRE_Real,  n_fine, HYPRE_MEMORY_HOST);
   for (i = 0; i < n_fine; i++)
   {
      if (CF_marker[i] < 0)
      {
         for (jj = A_diag_i[i]; jj < A_diag_i[i + 1]; jj++)
         {
            i1 = A_diag_j[jj];
            if ( i == i1 ) /* diagonal of A only */
            {
               a_diag[i] = 1.0 / A_diag_data[jj];
            }
         }
      }
   }

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,j,jl,i1,jj,ns,ne,size,rest,P_marker,P_marker_offd,jj_counter,jj_counter_offd,jj_begin_row,jj_end_row,jj_begin_row_offd,jj_end_row_offd) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (jl = 0; jl < num_threads; jl++)
   {
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;
      if (jl < rest)
      {
         ns = jl * size + jl;
         ne = (jl + 1) * size + jl + 1;
      }
      else
      {
         ns = jl * size + rest;
         ne = (jl + 1) * size + rest;
      }
      jj_counter = 0;
      if (jl > 0) { jj_counter = jj_count[jl - 1]; }
      jj_counter_offd = 0;
      if (jl > 0) { jj_counter_offd = jj_count_offd[jl - 1]; }
      P_marker = hypre_CTAlloc(HYPRE_Int,  n_fine, HYPRE_MEMORY_HOST);
      if (num_cols_A_offd)
      {
         P_marker_offd = hypre_CTAlloc(HYPRE_Int,  num_cols_A_offd, HYPRE_MEMORY_HOST);
      }
      else
      {
         P_marker_offd = NULL;
      }

      for (i = 0; i < n_fine; i++)
      {
         P_marker[i] = -1;
      }
      for (i = 0; i < num_cols_A_offd; i++)
      {
         P_marker_offd[i] = -1;
      }
      for (i = ns; i < ne; i++)
      {
         /*--------------------------------------------------------------------
         *  If i is a c-point, interpolation is the identity.
         *--------------------------------------------------------------------*/
         if (CF_marker[i] >= 0)
         {
            P_diag_i[i] = jj_counter;
            P_diag_j[jj_counter]    = fine_to_coarse[i];
            P_diag_data[jj_counter] = one;
            jj_counter++;
         }
         /*--------------------------------------------------------------------
         *  If i is an F-point, build interpolation.
         *--------------------------------------------------------------------*/
         else
         {
            /* Diagonal part of P */
            P_diag_i[i] = jj_counter;
            for (jj = A_diag_i[i]; jj < A_diag_i[i + 1]; jj++)
            {
               i1 = A_diag_j[jj];

               /*--------------------------------------------------------------
                * If neighbor i1 is a C-point, set column number in P_diag_j
                * and initialize interpolation weight to zero.
                *--------------------------------------------------------------*/

               if ((CF_marker[i1] >= 0) && (method > 0))
               {
                  P_marker[i1] = jj_counter;
                  P_diag_j[jj_counter]    = fine_to_coarse[i1];
                  /*
                  if(method == 0)
                  {
                    P_diag_data[jj_counter] = 0.0;
                  }
                  */
                  if (method == 1)
                  {
                     P_diag_data[jj_counter] = - A_diag_data[jj];
                  }
                  else if (method == 2)
                  {
                     P_diag_data[jj_counter] = - A_diag_data[jj] * a_diag[i];
                  }
                  jj_counter++;
               }
            }

            /* Off-Diagonal part of P */
            P_offd_i[i] = jj_counter_offd;

            if (num_procs > 1)
            {
               for (jj = A_offd_i[i]; jj < A_offd_i[i + 1]; jj++)
               {
                  i1 = A_offd_j[jj];

                  /*-----------------------------------------------------------
                  * If neighbor i1 is a C-point, set column number in P_offd_j
                  * and initialize interpolation weight to zero.
                  *-----------------------------------------------------------*/

                  if ((CF_marker_offd[i1] >= 0) && (method > 0))
                  {
                     P_marker_offd[i1] = jj_counter_offd;
                     /*P_offd_j[jj_counter_offd]  = fine_to_coarse_offd[i1];*/
                     P_offd_j[jj_counter_offd]  = i1;
                     /*
                     if(method == 0)
                     {
                       P_offd_data[jj_counter_offd] = 0.0;
                     }
                     */
                     if (method == 1)
                     {
                        P_offd_data[jj_counter_offd] = - A_offd_data[jj];
                     }
                     else if (method == 2)
                     {
                        P_offd_data[jj_counter_offd] = - A_offd_data[jj] * a_diag[i];
                     }
                     jj_counter_offd++;
                  }
               }
            }
         }
         P_offd_i[i + 1] = jj_counter_offd;
      }
      hypre_TFree(P_marker, HYPRE_MEMORY_HOST);
      hypre_TFree(P_marker_offd, HYPRE_MEMORY_HOST);
   }
   hypre_TFree(a_diag, HYPRE_MEMORY_HOST);
   P = hypre_ParCSRMatrixCreate(comm,
                                hypre_ParCSRMatrixGlobalNumRows(A),
                                total_global_cpts,
                                hypre_ParCSRMatrixColStarts(A),
                                num_cpts_global,
                                0,
                                P_diag_i[n_fine],
                                P_offd_i[n_fine]);

   P_diag = hypre_ParCSRMatrixDiag(P);
   hypre_CSRMatrixData(P_diag) = P_diag_data;
   hypre_CSRMatrixI(P_diag) = P_diag_i;
   hypre_CSRMatrixJ(P_diag) = P_diag_j;
   P_offd = hypre_ParCSRMatrixOffd(P);
   hypre_CSRMatrixData(P_offd) = P_offd_data;
   hypre_CSRMatrixI(P_offd) = P_offd_i;
   hypre_CSRMatrixJ(P_offd) = P_offd_j;

   num_cols_P_offd = 0;

   if (P_offd_size)
   {
      P_marker = hypre_CTAlloc(HYPRE_Int,  num_cols_A_offd, HYPRE_MEMORY_HOST);
#if 0
#ifdef HYPRE_USING_OPENMP
      #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
      for (i = 0; i < num_cols_A_offd; i++)
      {
         P_marker[i] = 0;
      }
      num_cols_P_offd = 0;
      for (i = 0; i < P_offd_size; i++)
      {
         index = P_offd_j[i];
         if (!P_marker[index])
         {
            num_cols_P_offd++;
            P_marker[index] = 1;
         }
      }

      col_map_offd_P = hypre_CTAlloc(HYPRE_BigInt, num_cols_P_offd, HYPRE_MEMORY_HOST);
      tmp_map_offd = hypre_CTAlloc(HYPRE_Int, num_cols_P_offd, HYPRE_MEMORY_HOST);
      index = 0;
      for (i = 0; i < num_cols_P_offd; i++)
      {
         while (P_marker[index] == 0) { index++; }
         tmp_map_offd[i] = index++;
      }

#if 0
#ifdef HYPRE_USING_OPENMP
      #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
      for (i = 0; i < P_offd_size; i++)
         P_offd_j[i] = hypre_BinarySearch(tmp_map_offd,
                                          P_offd_j[i],
                                          num_cols_P_offd);
      hypre_TFree(P_marker, HYPRE_MEMORY_HOST);
   }

   for (i = 0; i < n_fine; i++)
      if (CF_marker[i] == -3) { CF_marker[i] = -1; }
   if (num_cols_P_offd)
   {
      hypre_ParCSRMatrixColMapOffd(P) = col_map_offd_P;
      hypre_CSRMatrixNumCols(P_offd) = num_cols_P_offd;
   }
   hypre_GetCommPkgRTFromCommPkgA(P, A, fine_to_coarse, tmp_map_offd);

   *P_ptr = P;

   hypre_TFree(tmp_map_offd, HYPRE_MEMORY_HOST);
   hypre_TFree(CF_marker_offd, HYPRE_MEMORY_HOST);
   hypre_TFree(int_buf_data, HYPRE_MEMORY_HOST);
   hypre_TFree(fine_to_coarse, HYPRE_MEMORY_HOST);
   // hypre_TFree(fine_to_coarse_offd, HYPRE_MEMORY_HOST);
   hypre_TFree(coarse_counter, HYPRE_MEMORY_HOST);
   hypre_TFree(jj_count, HYPRE_MEMORY_HOST);
   hypre_TFree(jj_count_offd, HYPRE_MEMORY_HOST);

   return (0);
}


/* Interpolation for MGR - Dynamic Row Sum method */

HYPRE_Int
hypre_MGRBuildPDRS( hypre_ParCSRMatrix   *A,
                    HYPRE_Int            *CF_marker,
                    HYPRE_BigInt         *num_cpts_global,
                    HYPRE_Int             blk_size,
                    HYPRE_Int             reserved_coarse_size,
                    HYPRE_Int             debug_flag,
                    hypre_ParCSRMatrix  **P_ptr)
{
   MPI_Comm          comm = hypre_ParCSRMatrixComm(A);
   hypre_ParCSRCommPkg     *comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   hypre_ParCSRCommHandle  *comm_handle;

   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real      *A_diag_data = hypre_CSRMatrixData(A_diag);
   HYPRE_Int       *A_diag_i = hypre_CSRMatrixI(A_diag);
   HYPRE_Int       *A_diag_j = hypre_CSRMatrixJ(A_diag);

   hypre_CSRMatrix *A_offd         = hypre_ParCSRMatrixOffd(A);
   HYPRE_Real      *A_offd_data    = hypre_CSRMatrixData(A_offd);
   HYPRE_Int       *A_offd_i = hypre_CSRMatrixI(A_offd);
   HYPRE_Int       *A_offd_j = hypre_CSRMatrixJ(A_offd);
   HYPRE_Int        num_cols_A_offd = hypre_CSRMatrixNumCols(A_offd);
   HYPRE_Real      *a_diag;

   hypre_ParCSRMatrix    *P;
   HYPRE_BigInt    *col_map_offd_P;
   HYPRE_Int       *tmp_map_offd;

   HYPRE_Int       *CF_marker_offd = NULL;

   hypre_CSRMatrix *P_diag;
   hypre_CSRMatrix *P_offd;

   HYPRE_Real      *P_diag_data;
   HYPRE_Int       *P_diag_i;
   HYPRE_Int       *P_diag_j;
   HYPRE_Real      *P_offd_data;
   HYPRE_Int       *P_offd_i;
   HYPRE_Int       *P_offd_j;

   HYPRE_Int        P_diag_size, P_offd_size;

   HYPRE_Int       *P_marker, *P_marker_offd;

   HYPRE_Int        jj_counter, jj_counter_offd;
   HYPRE_Int       *jj_count, *jj_count_offd;
   //   HYPRE_Int              jj_begin_row,jj_begin_row_offd;
   //   HYPRE_Int              jj_end_row,jj_end_row_offd;

   HYPRE_Int        start_indexing = 0; /* start indexing for P_data at 0 */

   HYPRE_Int        n_fine  = hypre_CSRMatrixNumRows(A_diag);

   HYPRE_Int       *fine_to_coarse;
   //HYPRE_BigInt             *fine_to_coarse_offd;
   HYPRE_Int       *coarse_counter;
   HYPRE_Int        coarse_shift;
   HYPRE_BigInt     total_global_cpts;
   //HYPRE_BigInt     my_first_cpt;
   HYPRE_Int        num_cols_P_offd;

   HYPRE_Int        i, i1;
   HYPRE_Int        j, jl, jj;
   HYPRE_Int        start;

   HYPRE_Real       one  = 1.0;

   HYPRE_Int        my_id;
   HYPRE_Int        num_procs;
   HYPRE_Int        num_threads;
   HYPRE_Int        num_sends;
   HYPRE_Int        index;
   HYPRE_Int        ns, ne, size, rest;

   HYPRE_Int       *int_buf_data;

   HYPRE_Real       wall_time;  /* for debugging instrumentation  */

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);
   //num_threads = hypre_NumThreads();
   // Temporary fix, disable threading
   // TODO: enable threading
   num_threads = 1;

   //my_first_cpt = num_cpts_global[0];
   if (my_id == (num_procs - 1)) { total_global_cpts = num_cpts_global[1]; }
   hypre_MPI_Bcast(&total_global_cpts, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);

   /*-------------------------------------------------------------------
    * Get the CF_marker data for the off-processor columns
    *-------------------------------------------------------------------*/

   if (debug_flag < 0)
   {
      debug_flag = -debug_flag;
   }

   if (debug_flag == 4) { wall_time = time_getWallclockSeconds(); }

   if (num_cols_A_offd) { CF_marker_offd = hypre_CTAlloc(HYPRE_Int,  num_cols_A_offd, HYPRE_MEMORY_HOST); }

   if (!comm_pkg)
   {
      hypre_MatvecCommPkgCreate(A);
      comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   }

   num_sends = hypre_ParCSRCommPkgNumSends(comm_pkg);
   int_buf_data = hypre_CTAlloc(HYPRE_Int,  hypre_ParCSRCommPkgSendMapStart(comm_pkg,
                                                                            num_sends), HYPRE_MEMORY_HOST);

   index = 0;
   for (i = 0; i < num_sends; i++)
   {
      start = hypre_ParCSRCommPkgSendMapStart(comm_pkg, i);
      for (j = start; j < hypre_ParCSRCommPkgSendMapStart(comm_pkg, i + 1); j++)
         int_buf_data[index++]
            = CF_marker[hypre_ParCSRCommPkgSendMapElmt(comm_pkg, j)];
   }

   comm_handle = hypre_ParCSRCommHandleCreate( 11, comm_pkg, int_buf_data,
                                               CF_marker_offd);
   hypre_ParCSRCommHandleDestroy(comm_handle);

   if (debug_flag == 4)
   {
      wall_time = time_getWallclockSeconds() - wall_time;
      hypre_printf("Proc = %d     Interp: Comm 1 CF_marker =    %f\n",
                   my_id, wall_time);
      fflush(NULL);
   }

   /*-----------------------------------------------------------------------
    *  First Pass: Determine size of P and fill in fine_to_coarse mapping.
    *-----------------------------------------------------------------------*/

   /*-----------------------------------------------------------------------
    *  Intialize counters and allocate mapping vector.
    *-----------------------------------------------------------------------*/

   coarse_counter = hypre_CTAlloc(HYPRE_Int,  num_threads, HYPRE_MEMORY_HOST);
   jj_count = hypre_CTAlloc(HYPRE_Int,  num_threads, HYPRE_MEMORY_HOST);
   jj_count_offd = hypre_CTAlloc(HYPRE_Int,  num_threads, HYPRE_MEMORY_HOST);

   fine_to_coarse = hypre_CTAlloc(HYPRE_Int,  n_fine, HYPRE_MEMORY_HOST);
#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (i = 0; i < n_fine; i++) { fine_to_coarse[i] = -1; }

   jj_counter = start_indexing;
   jj_counter_offd = start_indexing;

   /*-----------------------------------------------------------------------
    *  Loop over fine grid.
    *-----------------------------------------------------------------------*/

   /* RDF: this looks a little tricky, but doable */
#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,j,i1,jj,ns,ne,size,rest) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (j = 0; j < num_threads; j++)
   {
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;

      if (j < rest)
      {
         ns = j * size + j;
         ne = (j + 1) * size + j + 1;
      }
      else
      {
         ns = j * size + rest;
         ne = (j + 1) * size + rest;
      }
      for (i = ns; i < ne; i++)
      {
         /*--------------------------------------------------------------------
          *  If i is a C-point, interpolation is the identity. Also set up
          *  mapping vector.
          *--------------------------------------------------------------------*/

         if (CF_marker[i] >= 0)
         {
            jj_count[j]++;
            fine_to_coarse[i] = coarse_counter[j];
            coarse_counter[j]++;
         }
         /*--------------------------------------------------------------------
          *  If i is an F-point, interpolation is the approximation of A_{ff}^{-1}A_{fc}
          *--------------------------------------------------------------------*/
         else
         {
            for (jj = A_diag_i[i]; jj < A_diag_i[i + 1]; jj++)
            {
               i1 = A_diag_j[jj];
               if (CF_marker[i1] >= 0)
               {
                  jj_count[j]++;
               }
            }

            if (num_procs > 1)
            {
               for (jj = A_offd_i[i]; jj < A_offd_i[i + 1]; jj++)
               {
                  i1 = A_offd_j[jj];
                  if (CF_marker_offd[i1] >= 0)
                  {
                     jj_count_offd[j]++;
                  }
               }
            }
         }
         /*--------------------------------------------------------------------
          *  Set up the indexes for the DRS method
          *--------------------------------------------------------------------*/

      }
   }

   /*-----------------------------------------------------------------------
    *  Allocate  arrays.
    *-----------------------------------------------------------------------*/
   for (i = 0; i < num_threads - 1; i++)
   {
      coarse_counter[i + 1] += coarse_counter[i];
      jj_count[i + 1] += jj_count[i];
      jj_count_offd[i + 1] += jj_count_offd[i];
   }
   i = num_threads - 1;
   jj_counter = jj_count[i];
   jj_counter_offd = jj_count_offd[i];

   P_diag_size = jj_counter;

   P_diag_i    = hypre_CTAlloc(HYPRE_Int,  n_fine + 1, HYPRE_MEMORY_HOST);
   P_diag_j    = hypre_CTAlloc(HYPRE_Int,  P_diag_size, HYPRE_MEMORY_HOST);
   P_diag_data = hypre_CTAlloc(HYPRE_Real,  P_diag_size, HYPRE_MEMORY_HOST);

   P_diag_i[n_fine] = jj_counter;


   P_offd_size = jj_counter_offd;

   P_offd_i    = hypre_CTAlloc(HYPRE_Int,  n_fine + 1, HYPRE_MEMORY_HOST);
   P_offd_j    = hypre_CTAlloc(HYPRE_Int,  P_offd_size, HYPRE_MEMORY_HOST);
   P_offd_data = hypre_CTAlloc(HYPRE_Real,  P_offd_size, HYPRE_MEMORY_HOST);

   /*-----------------------------------------------------------------------
    *  Intialize some stuff.
    *-----------------------------------------------------------------------*/

   jj_counter = start_indexing;
   jj_counter_offd = start_indexing;

   if (debug_flag == 4)
   {
      wall_time = time_getWallclockSeconds() - wall_time;
      hypre_printf("Proc = %d     Interp: Internal work 1 =     %f\n",
                   my_id, wall_time);
      fflush(NULL);
   }

   /*-----------------------------------------------------------------------
    *  Send and receive fine_to_coarse info.
    *-----------------------------------------------------------------------*/

   if (debug_flag == 4) { wall_time = time_getWallclockSeconds(); }

   //fine_to_coarse_offd = hypre_CTAlloc(HYPRE_Int,  num_cols_A_offd, HYPRE_MEMORY_HOST);

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,j,ns,ne,size,rest,coarse_shift) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (j = 0; j < num_threads; j++)
   {
      coarse_shift = 0;
      if (j > 0) { coarse_shift = coarse_counter[j - 1]; }
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;
      if (j < rest)
      {
         ns = j * size + j;
         ne = (j + 1) * size + j + 1;
      }
      else
      {
         ns = j * size + rest;
         ne = (j + 1) * size + rest;
      }
      for (i = ns; i < ne; i++)
      {
         fine_to_coarse[i] += coarse_shift;
      }
   }

   /*index = 0;
   for (i = 0; i < num_sends; i++)
   {
      start = hypre_ParCSRCommPkgSendMapStart(comm_pkg, i);
      for (j = start; j < hypre_ParCSRCommPkgSendMapStart(comm_pkg, i+1); j++)
         int_buf_data[index++]
            = fine_to_coarse[hypre_ParCSRCommPkgSendMapElmt(comm_pkg,j)];
   }

   comm_handle = hypre_ParCSRCommHandleCreate( 11, comm_pkg, int_buf_data,
                                    fine_to_coarse_offd);

   hypre_ParCSRCommHandleDestroy(comm_handle);
   */
   if (debug_flag == 4)
   {
      wall_time = time_getWallclockSeconds() - wall_time;
      hypre_printf("Proc = %d     Interp: Comm 4 FineToCoarse = %f\n",
                   my_id, wall_time);
      fflush(NULL);
   }

   if (debug_flag == 4) { wall_time = time_getWallclockSeconds(); }

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif

   //for (i = 0; i < n_fine; i++) fine_to_coarse[i] -= my_first_cpt;

   /*-----------------------------------------------------------------------
    *  Loop over fine grid points.
    *-----------------------------------------------------------------------*/
   a_diag = hypre_CTAlloc(HYPRE_Real,  n_fine, HYPRE_MEMORY_HOST);
   for (i = 0; i < n_fine; i++)
   {
      for (jj = A_diag_i[i]; jj < A_diag_i[i + 1]; jj++)
      {
         i1 = A_diag_j[jj];
         if ( i == i1 ) /* diagonal of A only */
         {
            a_diag[i] = 1.0 / A_diag_data[jj];
         }
      }
   }

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,j,jl,i1,jj,ns,ne,size,rest,P_marker,P_marker_offd,jj_counter,jj_counter_offd,jj_begin_row,jj_end_row,jj_begin_row_offd,jj_end_row_offd) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (jl = 0; jl < num_threads; jl++)
   {
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;
      if (jl < rest)
      {
         ns = jl * size + jl;
         ne = (jl + 1) * size + jl + 1;
      }
      else
      {
         ns = jl * size + rest;
         ne = (jl + 1) * size + rest;
      }
      jj_counter = 0;
      if (jl > 0) { jj_counter = jj_count[jl - 1]; }
      jj_counter_offd = 0;
      if (jl > 0) { jj_counter_offd = jj_count_offd[jl - 1]; }
      P_marker = hypre_CTAlloc(HYPRE_Int,  n_fine, HYPRE_MEMORY_HOST);
      if (num_cols_A_offd)
      {
         P_marker_offd = hypre_CTAlloc(HYPRE_Int,  num_cols_A_offd, HYPRE_MEMORY_HOST);
      }
      else
      {
         P_marker_offd = NULL;
      }

      for (i = 0; i < n_fine; i++)
      {
         P_marker[i] = -1;
      }
      for (i = 0; i < num_cols_A_offd; i++)
      {
         P_marker_offd[i] = -1;
      }
      for (i = ns; i < ne; i++)
      {
         /*--------------------------------------------------------------------
          *  If i is a c-point, interpolation is the identity.
          *--------------------------------------------------------------------*/
         if (CF_marker[i] >= 0)
         {
            P_diag_i[i] = jj_counter;
            P_diag_j[jj_counter]    = fine_to_coarse[i];
            P_diag_data[jj_counter] = one;
            jj_counter++;
         }
         /*--------------------------------------------------------------------
          *  If i is an F-point, build interpolation.
          *--------------------------------------------------------------------*/
         else
         {
            /* Diagonal part of P */
            P_diag_i[i] = jj_counter;
            for (jj = A_diag_i[i]; jj < A_diag_i[i + 1]; jj++)
            {
               i1 = A_diag_j[jj];

               /*--------------------------------------------------------------
                * If neighbor i1 is a C-point, set column number in P_diag_j
                * and initialize interpolation weight to zero.
                *--------------------------------------------------------------*/

               if (CF_marker[i1] >= 0)
               {
                  P_marker[i1] = jj_counter;
                  P_diag_j[jj_counter]    = fine_to_coarse[i1];
                  P_diag_data[jj_counter] = - A_diag_data[jj] * a_diag[i];

                  jj_counter++;
               }
            }

            /* Off-Diagonal part of P */
            P_offd_i[i] = jj_counter_offd;

            if (num_procs > 1)
            {
               for (jj = A_offd_i[i]; jj < A_offd_i[i + 1]; jj++)
               {
                  i1 = A_offd_j[jj];

                  /*-----------------------------------------------------------
                   * If neighbor i1 is a C-point, set column number in P_offd_j
                   * and initialize interpolation weight to zero.
                   *-----------------------------------------------------------*/

                  if (CF_marker_offd[i1] >= 0)
                  {
                     P_marker_offd[i1] = jj_counter_offd;
                     /*P_offd_j[jj_counter_offd]  = fine_to_coarse_offd[i1];*/
                     P_offd_j[jj_counter_offd]  = i1;
                     P_offd_data[jj_counter_offd] = - A_offd_data[jj] * a_diag[i];

                     jj_counter_offd++;
                  }
               }
            }
         }
         P_offd_i[i + 1] = jj_counter_offd;
      }
      hypre_TFree(P_marker, HYPRE_MEMORY_HOST);
      hypre_TFree(P_marker_offd, HYPRE_MEMORY_HOST);
   }
   hypre_TFree(a_diag, HYPRE_MEMORY_HOST);
   P = hypre_ParCSRMatrixCreate(comm,
                                hypre_ParCSRMatrixGlobalNumRows(A),
                                total_global_cpts,
                                hypre_ParCSRMatrixColStarts(A),
                                num_cpts_global,
                                0,
                                P_diag_i[n_fine],
                                P_offd_i[n_fine]);

   P_diag = hypre_ParCSRMatrixDiag(P);
   hypre_CSRMatrixData(P_diag) = P_diag_data;
   hypre_CSRMatrixI(P_diag) = P_diag_i;
   hypre_CSRMatrixJ(P_diag) = P_diag_j;
   P_offd = hypre_ParCSRMatrixOffd(P);
   hypre_CSRMatrixData(P_offd) = P_offd_data;
   hypre_CSRMatrixI(P_offd) = P_offd_i;
   hypre_CSRMatrixJ(P_offd) = P_offd_j;

   num_cols_P_offd = 0;

   if (P_offd_size)
   {
      P_marker = hypre_CTAlloc(HYPRE_Int,  num_cols_A_offd, HYPRE_MEMORY_HOST);

#if 0
#ifdef HYPRE_USING_OPENMP
      #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
      for (i = 0; i < num_cols_A_offd; i++)
      {
         P_marker[i] = 0;
      }
      num_cols_P_offd = 0;
      for (i = 0; i < P_offd_size; i++)
      {
         index = P_offd_j[i];
         if (!P_marker[index])
         {
            num_cols_P_offd++;
            P_marker[index] = 1;
         }
      }

      tmp_map_offd = hypre_CTAlloc(HYPRE_Int, num_cols_P_offd, HYPRE_MEMORY_HOST);
      col_map_offd_P = hypre_CTAlloc(HYPRE_BigInt, num_cols_P_offd, HYPRE_MEMORY_HOST);
      index = 0;
      for (i = 0; i < num_cols_P_offd; i++)
      {
         while (P_marker[index] == 0) { index++; }
         tmp_map_offd[i] = index++;
      }

#if 0
#ifdef HYPRE_USING_OPENMP
      #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
      for (i = 0; i < P_offd_size; i++)
         P_offd_j[i] = hypre_BinarySearch(tmp_map_offd,
                                          P_offd_j[i],
                                          num_cols_P_offd);
      hypre_TFree(P_marker, HYPRE_MEMORY_HOST);
   }

   for (i = 0; i < n_fine; i++)
      if (CF_marker[i] == -3) { CF_marker[i] = -1; }
   if (num_cols_P_offd)
   {
      hypre_ParCSRMatrixColMapOffd(P) = col_map_offd_P;
      hypre_CSRMatrixNumCols(P_offd) = num_cols_P_offd;
   }
   hypre_GetCommPkgRTFromCommPkgA(P, A, fine_to_coarse, tmp_map_offd);

   *P_ptr = P;

   hypre_TFree(tmp_map_offd, HYPRE_MEMORY_HOST);
   hypre_TFree(CF_marker_offd, HYPRE_MEMORY_HOST);
   hypre_TFree(int_buf_data, HYPRE_MEMORY_HOST);
   hypre_TFree(fine_to_coarse, HYPRE_MEMORY_HOST);
   // hypre_TFree(fine_to_coarse_offd, HYPRE_MEMORY_HOST);
   hypre_TFree(coarse_counter, HYPRE_MEMORY_HOST);
   hypre_TFree(jj_count, HYPRE_MEMORY_HOST);
   hypre_TFree(jj_count_offd, HYPRE_MEMORY_HOST);

   return (0);
}

/* Scale ParCSR matrix A = scalar * A
 * A: the target CSR matrix
 * vector: array of real numbers
 */
HYPRE_Int
hypre_ParCSRMatrixLeftScale(HYPRE_Real *vector,
                            hypre_ParCSRMatrix *A)
{
   HYPRE_Int i, j, n_local;
   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real      *A_diag_data = hypre_CSRMatrixData(A_diag);
   HYPRE_Int             *A_diag_i = hypre_CSRMatrixI(A_diag);

   hypre_CSRMatrix *A_offd         = hypre_ParCSRMatrixOffd(A);
   HYPRE_Real      *A_offd_data    = hypre_CSRMatrixData(A_offd);
   HYPRE_Int             *A_offd_i = hypre_CSRMatrixI(A_offd);

   n_local = hypre_CSRMatrixNumRows(A_diag);

   for (i = 0; i < n_local; i++)
   {
      HYPRE_Real factor = vector[i];
      for (j = A_diag_i[i]; j < A_diag_i[i + 1]; j++)
      {
         A_diag_data[j] *= factor;
      }
      for (j = A_offd_i[i]; j < A_offd_i[i + 1]; j++)
      {
         A_offd_data[j] *= factor;
      }
   }

   return (0);
}

HYPRE_Int
hypre_MGRGetAcfCPR(hypre_ParCSRMatrix    *A,
                   HYPRE_Int       blk_size,
                   HYPRE_Int             *c_marker,
                   HYPRE_Int             *f_marker,
                   hypre_ParCSRMatrix    **A_CF_ptr)
{
   MPI_Comm comm = hypre_ParCSRMatrixComm(A);
   HYPRE_Int i, j, jj, jj1;
   HYPRE_Int jj_counter, cpts_cnt;
   hypre_ParCSRMatrix *A_CF = NULL;
   hypre_CSRMatrix *A_CF_diag = NULL;

   HYPRE_MemoryLocation memory_location = hypre_ParCSRMatrixMemoryLocation(A);
   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);

   HYPRE_Int *A_diag_i = hypre_CSRMatrixI(A_diag);
   HYPRE_Int *A_diag_j = hypre_CSRMatrixJ(A_diag);
   HYPRE_Complex *A_diag_data = hypre_CSRMatrixData(A_diag);

   HYPRE_Int total_fpts, n_fpoints;
   HYPRE_Int num_rows = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));
   HYPRE_Int nnz_diag_new = 0;
   HYPRE_Int num_procs, my_id;
   hypre_IntArray *wrap_cf = NULL;
   hypre_IntArray *coarse_dof_func_ptr = NULL;
   HYPRE_BigInt num_row_cpts_global[2], num_col_fpts_global[2];
   HYPRE_BigInt total_global_row_cpts, total_global_col_fpts;

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);

   // Count total F-points
   // Also setup F to C column map
   total_fpts = 0;
   HYPRE_Int *f_to_c_col_map = hypre_CTAlloc(HYPRE_Int, num_rows, HYPRE_MEMORY_HOST);
   for (i = 0; i < num_rows; i++)
   {
      //      if (c_marker[i] == 1)
      //      {
      //         total_cpts++;
      //      }
      if (f_marker[i] == 1)
      {
         f_to_c_col_map[i] = total_fpts;
         total_fpts++;
      }
   }
   n_fpoints = blk_size;
   /* get the number of coarse rows */
   wrap_cf = hypre_IntArrayCreate(num_rows);
   hypre_IntArrayMemoryLocation(wrap_cf) = HYPRE_MEMORY_HOST;
   hypre_IntArrayData(wrap_cf) = c_marker;
   hypre_BoomerAMGCoarseParms(comm, num_rows, 1, NULL, wrap_cf, &coarse_dof_func_ptr,
                              num_row_cpts_global);
   hypre_IntArrayDestroy(coarse_dof_func_ptr);
   coarse_dof_func_ptr = NULL;

   //hypre_printf("my_id = %d, cpts_this = %d, cpts_next = %d\n", my_id, num_row_cpts_global[0], num_row_cpts_global[1]);

   if (my_id == (num_procs - 1)) { total_global_row_cpts = num_row_cpts_global[1]; }
   hypre_MPI_Bcast(&total_global_row_cpts, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);

   /* get the number of coarse rows */
   hypre_IntArrayData(wrap_cf) = f_marker;
   hypre_BoomerAMGCoarseParms(comm, num_rows, 1, NULL, wrap_cf, &coarse_dof_func_ptr,
                              num_col_fpts_global);
   hypre_IntArrayDestroy(coarse_dof_func_ptr);
   coarse_dof_func_ptr = NULL;
   hypre_IntArrayData(wrap_cf) = NULL;
   hypre_IntArrayDestroy(wrap_cf);

   //hypre_printf("my_id = %d, cpts_this = %d, cpts_next = %d\n", my_id, num_col_fpts_global[0], num_col_fpts_global[1]);

   if (my_id == (num_procs - 1)) { total_global_col_fpts = num_col_fpts_global[1]; }
   hypre_MPI_Bcast(&total_global_col_fpts, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);

   // First pass: count the nnz of A_CF
   jj_counter = 0;
   cpts_cnt = 0;
   for (i = 0; i < num_rows; i++)
   {
      if (c_marker[i] == 1)
      {
         for (j = A_diag_i[i]; j < A_diag_i[i + 1]; j++)
         {
            jj = A_diag_j[j];
            if (f_marker[jj] == 1)
            {
               jj1 = f_to_c_col_map[jj];
               if (jj1 >= cpts_cnt * n_fpoints && jj1 < (cpts_cnt + 1)*n_fpoints)
               {
                  jj_counter++;
               }
            }
         }
         cpts_cnt++;
      }
   }
   nnz_diag_new = jj_counter;

   HYPRE_Int *A_CF_diag_i = hypre_CTAlloc(HYPRE_Int, cpts_cnt + 1, memory_location);
   HYPRE_Int *A_CF_diag_j = hypre_CTAlloc(HYPRE_Int, nnz_diag_new, memory_location);
   HYPRE_Complex *A_CF_diag_data = hypre_CTAlloc(HYPRE_Complex, nnz_diag_new, memory_location);
   A_CF_diag_i[cpts_cnt] = nnz_diag_new;

   jj_counter = 0;
   cpts_cnt = 0;
   for (i = 0; i < num_rows; i++)
   {
      if (c_marker[i] == 1)
      {
         A_CF_diag_i[cpts_cnt] = jj_counter;
         for (j = A_diag_i[i]; j < A_diag_i[i + 1]; j++)
         {
            jj = A_diag_j[j];
            if (f_marker[jj] == 1)
            {
               jj1 = f_to_c_col_map[jj];
               if (jj1 >= cpts_cnt * n_fpoints && jj1 < (cpts_cnt + 1)*n_fpoints)
               {
                  A_CF_diag_j[jj_counter] = jj1;
                  A_CF_diag_data[jj_counter] = A_diag_data[j];
                  jj_counter++;
               }
            }
         }
         cpts_cnt++;
      }
   }

   //hypre_printf("rank = %d, Before creaing A_CF \n", my_id);
   A_CF = hypre_ParCSRMatrixCreate(comm,
                                   total_global_row_cpts,
                                   total_global_col_fpts,
                                   num_row_cpts_global,
                                   num_col_fpts_global,
                                   0,
                                   nnz_diag_new,
                                   0);
   //hypre_printf("rank = %d, After creaing A_CF \n", my_id);

   A_CF_diag = hypre_ParCSRMatrixDiag(A_CF);
   hypre_CSRMatrixData(A_CF_diag) = A_CF_diag_data;
   hypre_CSRMatrixI(A_CF_diag) = A_CF_diag_i;
   hypre_CSRMatrixJ(A_CF_diag) = A_CF_diag_j;

   hypre_CSRMatrixData(hypre_ParCSRMatrixOffd(A_CF)) = NULL;
   hypre_CSRMatrixI(hypre_ParCSRMatrixOffd(A_CF)) = NULL;
   hypre_CSRMatrixJ(hypre_ParCSRMatrixOffd(A_CF)) = NULL;

   *A_CF_ptr = A_CF;

   hypre_TFree(f_to_c_col_map, HYPRE_MEMORY_HOST);

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRTruncateAcfCPR(hypre_ParCSRMatrix    *A_CF,
                        hypre_ParCSRMatrix    **A_CF_new_ptr)
{
   HYPRE_Int i, j, jj;
   HYPRE_Int jj_counter;
   hypre_ParCSRMatrix *A_CF_new = NULL;
   hypre_CSRMatrix *A_CF_new_diag = NULL;

   HYPRE_MemoryLocation memory_location = hypre_ParCSRMatrixMemoryLocation(A_CF);
   hypre_CSRMatrix *A_CF_diag = hypre_ParCSRMatrixDiag(A_CF);

   HYPRE_Int *A_CF_diag_i = hypre_CSRMatrixI(A_CF_diag);
   HYPRE_Int *A_CF_diag_j = hypre_CSRMatrixJ(A_CF_diag);
   HYPRE_Complex *A_CF_diag_data = hypre_CSRMatrixData(A_CF_diag);

   HYPRE_Int global_nrows = hypre_ParCSRMatrixGlobalNumRows(A_CF);
   HYPRE_Int global_ncols = hypre_ParCSRMatrixGlobalNumCols(A_CF);
   HYPRE_Int n_fpoints = global_ncols / global_nrows;
   HYPRE_Int num_rows = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A_CF));
   HYPRE_Int nnz_diag_new = 0;

   // First pass: count the nnz of new A_CF
   jj_counter = 0;
   for (i = 0; i < num_rows; i++)
   {
      for (j = A_CF_diag_i[i]; j < A_CF_diag_i[i + 1]; j++)
      {
         jj = A_CF_diag_j[j];
         if (jj >= i * n_fpoints && jj < (i + 1)*n_fpoints)
         {
            jj_counter++;
         }
      }
   }
   nnz_diag_new = jj_counter;

   HYPRE_Int *A_CF_diag_i_new = hypre_CTAlloc(HYPRE_Int, num_rows + 1, memory_location);
   HYPRE_Int *A_CF_diag_j_new = hypre_CTAlloc(HYPRE_Int, nnz_diag_new, memory_location);
   HYPRE_Complex *A_CF_diag_data_new = hypre_CTAlloc(HYPRE_Complex, nnz_diag_new, memory_location);

   jj_counter = 0;
   for (i = 0; i < num_rows; i++)
   {
      A_CF_diag_i_new[i] = jj_counter;
      for (j = A_CF_diag_i[i]; j < A_CF_diag_i[i + 1]; j++)
      {
         jj = A_CF_diag_j[j];
         if (jj >= i * n_fpoints && jj < (i + 1)*n_fpoints)
         {
            A_CF_diag_j_new[jj_counter] = jj;
            A_CF_diag_data_new[jj_counter] = A_CF_diag_data[j];
            jj_counter++;
         }
      }
   }
   A_CF_diag_i_new[num_rows] = nnz_diag_new;

   A_CF_new = hypre_ParCSRMatrixCreate(hypre_ParCSRMatrixComm(A_CF),
                                       hypre_ParCSRMatrixGlobalNumRows(A_CF),
                                       hypre_ParCSRMatrixGlobalNumCols(A_CF),
                                       hypre_ParCSRMatrixRowStarts(A_CF),
                                       hypre_ParCSRMatrixColStarts(A_CF),
                                       0,
                                       nnz_diag_new,
                                       0);

   A_CF_new_diag = hypre_ParCSRMatrixDiag(A_CF_new);
   hypre_CSRMatrixData(A_CF_new_diag) = A_CF_diag_data_new;
   hypre_CSRMatrixI(A_CF_new_diag) = A_CF_diag_i_new;
   hypre_CSRMatrixJ(A_CF_new_diag) = A_CF_diag_j_new;

   //hypre_ParCSRMatrixOwnsRowStarts(A_CF_new) = 0;
   //hypre_ParCSRMatrixOwnsColStarts(A_CF_new) = 0;

   *A_CF_new_ptr = A_CF_new;

   return hypre_error_flag;
}

/************************************************************
* Available methods:
*   1: inv(A_FF) approximated by its (block) diagonal inverse
*   2: CPR-like approximation with inv(A_FF) approximated by its diagonal inverse
*   3: CPR-like approximation with inv(A_FF) approximated by its block diagonal inverse
*   4: inv(A_FF) approximated by sparse approximate inverse
*************************************************************/
HYPRE_Int
hypre_MGRComputeNonGalerkinCoarseGrid(hypre_ParCSRMatrix    *A,
                                      hypre_ParCSRMatrix    *Wp,
                                      hypre_ParCSRMatrix    *RT,
                                      HYPRE_Int             bsize,
                                      HYPRE_Int             ordering,
                                      HYPRE_Int             method,
                                      HYPRE_Int             Pmax,
                                      HYPRE_Int             *CF_marker,
                                      hypre_ParCSRMatrix    **A_h_ptr)
{
   HYPRE_Int *c_marker, *f_marker;
   HYPRE_Int n_local_fine_grid, i, i1, jj;
   hypre_ParCSRMatrix *A_cc = NULL;
   hypre_ParCSRMatrix *A_ff = NULL;
   hypre_ParCSRMatrix *A_fc = NULL;
   hypre_ParCSRMatrix *A_cf = NULL;
   hypre_ParCSRMatrix *A_ff_inv = NULL;
   hypre_ParCSRMatrix *A_h = NULL;
   hypre_ParCSRMatrix *A_h_correction = NULL;
   HYPRE_Int  max_elmts = Pmax;
   HYPRE_Real alpha = -1.0;

   HYPRE_BigInt         coarse_pnts_global[2];
   HYPRE_BigInt         fine_pnts_global[2];
   hypre_IntArray *marker_array = NULL;
   //   HYPRE_Real wall_time = 0.;
   //   HYPRE_Real wall_time_1 = 0.;

   HYPRE_Int my_id;
   MPI_Comm comm = hypre_ParCSRMatrixComm(A);
   hypre_MPI_Comm_rank(comm, &my_id);
   HYPRE_MemoryLocation memory_location = hypre_ParCSRMatrixMemoryLocation(A);

   //wall_time = time_getWallclockSeconds();
   n_local_fine_grid = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));
   c_marker = hypre_CTAlloc(HYPRE_Int, n_local_fine_grid, memory_location);
   f_marker = hypre_CTAlloc(HYPRE_Int, n_local_fine_grid, memory_location);

   for (i = 0; i < n_local_fine_grid; i++)
   {
      HYPRE_Int point_type = CF_marker[i];
      //hypre_assert(point_type == 1 || point_type == -1);
      c_marker[i] = point_type;
      f_marker[i] = -point_type;
   }
   // get local range for C and F points
   // Set IntArray pointers to obtain global row and col (start) ranges
   marker_array = hypre_IntArrayCreate(n_local_fine_grid);
   hypre_IntArrayMemoryLocation(marker_array) = memory_location;
   hypre_IntArrayData(marker_array) = c_marker;
   // get range for c_points
   hypre_BoomerAMGCoarseParms(comm, n_local_fine_grid, 1, NULL, marker_array, NULL,
                              coarse_pnts_global);
   // get range for f_points
   hypre_IntArrayData(marker_array) = f_marker;
   hypre_BoomerAMGCoarseParms(comm, n_local_fine_grid, 1, NULL, marker_array, NULL, fine_pnts_global);

   // Generate A_FF, A_FC, A_CC and A_CF submatrices.
   // Note: Not all submatrices are needed for each method below.
   // hypre_ParCSRMatrixGenerateFFFC computes A_FF and A_FC given the CF_marker and start locations for the global C-points.
   // To compute A_CC and A_CF, we need to pass in equivalent information for F-points. (i.e. CF_marker marking F-points
   // and start locations for the global F-points.
   hypre_ParCSRMatrixGenerateFFFC(A, c_marker, coarse_pnts_global, NULL, &A_fc, &A_ff);
   hypre_ParCSRMatrixGenerateFFFC(A, f_marker, fine_pnts_global, NULL, &A_cf, &A_cc);

   if (method == 1)
   {
      if (Wp != NULL)
      {
         A_h_correction = hypre_ParCSRMatMat(A_cf, Wp);
      }
      else
      {
         // Build block diagonal inverse for A_FF
         hypre_ParCSRMatrixBlockDiagMatrix(A_ff, 1, -1, NULL, &A_ff_inv, 1);
         // compute Wp = A_ff_inv * A_fc
         // NOTE: Use hypre_ParMatmul here instead of hypre_ParCSRMatMat to avoid padding
         // zero entries at diagonals for the latter routine. Use MatMat once this padding
         // issue is resolved since it is more efficient.
         //         hypre_ParCSRMatrix *Wp_tmp = hypre_ParCSRMatMat(A_ff_inv, A_fc);
         hypre_ParCSRMatrix *Wp_tmp = hypre_ParMatmul(A_ff_inv, A_fc);
         // compute correction A_h_correction = A_cf * (A_ff_inv * A_fc);
         //         A_h_correction = hypre_ParMatmul(A_cf, Wp_tmp);
         A_h_correction = hypre_ParCSRMatMat(A_cf, Wp_tmp);
         hypre_ParCSRMatrixDestroy(Wp_tmp);
         hypre_ParCSRMatrixDestroy(A_ff_inv);
      }
   }
   else if (method == 2 || method == 3)
   {
      // extract the diagonal of A_cf
      hypre_ParCSRMatrix *A_cf_truncated = NULL;
      hypre_MGRGetAcfCPR(A, bsize, c_marker, f_marker, &A_cf_truncated);
      if (Wp != NULL)
      {
         A_h_correction = hypre_ParCSRMatMat(A_cf_truncated, Wp);
      }
      else
      {
         HYPRE_Int blk_inv_size = method == 2 ? bsize : 1;
         hypre_ParCSRMatrixBlockDiagMatrix(A_ff, blk_inv_size, -1, NULL, &A_ff_inv, 1);
         hypre_ParCSRMatrix *Wr = NULL;
         Wr = hypre_ParCSRMatMat(A_cf_truncated, A_ff_inv);
         A_h_correction = hypre_ParCSRMatMat(Wr, A_fc);
         hypre_ParCSRMatrixDestroy(Wr);
         hypre_ParCSRMatrixDestroy(A_ff_inv);
      }
      hypre_ParCSRMatrixDestroy(A_cf_truncated);
   }
   else if (method == 4)
   {
      // Approximate inverse for ideal interploation
      hypre_ParCSRMatrix *A_ff_inv = NULL;
      hypre_ParCSRMatrix *minus_Wp = NULL;
      hypre_MGRApproximateInverse(A_ff, &A_ff_inv);
      minus_Wp = hypre_ParCSRMatMat(A_ff_inv, A_fc);
      A_h_correction = hypre_ParCSRMatMat(A_cf, minus_Wp);

      hypre_ParCSRMatrixDestroy(minus_Wp);
   }
   // Free data
   hypre_ParCSRMatrixDestroy(A_ff);
   hypre_ParCSRMatrixDestroy(A_fc);
   hypre_ParCSRMatrixDestroy(A_cf);

   // perform dropping for A_h_correction
   // specific to multiphase poromechanics
   // we only keep the diagonal of each block
   HYPRE_Int n_local_cpoints = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A_h_correction));

   hypre_CSRMatrix *A_h_correction_diag = hypre_ParCSRMatrixDiag(A_h_correction);
   HYPRE_Real      *A_h_correction_diag_data = hypre_CSRMatrixData(A_h_correction_diag);
   HYPRE_Int             *A_h_correction_diag_i = hypre_CSRMatrixI(A_h_correction_diag);
   HYPRE_Int             *A_h_correction_diag_j = hypre_CSRMatrixJ(A_h_correction_diag);
   HYPRE_Int             ncol_diag = hypre_CSRMatrixNumCols(A_h_correction_diag);

   hypre_CSRMatrix *A_h_correction_offd = hypre_ParCSRMatrixOffd(A_h_correction);
   HYPRE_Real      *A_h_correction_offd_data = hypre_CSRMatrixData(A_h_correction_offd);
   HYPRE_Int             *A_h_correction_offd_i = hypre_CSRMatrixI(A_h_correction_offd);
   HYPRE_Int             *A_h_correction_offd_j = hypre_CSRMatrixJ(A_h_correction_offd);

   // drop small entries in the correction
   if (Pmax > 0)
   {
      if (ordering == 0) // interleaved ordering
      {
         HYPRE_Int *A_h_correction_diag_i_new = hypre_CTAlloc(HYPRE_Int, n_local_cpoints + 1,
                                                              memory_location);
         HYPRE_Int *A_h_correction_diag_j_new = hypre_CTAlloc(HYPRE_Int,
                                                              (bsize + max_elmts) * n_local_cpoints, memory_location);
         HYPRE_Complex *A_h_correction_diag_data_new = hypre_CTAlloc(HYPRE_Complex,
                                                                     (bsize + max_elmts) * n_local_cpoints, memory_location);
         HYPRE_Int num_nonzeros_diag_new = 0;

         HYPRE_Int *A_h_correction_offd_i_new = hypre_CTAlloc(HYPRE_Int, n_local_cpoints + 1,
                                                              memory_location);
         HYPRE_Int *A_h_correction_offd_j_new = hypre_CTAlloc(HYPRE_Int, max_elmts * n_local_cpoints,
                                                              memory_location);
         HYPRE_Complex *A_h_correction_offd_data_new = hypre_CTAlloc(HYPRE_Complex,
                                                                     max_elmts * n_local_cpoints, memory_location);
         HYPRE_Int num_nonzeros_offd_new = 0;


         for (i = 0; i < n_local_cpoints; i++)
         {
            HYPRE_Int max_num_nonzeros = A_h_correction_diag_i[i + 1] - A_h_correction_diag_i[i] +
                                         A_h_correction_offd_i[i + 1] - A_h_correction_offd_i[i];
            HYPRE_Int *aux_j = hypre_CTAlloc(HYPRE_Int, max_num_nonzeros, memory_location);
            HYPRE_Real *aux_data = hypre_CTAlloc(HYPRE_Real, max_num_nonzeros, memory_location);
            HYPRE_Int row_start = i - (i % bsize);
            HYPRE_Int row_stop = row_start + bsize - 1;
            HYPRE_Int cnt = 0;
            for (jj = A_h_correction_offd_i[i]; jj < A_h_correction_offd_i[i + 1]; jj++)
            {
               aux_j[cnt] = A_h_correction_offd_j[jj] + ncol_diag;
               aux_data[cnt] = A_h_correction_offd_data[jj];
               cnt++;
            }
            for (jj = A_h_correction_diag_i[i]; jj < A_h_correction_diag_i[i + 1]; jj++)
            {
               aux_j[cnt] = A_h_correction_diag_j[jj];
               aux_data[cnt] = A_h_correction_diag_data[jj];
               cnt++;
            }
            hypre_qsort2_abs(aux_j, aux_data, 0, cnt - 1);

            for (jj = A_h_correction_diag_i[i]; jj < A_h_correction_diag_i[i + 1]; jj++)
            {
               i1 = A_h_correction_diag_j[jj];
               if (i1 >= row_start && i1 <= row_stop)
               {
                  // copy data to new arrays
                  A_h_correction_diag_j_new[num_nonzeros_diag_new] = i1;
                  A_h_correction_diag_data_new[num_nonzeros_diag_new] = A_h_correction_diag_data[jj];
                  ++num_nonzeros_diag_new;
               }
               else
               {
                  // Do nothing
               }
            }

            if (max_elmts > 0)
            {
               for (jj = 0; jj < hypre_min(max_elmts, cnt); jj++)
               {
                  HYPRE_Int col_idx = aux_j[jj];
                  HYPRE_Real col_value = aux_data[jj];
                  if (col_idx < ncol_diag && (col_idx < row_start || col_idx > row_stop))
                  {
                     A_h_correction_diag_j_new[num_nonzeros_diag_new] = col_idx;
                     A_h_correction_diag_data_new[num_nonzeros_diag_new] = col_value;
                     ++num_nonzeros_diag_new;
                  }
                  else if (col_idx >= ncol_diag)
                  {
                     A_h_correction_offd_j_new[num_nonzeros_offd_new] = col_idx - ncol_diag;
                     A_h_correction_offd_data_new[num_nonzeros_offd_new] = col_value;
                     ++num_nonzeros_offd_new;
                  }
               }
            }
            A_h_correction_diag_i_new[i + 1] = num_nonzeros_diag_new;
            A_h_correction_offd_i_new[i + 1] = num_nonzeros_offd_new;

            hypre_TFree(aux_j, memory_location);
            hypre_TFree(aux_data, memory_location);
         }

         hypre_TFree(A_h_correction_diag_i, memory_location);
         hypre_TFree(A_h_correction_diag_j, memory_location);
         hypre_TFree(A_h_correction_diag_data, memory_location);
         hypre_CSRMatrixI(A_h_correction_diag) = A_h_correction_diag_i_new;
         hypre_CSRMatrixJ(A_h_correction_diag) = A_h_correction_diag_j_new;
         hypre_CSRMatrixData(A_h_correction_diag) = A_h_correction_diag_data_new;
         hypre_CSRMatrixNumNonzeros(A_h_correction_diag) = num_nonzeros_diag_new;

         if (A_h_correction_offd_i) { hypre_TFree(A_h_correction_offd_i, memory_location); }
         if (A_h_correction_offd_j) { hypre_TFree(A_h_correction_offd_j, memory_location); }
         if (A_h_correction_offd_data) { hypre_TFree(A_h_correction_offd_data, memory_location); }
         hypre_CSRMatrixI(A_h_correction_offd) = A_h_correction_offd_i_new;
         hypre_CSRMatrixJ(A_h_correction_offd) = A_h_correction_offd_j_new;
         hypre_CSRMatrixData(A_h_correction_offd) = A_h_correction_offd_data_new;
         hypre_CSRMatrixNumNonzeros(A_h_correction_offd) = num_nonzeros_offd_new;
      }
      else
      {
         // do nothing. Dropping not yet implemented for non-interleaved variable ordering options
         //  hypre_printf("Error!! Block ordering for non-Galerkin coarse grid is not currently supported\n");
         //  exit(-1);
      }
   }
   // coarse grid / schur complement
   alpha = -1;
   hypre_ParCSRMatrixAdd(1.0, A_cc, alpha, A_h_correction, &A_h);
   *A_h_ptr = A_h;
   hypre_ParCSRMatrixDestroy(A_cc);
   hypre_ParCSRMatrixDestroy(A_h_correction);
   hypre_TFree(c_marker, memory_location);
   hypre_TFree(f_marker, memory_location);
   // free IntArray. Note: IntArrayData was not initialized so need not be freed here (pointer is already freed elsewhere).
   hypre_TFree(marker_array, memory_location);

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRComputeAlgebraicFixedStress(hypre_ParCSRMatrix  *A,
                                     HYPRE_BigInt        *mgr_idx_array,
                                     HYPRE_Solver         A_ff_solver)
{
   HYPRE_Int *U_marker, *S_marker, *P_marker;
   HYPRE_Int n_fine, i;
   HYPRE_BigInt ibegin;
   hypre_ParCSRMatrix *A_up;
   hypre_ParCSRMatrix *A_uu;
   hypre_ParCSRMatrix *A_su;
   hypre_ParCSRMatrix *A_pu;
   hypre_ParVector *e1_vector;
   hypre_ParVector *e2_vector;
   hypre_ParVector *e3_vector;
   hypre_ParVector *e4_vector;
   hypre_ParVector *e5_vector;

   n_fine = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));
   ibegin = hypre_ParCSRMatrixFirstRowIndex(A);
   hypre_assert(ibegin == mgr_idx_array[0]);
   U_marker = hypre_CTAlloc(HYPRE_Int, n_fine, HYPRE_MEMORY_HOST);
   S_marker = hypre_CTAlloc(HYPRE_Int, n_fine, HYPRE_MEMORY_HOST);
   P_marker = hypre_CTAlloc(HYPRE_Int, n_fine, HYPRE_MEMORY_HOST);

   for (i = 0; i < n_fine; i++)
   {
      U_marker[i] = -1;
      S_marker[i] = -1;
      P_marker[i] = -1;
   }

   // create C and F markers
   for (i = 0; i < n_fine; i++)
   {
      if (i < mgr_idx_array[1] - ibegin)
      {
         U_marker[i] = 1;
      }
      else if (i >= (mgr_idx_array[1] - ibegin) && i < (mgr_idx_array[2] - ibegin))
      {
         S_marker[i] = 1;
      }
      else
      {
         P_marker[i] = 1;
      }
   }

   // Get A_up
   hypre_MGRGetSubBlock(A, U_marker, P_marker, 0, &A_up);
   // GetA_uu
   hypre_MGRGetSubBlock(A, U_marker, U_marker, 0, &A_uu);
   // Get A_su
   hypre_MGRGetSubBlock(A, S_marker, U_marker, 0, &A_su);
   // Get A_pu
   hypre_MGRGetSubBlock(A, P_marker, U_marker, 0, &A_pu);

   e1_vector = hypre_ParVectorCreate(hypre_ParCSRMatrixComm(A_up),
                                     hypre_ParCSRMatrixGlobalNumCols(A_up),
                                     hypre_ParCSRMatrixColStarts(A_up));
   hypre_ParVectorInitialize(e1_vector);
   hypre_ParVectorSetConstantValues(e1_vector, 1.0);

   e2_vector = hypre_ParVectorCreate(hypre_ParCSRMatrixComm(A_uu),
                                     hypre_ParCSRMatrixGlobalNumRows(A_uu),
                                     hypre_ParCSRMatrixRowStarts(A_uu));
   hypre_ParVectorInitialize(e2_vector);
   hypre_ParVectorSetConstantValues(e2_vector, 0.0);

   e3_vector = hypre_ParVectorCreate(hypre_ParCSRMatrixComm(A_uu),
                                     hypre_ParCSRMatrixGlobalNumRows(A_uu),
                                     hypre_ParCSRMatrixRowStarts(A_uu));
   hypre_ParVectorInitialize(e3_vector);
   hypre_ParVectorSetConstantValues(e3_vector, 0.0);

   e4_vector = hypre_ParVectorCreate(hypre_ParCSRMatrixComm(A_su),
                                     hypre_ParCSRMatrixGlobalNumRows(A_su),
                                     hypre_ParCSRMatrixRowStarts(A_su));
   hypre_ParVectorInitialize(e4_vector);
   hypre_ParVectorSetConstantValues(e4_vector, 0.0);

   e5_vector = hypre_ParVectorCreate(hypre_ParCSRMatrixComm(A_pu),
                                     hypre_ParCSRMatrixGlobalNumRows(A_pu),
                                     hypre_ParCSRMatrixRowStarts(A_pu));
   hypre_ParVectorInitialize(e5_vector);
   hypre_ParVectorSetConstantValues(e5_vector, 0.0);

   // compute e2 = A_up * e1
   hypre_ParCSRMatrixMatvecOutOfPlace(1.0, A_up, e1_vector, 0.0, e2_vector, e2_vector);

   // solve e3 = A_uu^-1 * e2
   hypre_BoomerAMGSolve(A_ff_solver, A_uu, e2_vector, e3_vector);

   // compute e4 = A_su * e3
   hypre_ParCSRMatrixMatvecOutOfPlace(1.0, A_su, e3_vector, 0.0, e4_vector, e4_vector);

   // compute e4 = A_su * e3
   hypre_ParCSRMatrixMatvecOutOfPlace(1.0, A_su, e3_vector, 0.0, e4_vector, e4_vector);

   // print e4
   hypre_ParVectorPrintIJ(e4_vector, 1, "Dsp");

   // compute e5 = A_pu * e3
   hypre_ParCSRMatrixMatvecOutOfPlace(1.0, A_pu, e3_vector, 0.0, e5_vector, e5_vector);

   hypre_ParVectorPrintIJ(e5_vector, 1, "Dpp");

   hypre_ParVectorDestroy(e1_vector);
   hypre_ParVectorDestroy(e2_vector);
   hypre_ParVectorDestroy(e3_vector);
   hypre_ParCSRMatrixDestroy(A_uu);
   hypre_ParCSRMatrixDestroy(A_up);
   hypre_ParCSRMatrixDestroy(A_pu);
   hypre_ParCSRMatrixDestroy(A_su);
   hypre_TFree(U_marker, HYPRE_MEMORY_HOST);
   hypre_TFree(S_marker, HYPRE_MEMORY_HOST);
   hypre_TFree(P_marker, HYPRE_MEMORY_HOST);

   return hypre_error_flag;
}


HYPRE_Int
hypre_MGRApproximateInverse(hypre_ParCSRMatrix      *A,
                            hypre_ParCSRMatrix     **A_inv)
{
   HYPRE_Int print_level, mr_max_row_nnz, mr_max_iter, nsh_max_row_nnz, nsh_max_iter, mr_col_version;
   HYPRE_Real mr_tol, nsh_tol;
   HYPRE_Real *droptol = hypre_CTAlloc(HYPRE_Real, 2, HYPRE_MEMORY_HOST);
   hypre_ParCSRMatrix *approx_A_inv = NULL;

   print_level = 0;
   nsh_max_iter = 2;
   nsh_max_row_nnz = 2; // default 1000
   mr_max_iter = 1;
   mr_tol = 1.0e-3;
   mr_max_row_nnz = 2; // default 800
   mr_col_version = 0;
   nsh_tol = 1.0e-3;
   droptol[0] = 1.0e-2;
   droptol[1] = 1.0e-2;

   hypre_ILUParCSRInverseNSH(A, &approx_A_inv, droptol, mr_tol, nsh_tol, HYPRE_REAL_MIN,
                             mr_max_row_nnz,
                             nsh_max_row_nnz, mr_max_iter, nsh_max_iter, mr_col_version, print_level);
   *A_inv = approx_A_inv;

   if (droptol) { hypre_TFree(droptol, HYPRE_MEMORY_HOST); }

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRBuildInterpApproximateInverse(hypre_ParCSRMatrix   *A,
                                       HYPRE_Int            *CF_marker,
                                       HYPRE_BigInt            *num_cpts_global,
                                       HYPRE_Int            debug_flag,
                                       hypre_ParCSRMatrix   **P_ptr)
{
   HYPRE_Int            *C_marker;
   HYPRE_Int            *F_marker;
   hypre_ParCSRMatrix   *A_ff;
   hypre_ParCSRMatrix   *A_fc;
   hypre_ParCSRMatrix   *A_ff_inv;
   hypre_ParCSRMatrix   *W;
   MPI_Comm        comm = hypre_ParCSRMatrixComm(A);
   hypre_ParCSRMatrix    *P;
   HYPRE_BigInt         *col_map_offd_P;
   HYPRE_Real      *P_diag_data;
   HYPRE_Int             *P_diag_i;
   HYPRE_Int             *P_diag_j;
   HYPRE_Int             *P_offd_i;
   HYPRE_Int              P_diag_nnz;
   HYPRE_Int              n_fine = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));
   HYPRE_BigInt              total_global_cpts;
   HYPRE_Int              num_cols_P_offd;

   HYPRE_Int              i;

   HYPRE_Real      m_one = -1.0;

   HYPRE_Int              my_id;
   HYPRE_Int              num_procs;

   HYPRE_MemoryLocation memory_location_P = hypre_ParCSRMatrixMemoryLocation(A);

   C_marker = hypre_CTAlloc(HYPRE_Int, n_fine, HYPRE_MEMORY_HOST);
   F_marker = hypre_CTAlloc(HYPRE_Int, n_fine, HYPRE_MEMORY_HOST);

   // create C and F markers
   for (i = 0; i < n_fine; i++)
   {
      C_marker[i] = (CF_marker[i] == 1) ? 1 : -1;
      F_marker[i] = (CF_marker[i] == 1) ? -1 : 1;
   }

   // Get A_FF
   hypre_MGRGetSubBlock(A, F_marker, F_marker, 0, &A_ff);
   //  hypre_ParCSRMatrixPrintIJ(A_ff, 1, 1, "A_ff");
   // Get A_FC
   hypre_MGRGetSubBlock(A, F_marker, C_marker, 0, &A_fc);

   hypre_MGRApproximateInverse(A_ff, &A_ff_inv);
   //  hypre_ParCSRMatrixPrintIJ(A_ff_inv, 1, 1, "A_ff_inv");
   //  hypre_ParCSRMatrixPrintIJ(A_fc, 1, 1, "A_fc");
   W = hypre_ParMatmul(A_ff_inv, A_fc);
   hypre_ParCSRMatrixScale(W, m_one);
   //  hypre_ParCSRMatrixPrintIJ(W, 1, 1, "Wp");

   hypre_CSRMatrix *W_diag = hypre_ParCSRMatrixDiag(W);

   hypre_CSRMatrix *W_offd         = hypre_ParCSRMatrixOffd(W);

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);

   if (my_id == (num_procs - 1)) { total_global_cpts = num_cpts_global[1]; }
   hypre_MPI_Bcast(&total_global_cpts, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);

   /*-----------------------------------------------------------------------
    *  Allocate  arrays.
    *-----------------------------------------------------------------------*/

   P_diag_nnz  = hypre_CSRMatrixNumNonzeros(W_diag) + hypre_CSRMatrixNumCols(W_diag);
   P_diag_i    = hypre_CTAlloc(HYPRE_Int,  n_fine + 1, memory_location_P);
   P_diag_j    = hypre_CTAlloc(HYPRE_Int,  P_diag_nnz, memory_location_P);
   P_diag_data = hypre_CTAlloc(HYPRE_Real,  P_diag_nnz, memory_location_P);
   P_offd_i    = hypre_CTAlloc(HYPRE_Int,  n_fine + 1, memory_location_P);

   /* Extend W data to P data */
   hypre_ExtendWtoPHost( n_fine,
                         CF_marker,
                         hypre_CSRMatrixI(W_diag),
                         hypre_CSRMatrixJ(W_diag),
                         hypre_CSRMatrixData(W_diag),
                         P_diag_i,
                         P_diag_j,
                         P_diag_data,
                         hypre_CSRMatrixI(W_offd),
                         P_offd_i );
   // final P
   P = hypre_ParCSRMatrixCreate(comm,
                                hypre_ParCSRMatrixGlobalNumRows(A),
                                total_global_cpts,
                                hypre_ParCSRMatrixColStarts(A),
                                num_cpts_global,
                                hypre_CSRMatrixNumCols(W_offd),
                                P_diag_nnz,
                                hypre_CSRMatrixNumNonzeros(W_offd) );

   hypre_CSRMatrixMemoryLocation(hypre_ParCSRMatrixDiag(P)) = memory_location_P;
   hypre_CSRMatrixMemoryLocation(hypre_ParCSRMatrixOffd(P)) = memory_location_P;

   hypre_CSRMatrixI(hypre_ParCSRMatrixDiag(P))    = P_diag_i;
   hypre_CSRMatrixJ(hypre_ParCSRMatrixDiag(P))    = P_diag_j;
   hypre_CSRMatrixData(hypre_ParCSRMatrixDiag(P)) = P_diag_data;

   hypre_CSRMatrixI(hypre_ParCSRMatrixOffd(P))    = P_offd_i;
   hypre_CSRMatrixJ(hypre_ParCSRMatrixOffd(P))    = hypre_CSRMatrixJ(W_offd);
   hypre_CSRMatrixData(hypre_ParCSRMatrixOffd(P)) = hypre_CSRMatrixData(W_offd);
   hypre_CSRMatrixJ(W_offd)    = NULL;
   hypre_CSRMatrixData(W_offd) = NULL;

   num_cols_P_offd = hypre_CSRMatrixNumCols(W_offd);
   HYPRE_BigInt *col_map_offd_tmp = hypre_ParCSRMatrixColMapOffd(W);
   if (hypre_CSRMatrixNumNonzeros(hypre_ParCSRMatrixOffd(P)))
   {
      col_map_offd_P = hypre_CTAlloc(HYPRE_BigInt, num_cols_P_offd, HYPRE_MEMORY_HOST);
      for (i = 0; i < num_cols_P_offd; i++)
      {
         col_map_offd_P[i] = col_map_offd_tmp[i];
      }
   }

   if (num_cols_P_offd)
   {
      hypre_ParCSRMatrixColMapOffd(P) = col_map_offd_P;
      hypre_CSRMatrixNumCols(hypre_ParCSRMatrixOffd(P)) = num_cols_P_offd;
   }
   hypre_MatvecCommPkgCreate(P);

   *P_ptr = P;

   hypre_TFree(C_marker, HYPRE_MEMORY_HOST);
   hypre_TFree(F_marker, HYPRE_MEMORY_HOST);
   hypre_ParCSRMatrixDestroy(A_ff);
   hypre_ParCSRMatrixDestroy(A_fc);
   hypre_ParCSRMatrixDestroy(A_ff_inv);
   hypre_ParCSRMatrixDestroy(W);

   return 0;
}

/* Setup interpolation operator */
HYPRE_Int
hypre_MGRBuildInterp(hypre_ParCSRMatrix   *A,
                     HYPRE_Int            *CF_marker,
                     hypre_ParCSRMatrix   *aux_mat,
                     HYPRE_BigInt         *num_cpts_global,
                     HYPRE_Int             num_functions,
                     HYPRE_Int            *dof_func,
                     HYPRE_Int             debug_flag,
                     HYPRE_Real            trunc_factor,
                     HYPRE_Int             max_elmts,
                     HYPRE_Int             blk_size,
                     hypre_ParCSRMatrix   **P,
                     HYPRE_Int             interp_type,
                     HYPRE_Int             numsweeps)
{
   //  HYPRE_Int i;
   hypre_ParCSRMatrix    *P_ptr = NULL;
   //HYPRE_Real       jac_trunc_threshold = trunc_factor;
   //HYPRE_Real       jac_trunc_threshold_minus = 0.5*jac_trunc_threshold;
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
   HYPRE_ExecutionPolicy exec = hypre_GetExecPolicy1( hypre_ParCSRMatrixMemoryLocation(A) );
#endif

   /* Interpolation for each level */
   if (interp_type < 3)
   {
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
      if (exec == HYPRE_EXEC_DEVICE)
      {
         hypre_MGRBuildPDevice(A, CF_marker, num_cpts_global, interp_type, &P_ptr);
         //hypre_ParCSRMatrixPrintIJ(P_ptr, 0, 0, "P_device");
      }
      else
#endif
      {
         //      hypre_MGRBuildP(A, CF_marker, num_cpts_global, interp_type, debug_flag, &P_ptr);
         hypre_MGRBuildPHost(A, CF_marker, num_cpts_global, interp_type, &P_ptr);
         //hypre_ParCSRMatrixPrintIJ(P_ptr, 0, 0, "P_host");
      }
      /* Could do a few sweeps of Jacobi to further improve Jacobi interpolation P */
      /*
          if(interp_type == 2)
          {
             for(i=0; i<numsweeps; i++)
             {
               hypre_BoomerAMGJacobiInterp(A, &P_ptr, S,1, NULL, CF_marker, 0, jac_trunc_threshold, jac_trunc_threshold_minus );
             }
             hypre_BoomerAMGInterpTruncation(P_ptr, trunc_factor, max_elmts);
          }
      */
   }
   else if (interp_type == 4)
   {
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
      if (exec == HYPRE_EXEC_DEVICE)
      {
         hypre_NoGPUSupport("interpolation");
      }
      else
#endif
      {
         hypre_MGRBuildInterpApproximateInverse(A, CF_marker, num_cpts_global, debug_flag, &P_ptr);
         hypre_BoomerAMGInterpTruncation(P_ptr, trunc_factor, max_elmts);
      }
   }
   else if (interp_type == 5)
   {
      hypre_BoomerAMGBuildModExtInterp(A, CF_marker, aux_mat, num_cpts_global, 1, NULL, debug_flag,
                                       trunc_factor, max_elmts, &P_ptr);
   }
   else if (interp_type == 6)
   {
      hypre_BoomerAMGBuildModExtPIInterp(A, CF_marker, aux_mat, num_cpts_global, 1, NULL, debug_flag,
                                         trunc_factor, max_elmts, &P_ptr);
   }
   else if (interp_type == 7)
   {
      hypre_BoomerAMGBuildModExtPEInterp(A, CF_marker, aux_mat, num_cpts_global, 1, NULL, debug_flag,
                                         trunc_factor, max_elmts, &P_ptr);
   }
   else if (interp_type == 12)
   {
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
      if (exec == HYPRE_EXEC_DEVICE)
      {
         hypre_NoGPUSupport("interpolation");
      }
      else
#endif
      {
         hypre_MGRBuildPBlockJacobi(A, aux_mat, blk_size, CF_marker, num_cpts_global, debug_flag, &P_ptr);
      }
   }
   else
   {
      /* Classical modified interpolation */
      hypre_BoomerAMGBuildInterp(A, CF_marker, aux_mat, num_cpts_global, 1, NULL, debug_flag,
                                 trunc_factor, max_elmts, &P_ptr);
   }

   /* set pointer to P */
   *P = P_ptr;

   return hypre_error_flag;
}

/* Setup restriction operator */
HYPRE_Int
hypre_MGRBuildRestrict(hypre_ParCSRMatrix     *A,
                       HYPRE_Int              *CF_marker,
                       HYPRE_BigInt           *num_cpts_global,
                       HYPRE_Int              num_functions,
                       HYPRE_Int              *dof_func,
                       HYPRE_Int              debug_flag,
                       HYPRE_Real             trunc_factor,
                       HYPRE_Int              max_elmts,
                       HYPRE_Real             strong_threshold,
                       HYPRE_Real             max_row_sum,
                       HYPRE_Int    blk_size,
                       hypre_ParCSRMatrix     **R,
                       HYPRE_Int              restrict_type,
                       HYPRE_Int              numsweeps)
{
   //   HYPRE_Int i;
   hypre_ParCSRMatrix    *R_ptr = NULL;
   hypre_ParCSRMatrix    *AT = NULL;
   hypre_ParCSRMatrix    *ST = NULL;
   //   HYPRE_Real       jac_trunc_threshold = trunc_factor;
   //   HYPRE_Real       jac_trunc_threshold_minus = 0.5*jac_trunc_threshold;
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
   HYPRE_ExecutionPolicy exec = hypre_GetExecPolicy1( hypre_ParCSRMatrixMemoryLocation(A) );
#endif

   /* Build AT (transpose A) */
   if (restrict_type > 0)
   {
      hypre_ParCSRMatrixTranspose(A, &AT, 1);
   }

   /* Restriction for each level */
   if (restrict_type == 0)
   {
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
      if (exec == HYPRE_EXEC_DEVICE)
      {
         hypre_MGRBuildPDevice(A, CF_marker, num_cpts_global, restrict_type, &R_ptr);
         //hypre_ParCSRMatrixPrintIJ(R_ptr, 0, 0, "R_device");
      }
      else
#endif
      {
         hypre_MGRBuildP(A, CF_marker, num_cpts_global, restrict_type, debug_flag, &R_ptr);
         //hypre_ParCSRMatrixPrintIJ(R_ptr, 0, 0, "R_host");
      }
   }
   else if (restrict_type == 1 || restrict_type == 2)
   {
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
      if (exec == HYPRE_EXEC_DEVICE)
      {
         hypre_MGRBuildPDevice(AT, CF_marker, num_cpts_global, restrict_type, &R_ptr);
         //hypre_ParCSRMatrixPrintIJ(R_ptr, 0, 0, "R_device");
      }
      else
#endif
      {
         hypre_MGRBuildP(AT, CF_marker, num_cpts_global, restrict_type, debug_flag, &R_ptr);
         //hypre_ParCSRMatrixPrintIJ(R_ptr, 0, 0, "R_host");
      }
   }
   else if (restrict_type == 3)
   {
      /* move diagonal to first entry */
      hypre_CSRMatrixReorder(hypre_ParCSRMatrixDiag(AT));
      hypre_MGRBuildInterpApproximateInverse(AT, CF_marker, num_cpts_global, debug_flag, &R_ptr);
      hypre_BoomerAMGInterpTruncation(R_ptr, trunc_factor, max_elmts);
   }
   else if (restrict_type == 12)
   {
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
      if (exec == HYPRE_EXEC_DEVICE)
      {
         hypre_NoGPUSupport("restriction");
      }
      else
#endif
      {
         hypre_MGRBuildPBlockJacobi(AT, NULL, blk_size, CF_marker, num_cpts_global, debug_flag, &R_ptr);
      }
   }
   else if (restrict_type == 13) // CPR-like restriction operator
   {
      hypre_ParCSRMatrix *blk_A_cf = NULL;
      hypre_ParCSRMatrix *blk_A_cf_transpose = NULL;
      hypre_ParCSRMatrix *Wr_transpose = NULL;
      hypre_ParCSRMatrix *blk_A_ff_inv_transpose = NULL;
      HYPRE_Int *c_marker = NULL;
      HYPRE_Int *f_marker = NULL;
      HYPRE_Int i;
      HYPRE_Int nrows = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));

      HYPRE_MemoryLocation memory_location = hypre_ParCSRMatrixMemoryLocation(A);

      /* create C and F markers to extract A_CF */
      c_marker = CF_marker;
      f_marker = hypre_CTAlloc(HYPRE_Int, nrows, memory_location);
      for (i = 0; i < nrows; i++)
      {
         HYPRE_Int point_type = CF_marker[i];
         f_marker[i] = -point_type;
      }
#if defined (HYPRE_USING_CUDA) || defined (HYPRE_USING_HIP)
      if (exec == HYPRE_EXEC_DEVICE)
      {
         hypre_NoGPUSupport("restriction");
      }
      else
#endif
      {
         /* get block A_cf */
         hypre_MGRGetAcfCPR(A, blk_size, c_marker, f_marker, &blk_A_cf);
         /* transpose block A_cf */
         hypre_ParCSRMatrixTranspose(blk_A_cf, &blk_A_cf_transpose, 1);
         /* compute block diagonal A_ff */
         hypre_ParCSRMatrixBlockDiagMatrix(AT, blk_size, -1, CF_marker, &blk_A_ff_inv_transpose, 1);
         /* compute  Wr = A^{-T} * A_cf^{T}  */
         Wr_transpose = hypre_ParCSRMatMat(blk_A_ff_inv_transpose, blk_A_cf_transpose);
         /* compute restriction operator R = [-Wr  I] (transposed for use with RAP) */
         hypre_MGRBuildPFromWp(AT, Wr_transpose, CF_marker, debug_flag, &R_ptr);
      }
      hypre_ParCSRMatrixDestroy(blk_A_cf);
      hypre_ParCSRMatrixDestroy(blk_A_cf_transpose);
      hypre_ParCSRMatrixDestroy(Wr_transpose);
      hypre_ParCSRMatrixDestroy(blk_A_ff_inv_transpose);
      hypre_TFree(f_marker, memory_location);
   }
   else
   {
      /* Build new strength matrix */
      hypre_BoomerAMGCreateS(AT, strong_threshold, max_row_sum, 1, NULL, &ST);

      /* Classical modified interpolation */
      hypre_BoomerAMGBuildInterp(AT, CF_marker, ST, num_cpts_global, 1, NULL, debug_flag,
                                 trunc_factor, max_elmts, &R_ptr);
   }

   /* set pointer to P */
   *R = R_ptr;

   /* Free memory */
   if (restrict_type > 0)
   {
      hypre_ParCSRMatrixDestroy(AT);
   }
   if (restrict_type > 5)
   {
      hypre_ParCSRMatrixDestroy(ST);
   }

   return hypre_error_flag;
}

void hypre_blas_smat_inv_n2 (HYPRE_Real *a)
{
   const HYPRE_Real a11 = a[0], a12 = a[1];
   const HYPRE_Real a21 = a[2], a22 = a[3];
   const HYPRE_Real det_inv = 1.0 / (a11 * a22 - a12 * a21);
   a[0] = a22 * det_inv; a[1] = -a12 * det_inv;
   a[2] = -a21 * det_inv; a[3] = a11 * det_inv;
}

void hypre_blas_smat_inv_n3 (HYPRE_Real *a)
{
   const HYPRE_Real a11 = a[0],  a12 = a[1],  a13 = a[2];
   const HYPRE_Real a21 = a[3],  a22 = a[4],  a23 = a[5];
   const HYPRE_Real a31 = a[6],  a32 = a[7],  a33 = a[8];

   const HYPRE_Real det = a11 * a22 * a33 - a11 * a23 * a32 - a12 * a21 * a33 + a12 * a23 * a31 + a13 *
                          a21 * a32 - a13 * a22 * a31;
   const HYPRE_Real det_inv = 1.0 / det;

   a[0] = (a22 * a33 - a23 * a32) * det_inv; a[1] = (a13 * a32 - a12 * a33) * det_inv;
   a[2] = (a12 * a23 - a13 * a22) * det_inv;
   a[3] = (a23 * a31 - a21 * a33) * det_inv; a[4] = (a11 * a33 - a13 * a31) * det_inv;
   a[5] = (a13 * a21 - a11 * a23) * det_inv;
   a[6] = (a21 * a32 - a22 * a31) * det_inv; a[7] = (a12 * a31 - a11 * a32) * det_inv;
   a[8] = (a11 * a22 - a12 * a21) * det_inv;
}

void hypre_blas_smat_inv_n4 (HYPRE_Real *a)
{
   const HYPRE_Real a11 = a[0],  a12 = a[1],  a13 = a[2],  a14 = a[3];
   const HYPRE_Real a21 = a[4],  a22 = a[5],  a23 = a[6],  a24 = a[7];
   const HYPRE_Real a31 = a[8],  a32 = a[9],  a33 = a[10], a34 = a[11];
   const HYPRE_Real a41 = a[12], a42 = a[13], a43 = a[14], a44 = a[15];

   const HYPRE_Real M11 = a22 * a33 * a44 + a23 * a34 * a42 + a24 * a32 * a43 - a22 * a34 * a43 - a23 *
                          a32 * a44 - a24 * a33 * a42;
   const HYPRE_Real M12 = a12 * a34 * a43 + a13 * a32 * a44 + a14 * a33 * a42 - a12 * a33 * a44 - a13 *
                          a34 * a42 - a14 * a32 * a43;
   const HYPRE_Real M13 = a12 * a23 * a44 + a13 * a24 * a42 + a14 * a22 * a43 - a12 * a24 * a43 - a13 *
                          a22 * a44 - a14 * a23 * a42;
   const HYPRE_Real M14 = a12 * a24 * a33 + a13 * a22 * a34 + a14 * a23 * a32 - a12 * a23 * a34 - a13 *
                          a24 * a32 - a14 * a22 * a33;
   const HYPRE_Real M21 = a21 * a34 * a43 + a23 * a31 * a44 + a24 * a33 * a41 - a21 * a33 * a44 - a23 *
                          a34 * a41 - a24 * a31 * a43;
   const HYPRE_Real M22 = a11 * a33 * a44 + a13 * a34 * a41 + a14 * a31 * a43 - a11 * a34 * a43 - a13 *
                          a31 * a44 - a14 * a33 * a41;
   const HYPRE_Real M23 = a11 * a24 * a43 + a13 * a21 * a44 + a14 * a23 * a41 - a11 * a23 * a44 - a13 *
                          a24 * a41 - a14 * a21 * a43;
   const HYPRE_Real M24 = a11 * a23 * a34 + a13 * a24 * a31 + a14 * a21 * a33 - a11 * a24 * a33 - a13 *
                          a21 * a34 - a14 * a23 * a31;
   const HYPRE_Real M31 = a21 * a32 * a44 + a22 * a34 * a41 + a24 * a31 * a42 - a21 * a34 * a42 - a22 *
                          a31 * a44 - a24 * a32 * a41;
   const HYPRE_Real M32 = a11 * a34 * a42 + a12 * a31 * a44 + a14 * a32 * a41 - a11 * a32 * a44 - a12 *
                          a34 * a41 - a14 * a31 * a42;
   const HYPRE_Real M33 = a11 * a22 * a44 + a12 * a24 * a41 + a14 * a21 * a42 - a11 * a24 * a42 - a12 *
                          a21 * a44 - a14 * a22 * a41;
   const HYPRE_Real M34 = a11 * a24 * a32 + a12 * a21 * a34 + a14 * a22 * a31 - a11 * a22 * a34 - a12 *
                          a24 * a31 - a14 * a21 * a32;
   const HYPRE_Real M41 = a21 * a33 * a42 + a22 * a31 * a43 + a23 * a32 * a41 - a21 * a32 * a43 - a22 *
                          a33 * a41 - a23 * a31 * a42;
   const HYPRE_Real M42 = a11 * a32 * a43 + a12 * a33 * a41 + a13 * a31 * a42 - a11 * a33 * a42 - a12 *
                          a31 * a43 - a13 * a32 * a41;
   const HYPRE_Real M43 = a11 * a23 * a42 + a12 * a21 * a43 + a13 * a22 * a41 - a11 * a22 * a43 - a12 *
                          a23 * a41 - a13 * a21 * a42;
   const HYPRE_Real M44 = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 - a11 * a23 * a32 - a12 *
                          a21 * a33 - a13 * a22 * a31;

   const HYPRE_Real det = a11 * M11 + a12 * M21 + a13 * M31 + a14 * M41;
   HYPRE_Real det_inv;

   //if ( fabs(det) < 1e-22 ) {
   //hypre_printf("### WARNING: Matrix is nearly singular! det = %e\n", det);
   /*
   printf("##----------------------------------------------\n");
   printf("## %12.5e %12.5e %12.5e \n", a0, a1, a2);
   printf("## %12.5e %12.5e %12.5e \n", a3, a4, a5);
   printf("## %12.5e %12.5e %12.5e \n", a5, a6, a7);
   printf("##----------------------------------------------\n");
   getchar();
   */
   //}

   det_inv = 1.0 / det;

   a[0] = M11 * det_inv;  a[1] = M12 * det_inv;  a[2] = M13 * det_inv;  a[3] = M14 * det_inv;
   a[4] = M21 * det_inv;  a[5] = M22 * det_inv;  a[6] = M23 * det_inv;  a[7] = M24 * det_inv;
   a[8] = M31 * det_inv;  a[9] = M32 * det_inv;  a[10] = M33 * det_inv; a[11] = M34 * det_inv;
   a[12] = M41 * det_inv; a[13] = M42 * det_inv; a[14] = M43 * det_inv; a[15] = M44 * det_inv;

}

void hypre_MGRSmallBlkInverse(HYPRE_Real *mat,
                              HYPRE_Int  blk_size)
{
   if (blk_size == 2)
   {
      hypre_blas_smat_inv_n2(mat);
   }
   else if (blk_size == 3)
   {
      hypre_blas_smat_inv_n3(mat);
   }
   else if (blk_size == 4)
   {
      hypre_blas_smat_inv_n4(mat);
   }
}

void hypre_blas_mat_inv(HYPRE_Real *a,
                        HYPRE_Int n)
{
   HYPRE_Int i, j, k, l, u, kn, in;
   HYPRE_Real alinv;
   if (n == 4)
   {
      hypre_blas_smat_inv_n4(a);
   }
   else
   {
      for (k = 0; k < n; ++k)
      {
         kn = k * n;
         l  = kn + k;

         //if (fabs(a[l]) < HYPRE_REAL_MIN) {
         //   printf("### WARNING: Diagonal entry is close to zero!");
         //   printf("### WARNING: diag_%d=%e\n", k, a[l]);
         //   a[l] = HYPRE_REAL_MIN;
         //}
         alinv = 1.0 / a[l];
         a[l] = alinv;

         for (j = 0; j < k; ++j)
         {
            u = kn + j; a[u] *= alinv;
         }

         for (j = k + 1; j < n; ++j)
         {
            u = kn + j; a[u] *= alinv;
         }

         for (i = 0; i < k; ++i)
         {
            in = i * n;
            for (j = 0; j < n; ++j)
               if (j != k)
               {
                  u = in + j; a[u] -= a[in + k] * a[kn + j];
               } // end if (j!=k)
         }

         for (i = k + 1; i < n; ++i)
         {
            in = i * n;
            for (j = 0; j < n; ++j)
               if (j != k)
               {
                  u = in + j; a[u] -= a[in + k] * a[kn + j];
               } // end if (j!=k)
         }

         for (i = 0; i < k; ++i)
         {
            u = i * n + k; a[u] *= -alinv;
         }

         for (i = k + 1; i < n; ++i)
         {
            u = i * n + k; a[u] *= -alinv;
         }
      } // end for (k=0; k<n; ++k)
   }// end if
}

HYPRE_Int hypre_block_jacobi_solve(hypre_ParCSRMatrix *A,
                                   hypre_ParVector    *f,
                                   hypre_ParVector    *u,
                                   HYPRE_Int          blk_size,
                                   HYPRE_Int          method,
                                   HYPRE_Real         *diaginv,
                                   hypre_ParVector    *Vtemp)
{
   MPI_Comm      comm = hypre_ParCSRMatrixComm(A);
   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real      *A_diag_data  = hypre_CSRMatrixData(A_diag);
   HYPRE_Int       *A_diag_i     = hypre_CSRMatrixI(A_diag);
   HYPRE_Int       *A_diag_j     = hypre_CSRMatrixJ(A_diag);
   hypre_CSRMatrix *A_offd = hypre_ParCSRMatrixOffd(A);
   HYPRE_Int       *A_offd_i     = hypre_CSRMatrixI(A_offd);
   HYPRE_Real      *A_offd_data  = hypre_CSRMatrixData(A_offd);
   HYPRE_Int       *A_offd_j     = hypre_CSRMatrixJ(A_offd);
   hypre_ParCSRCommPkg  *comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   hypre_ParCSRCommHandle *comm_handle;

   HYPRE_Int        n       = hypre_CSRMatrixNumRows(A_diag);
   HYPRE_Int        num_cols_offd = hypre_CSRMatrixNumCols(A_offd);

   hypre_Vector    *u_local = hypre_ParVectorLocalVector(u);
   HYPRE_Real      *u_data  = hypre_VectorData(u_local);

   hypre_Vector    *f_local = hypre_ParVectorLocalVector(f);
   HYPRE_Real      *f_data  = hypre_VectorData(f_local);

   hypre_Vector    *Vtemp_local = hypre_ParVectorLocalVector(Vtemp);
   HYPRE_Real      *Vtemp_data = hypre_VectorData(Vtemp_local);
   HYPRE_Real      *Vext_data = NULL;
   HYPRE_Real      *v_buf_data;

   HYPRE_Int        i, j, k;
   HYPRE_Int        ii, jj;
   HYPRE_Int        bidx, bidx1;
   HYPRE_Int        num_sends;
   HYPRE_Int        index, start;
   HYPRE_Int        num_procs, my_id;
   HYPRE_Real      *res;

   const HYPRE_Int  nb2 = blk_size * blk_size;
   const HYPRE_Int  n_block = n / blk_size;

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);
   //   HYPRE_Int num_threads = hypre_NumThreads();

   res = hypre_CTAlloc(HYPRE_Real, blk_size, HYPRE_MEMORY_HOST);

   if (!comm_pkg)
   {
      hypre_MatvecCommPkgCreate(A);
      comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   }

   if (num_procs > 1)
   {
      num_sends = hypre_ParCSRCommPkgNumSends(comm_pkg);

      v_buf_data = hypre_CTAlloc(HYPRE_Real,
                                 hypre_ParCSRCommPkgSendMapStart(comm_pkg,  num_sends), HYPRE_MEMORY_HOST);

      Vext_data = hypre_CTAlloc(HYPRE_Real, num_cols_offd, HYPRE_MEMORY_HOST);

      if (num_cols_offd)
      {
         A_offd_j = hypre_CSRMatrixJ(A_offd);
         A_offd_data = hypre_CSRMatrixData(A_offd);
      }

      index = 0;
      for (i = 0; i < num_sends; i++)
      {
         start = hypre_ParCSRCommPkgSendMapStart(comm_pkg, i);
         for (j = start; j < hypre_ParCSRCommPkgSendMapStart(comm_pkg, i + 1); j++)
            v_buf_data[index++]
               = u_data[hypre_ParCSRCommPkgSendMapElmt(comm_pkg, j)];
      }

      comm_handle = hypre_ParCSRCommHandleCreate( 1, comm_pkg, v_buf_data,
                                                  Vext_data);
   }

   /*-----------------------------------------------------------------
   * Copy current approximation into temporary vector.
   *-----------------------------------------------------------------*/

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (i = 0; i < n; i++)
   {
      Vtemp_data[i] = u_data[i];
      //printf("u_old[%d] = %e\n",i,Vtemp_data[i]);
   }
   if (num_procs > 1)
   {
      hypre_ParCSRCommHandleDestroy(comm_handle);
      comm_handle = NULL;
   }

   /*-----------------------------------------------------------------
   * Relax points block by block
   *-----------------------------------------------------------------*/
   for (i = 0; i < n_block; i++)
   {
      for (j = 0; j < blk_size; j++)
      {
         bidx = i * blk_size + j;
         res[j] = f_data[bidx];
         for (jj = A_diag_i[bidx]; jj < A_diag_i[bidx + 1]; jj++)
         {
            ii = A_diag_j[jj];
            if (method == 0)
            {
               // Jacobi for diagonal part
               res[j] -= A_diag_data[jj] * Vtemp_data[ii];
            }
            else if (method == 1)
            {
               // Gauss-Seidel for diagonal part
               res[j] -= A_diag_data[jj] * u_data[ii];
            }
            else
            {
               // Default do Jacobi for diagonal part
               res[j] -= A_diag_data[jj] * Vtemp_data[ii];
            }
            //printf("%d: Au= %e * %e =%e\n",ii,A_diag_data[jj],Vtemp_data[ii], res[j]);
         }
         for (jj = A_offd_i[bidx]; jj < A_offd_i[bidx + 1]; jj++)
         {
            // always do Jacobi for off-diagonal part
            ii = A_offd_j[jj];
            res[j] -= A_offd_data[jj] * Vext_data[ii];
         }
         //printf("%d: res = %e\n",bidx,res[j]);
      }

      for (j = 0; j < blk_size; j++)
      {
         bidx1 = i * blk_size + j;
         for (k = 0; k < blk_size; k++)
         {
            bidx  = i * nb2 + j * blk_size + k;
            u_data[bidx1] += res[k] * diaginv[bidx];
            //printf("u[%d] = %e, diaginv[%d] = %e\n",bidx1,u_data[bidx1],bidx,diaginv[bidx]);
         }
         //printf("u[%d] = %e\n",bidx1,u_data[bidx1]);
      }
   }

   if (num_procs > 1)
   {
      hypre_TFree(Vext_data, HYPRE_MEMORY_HOST);
      hypre_TFree(v_buf_data, HYPRE_MEMORY_HOST);
   }
   hypre_TFree(res, HYPRE_MEMORY_HOST);
   return hypre_error_flag;
}

/* Computes a block Jacobi relaxation of matrix A, given the inverse of the diagonal blocks (of A) obtained
 * by calling hypre_MGRBlockRelaxSetup.
 * TODO: Adapt to relax on specific points based on CF_marker information
*/
HYPRE_Int hypre_MGRBlockRelaxSolve (hypre_ParCSRMatrix *A,
                                    hypre_ParVector    *f,
                                    hypre_ParVector    *u,
                                    HYPRE_Real         blk_size,
                                    HYPRE_Int           n_block,
                                    HYPRE_Int           left_size,
                                    HYPRE_Int          method,
                                    HYPRE_Real         *diaginv,
                                    hypre_ParVector    *Vtemp)
{
   MPI_Comm      comm = hypre_ParCSRMatrixComm(A);
   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real      *A_diag_data  = hypre_CSRMatrixData(A_diag);
   HYPRE_Int       *A_diag_i     = hypre_CSRMatrixI(A_diag);
   HYPRE_Int       *A_diag_j     = hypre_CSRMatrixJ(A_diag);
   hypre_CSRMatrix *A_offd = hypre_ParCSRMatrixOffd(A);
   HYPRE_Int       *A_offd_i     = hypre_CSRMatrixI(A_offd);
   HYPRE_Real      *A_offd_data  = hypre_CSRMatrixData(A_offd);
   HYPRE_Int       *A_offd_j     = hypre_CSRMatrixJ(A_offd);
   hypre_ParCSRCommPkg  *comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   hypre_ParCSRCommHandle *comm_handle;

   HYPRE_Int        n       = hypre_CSRMatrixNumRows(A_diag);
   HYPRE_Int        num_cols_offd = hypre_CSRMatrixNumCols(A_offd);

   hypre_Vector    *u_local = hypre_ParVectorLocalVector(u);
   HYPRE_Real      *u_data  = hypre_VectorData(u_local);

   hypre_Vector    *f_local = hypre_ParVectorLocalVector(f);
   HYPRE_Real      *f_data  = hypre_VectorData(f_local);

   hypre_Vector    *Vtemp_local = hypre_ParVectorLocalVector(Vtemp);
   HYPRE_Real      *Vtemp_data = hypre_VectorData(Vtemp_local);
   HYPRE_Real      *Vext_data = NULL;
   HYPRE_Real      *v_buf_data;

   HYPRE_Int        i, j, k;
   HYPRE_Int        ii, jj;
   HYPRE_Int        bidx, bidx1, bidxm1;
   HYPRE_Int        num_sends;
   HYPRE_Int        index, start;
   HYPRE_Int        num_procs, my_id;
   HYPRE_Real      *res;

   const HYPRE_Int  nb2 = blk_size * blk_size;

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);
   //   HYPRE_Int num_threads = hypre_NumThreads();

   res = hypre_CTAlloc(HYPRE_Real,  blk_size, HYPRE_MEMORY_HOST);

   if (!comm_pkg)
   {
      hypre_MatvecCommPkgCreate(A);
      comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   }

   if (num_procs > 1)
   {
      num_sends = hypre_ParCSRCommPkgNumSends(comm_pkg);

      v_buf_data = hypre_CTAlloc(HYPRE_Real,
                                 hypre_ParCSRCommPkgSendMapStart(comm_pkg,  num_sends), HYPRE_MEMORY_HOST);

      Vext_data = hypre_CTAlloc(HYPRE_Real, num_cols_offd, HYPRE_MEMORY_HOST);

      if (num_cols_offd)
      {
         A_offd_j = hypre_CSRMatrixJ(A_offd);
         A_offd_data = hypre_CSRMatrixData(A_offd);
      }

      index = 0;
      for (i = 0; i < num_sends; i++)
      {
         start = hypre_ParCSRCommPkgSendMapStart(comm_pkg, i);
         for (j = start; j < hypre_ParCSRCommPkgSendMapStart(comm_pkg, i + 1); j++)
            v_buf_data[index++]
               = u_data[hypre_ParCSRCommPkgSendMapElmt(comm_pkg, j)];
      }

      comm_handle = hypre_ParCSRCommHandleCreate( 1, comm_pkg, v_buf_data,
                                                  Vext_data);
   }

   /*-----------------------------------------------------------------
   * Copy current approximation into temporary vector.
   *-----------------------------------------------------------------*/

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (i = 0; i < n; i++)
   {
      Vtemp_data[i] = u_data[i];
      //printf("u_old[%d] = %e\n",i,Vtemp_data[i]);
   }
   if (num_procs > 1)
   {
      hypre_ParCSRCommHandleDestroy(comm_handle);
      comm_handle = NULL;
   }

   /*-----------------------------------------------------------------
   * Relax points block by block
   *-----------------------------------------------------------------*/
   for (i = 0; i < n_block; i++)
   {
      bidxm1 = i * blk_size;
      for (j = 0; j < blk_size; j++)
      {
         bidx = bidxm1 + j;
         res[j] = f_data[bidx];
         for (jj = A_diag_i[bidx]; jj < A_diag_i[bidx + 1]; jj++)
         {
            ii = A_diag_j[jj];
            if (method == 0)
            {
               // Jacobi for diagonal part
               res[j] -= A_diag_data[jj] * Vtemp_data[ii];
            }
            else if (method == 1)
            {
               // Gauss-Seidel for diagonal part
               res[j] -= A_diag_data[jj] * u_data[ii];
            }
            else
            {
               // Default do Jacobi for diagonal part
               res[j] -= A_diag_data[jj] * Vtemp_data[ii];
            }
            //printf("%d: Au= %e * %e =%e\n",ii,A_diag_data[jj],Vtemp_data[ii], res[j]);
         }
         for (jj = A_offd_i[bidx]; jj < A_offd_i[bidx + 1]; jj++)
         {
            // always do Jacobi for off-diagonal part
            ii = A_offd_j[jj];
            res[j] -= A_offd_data[jj] * Vext_data[ii];
         }
         //printf("%d: res = %e\n",bidx,res[j]);
      }

      for (j = 0; j < blk_size; j++)
      {
         bidx1 = bidxm1 + j;
         for (k = 0; k < blk_size; k++)
         {
            bidx  = i * nb2 + j * blk_size + k;
            u_data[bidx1] += res[k] * diaginv[bidx];
            //printf("u[%d] = %e, diaginv[%d] = %e\n",bidx1,u_data[bidx1],bidx,diaginv[bidx]);
         }
         //printf("u[%d] = %e\n",bidx1,u_data[bidx1]);
      }
   }
   if (num_procs > 1)
   {
      hypre_TFree(Vext_data, HYPRE_MEMORY_HOST);
      hypre_TFree(v_buf_data, HYPRE_MEMORY_HOST);
   }
   hypre_TFree(res, HYPRE_MEMORY_HOST);
   return hypre_error_flag;
}

HYPRE_Int
hypre_BlockDiagInvLapack(HYPRE_Real *diag, HYPRE_Int N, HYPRE_Int blk_size)
{
   HYPRE_Int nblock, left_size, i;
   //HYPRE_Int *IPIV = hypre_CTAlloc(HYPRE_Int, blk_size, HYPRE_MEMORY_HOST);
   HYPRE_Int LWORK = blk_size * blk_size;
   HYPRE_Real *WORK = hypre_CTAlloc(HYPRE_Real, LWORK, HYPRE_MEMORY_HOST);
   HYPRE_Int INFO;

   HYPRE_Real wall_time;
   HYPRE_Int my_id;
   MPI_Comm comm = hypre_MPI_COMM_WORLD;
   hypre_MPI_Comm_rank(comm, &my_id);

   nblock = N / blk_size;
   left_size = N - blk_size * nblock;
   HYPRE_Int *IPIV = hypre_CTAlloc(HYPRE_Int, blk_size, HYPRE_MEMORY_HOST);

   wall_time = time_getWallclockSeconds();
   if (blk_size >= 2 && blk_size <= 4)
   {
      for (i = 0; i < nblock; i++)
      {
         hypre_MGRSmallBlkInverse(diag + i * LWORK, blk_size);
         //hypre_blas_smat_inv_n2(diag+i*LWORK);
      }
   }
   else if (blk_size > 4)
   {
      for (i = 0; i < nblock; i++)
      {
         hypre_dgetrf(&blk_size, &blk_size, diag + i * LWORK, &blk_size, IPIV, &INFO);
         hypre_dgetri(&blk_size, diag + i * LWORK, &blk_size, IPIV, WORK, &LWORK, &INFO);
      }
   }

   // Left size
   if (left_size > 0)
   {
      hypre_dgetrf(&left_size, &left_size, diag + i * LWORK, &left_size, IPIV, &INFO);
      hypre_dgetri(&left_size, diag + i * LWORK, &left_size, IPIV, WORK, &LWORK, &INFO);
   }
   wall_time = time_getWallclockSeconds() - wall_time;
   //if (my_id == 0) hypre_printf("Proc = %d, Compute inverse time: %1.5f\n", my_id, wall_time);

   hypre_TFree(IPIV, HYPRE_MEMORY_HOST);
   hypre_TFree(WORK, HYPRE_MEMORY_HOST);

   return hypre_error_flag;
}
// Extract the block diagonal part of a A or a principal submatrix of A defined by a marker (point_type)
// in an associated CF_marker array. The result is an array of (flattened) block diagonals.
// If CF marker array is NULL, it returns an array of the (flattened)
// block diagonal of the entire matrix A. Options for diag_type are:
// diag_type = 1: return the inverse of the block diagonals
// otherwise : return the block diagonals
// On return, blk_diag_size contains the size of the returned (flattened) array.
// (i.e. nnz of extracted block diagonal)
// ***Input***
// A - parCSR matrix
// blk_size - Size of diagonal blocks to extract
// CF_marker - Array prescribing submatrix from which to extract block diagonals. Ignored if NULL.
// point_type - marker tag in CF_marker array to extract diagonal
// diag_type - Type of block diagonal entries to return. Currently supports block diagonal or inverse block diagonal entries (diag_type = 1).
// ***Output***
// diag_ptr: Array of block diagonal entries
// blk_diag_size - number of entries in extracted block diagonal (size of diag_ptr).
//
HYPRE_Int
hypre_ParCSRMatrixExtractBlockDiag(hypre_ParCSRMatrix   *A,
                                   HYPRE_Int            blk_size,
                                   HYPRE_Int            point_type,
                                   HYPRE_Int            *CF_marker,
                                   HYPRE_Int            *blk_diag_size,
                                   HYPRE_Real           **diag_ptr,
                                   HYPRE_Int            diag_type)
{
   MPI_Comm      comm = hypre_ParCSRMatrixComm(A);
   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real     *A_diag_data  = hypre_CSRMatrixData(A_diag);
   HYPRE_Int            *A_diag_i     = hypre_CSRMatrixI(A_diag);
   HYPRE_Int            *A_diag_j     = hypre_CSRMatrixJ(A_diag);

   HYPRE_Int             i, j;
   HYPRE_Int             ii, jj;
   HYPRE_Int             bidx, bidxm1, bidxp1, ridx, didx;
   HYPRE_Int             num_procs, my_id;
   HYPRE_Int             row_offset;

   HYPRE_Int           num_points, whole_num_points, cnt, bstart;
   const HYPRE_Int     bs2 = blk_size * blk_size;
   HYPRE_Int           n_block;
   HYPRE_Int           left_size = 0, bdiag_size;
   HYPRE_Real          *diag = *diag_ptr;
   HYPRE_Real          wall_time;
   HYPRE_Int           nrows = hypre_CSRMatrixNumRows(A_diag);

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);
   //HYPRE_Int num_threads = hypre_NumThreads();

   HYPRE_MemoryLocation memory_location = HYPRE_MEMORY_HOST; //hypre_ParCSRMatrixMemoryLocation(A);

   // First count the number of points matching point_type in CF_marker
   num_points = 0;
   if (CF_marker == NULL)
   {
      num_points = nrows;
   }
   else
   {
      for (i = 0; i < nrows; i++)
      {
         if (CF_marker[i] == point_type)
         {
            num_points++;
         }
      }
   }
   n_block = num_points / blk_size;
   left_size = num_points - blk_size * n_block;
   whole_num_points = blk_size * n_block;
   bstart = bs2 * n_block;

   bdiag_size  = bstart + left_size * left_size;
   *blk_diag_size = bdiag_size;

   if (diag != NULL)
   {
      hypre_TFree(diag, memory_location);
      diag = hypre_CTAlloc(HYPRE_Real, bdiag_size, memory_location);
   }
   else
   {
      diag = hypre_CTAlloc(HYPRE_Real, bdiag_size, memory_location);
   }

   /*-----------------------------------------------------------------
   * Get all the diagonal sub-blocks
   *-----------------------------------------------------------------*/
   wall_time = time_getWallclockSeconds();
   //CF Marker is NULL. Consider all rows of matrix.
   if (CF_marker == NULL)
   {
      for (i = 0; i < n_block; i++)
      {
         bidxm1 = i * blk_size;
         bidxp1 = (i + 1) * blk_size;

         for (j = 0; j < blk_size; j++)
         {
            for (ii = A_diag_i[bidxm1 + j]; ii < A_diag_i[bidxm1 + j + 1]; ii++)
            {
               jj = A_diag_j[ii];
               if (jj >= bidxm1 && jj < bidxp1 && fabs(A_diag_data[ii]) > HYPRE_REAL_MIN)
               {
                  bidx = j * blk_size + jj - bidxm1;
                  diag[i * bs2 + bidx] = A_diag_data[ii];
               }
            }
         }
      }
      // deal with remaining points if any
      if (left_size)
      {
         bidxm1 = whole_num_points;
         bidxp1 = num_points;
         for ( j = 0; j < left_size; j++)
         {
            for (ii = A_diag_i[bidxm1 + j]; ii < A_diag_i[bidxm1 + j + 1]; ii++)
            {
               jj = A_diag_j[ii];
               if (jj >= bidxm1 && jj < bidxp1 && fabs(A_diag_data[ii]) > HYPRE_REAL_MIN)
               {
                  bidx = j * left_size + jj - bidxm1;
                  diag[bstart + bidx] = A_diag_data[ii];
               }
            }
         }
      }
   }
   // extract only block diagonal of submatrix defined by CF marker
   else
   {
      cnt = 0;
      row_offset = 0;
      for (i = 0; i < nrows; i++)
      {
         if (CF_marker[i] == point_type)
         {
            bidx = cnt / blk_size;
            ridx = cnt % blk_size;
            bidxm1 = bidx * blk_size;
            bidxp1 = (bidx + 1) * blk_size;
            for (ii = A_diag_i[i]; ii < A_diag_i[i + 1]; ii++)
            {
               jj = A_diag_j[ii];
               if (CF_marker[jj] == point_type)
               {
                  if (jj - row_offset >= bidxm1 && jj - row_offset < bidxp1 && fabs(A_diag_data[ii]) > HYPRE_REAL_MIN)
                  {
                     didx = bidx * bs2 + ridx * blk_size + jj - bidxm1 - row_offset;
                     diag[didx] = A_diag_data[ii];
                  }
               }
            }
            if (++cnt == whole_num_points) { break; }
         }
         else
         {
            row_offset++;
         }
      }
      // remaining points
      for (i = whole_num_points; i < num_points; i++)
      {
         if (CF_marker[i] == point_type)
         {
            bidx = n_block;
            ridx = cnt - whole_num_points;
            bidxm1 = whole_num_points;
            bidxp1 = num_points;
            for (ii = A_diag_i[i]; ii < A_diag_i[i + 1]; ii++)
            {
               jj = A_diag_j[ii];
               if (CF_marker[jj] == point_type)
               {
                  if (jj - row_offset >= bidxm1 && jj - row_offset < bidxp1 && fabs(A_diag_data[ii]) > HYPRE_REAL_MIN)
                  {
                     didx = bstart + ridx * left_size + jj - bidxm1 - row_offset;
                     diag[didx] = A_diag_data[ii];
                  }
               }
            }
            cnt++;
         }
         else
         {
            row_offset++;
         }
      }
   }
   wall_time = time_getWallclockSeconds() - wall_time;

   /*-----------------------------------------------------------------
   * compute the inverses of all the diagonal sub-blocks
   *-----------------------------------------------------------------*/
   wall_time = time_getWallclockSeconds();
   if (diag_type == 1) // compute the inverse
   {
      if (blk_size > 1)
      {
         hypre_BlockDiagInvLapack(diag, num_points, blk_size);
      }
      else
      {
         for (i = 0; i < num_points; i++)
         {
            if (fabs(diag[i]) < HYPRE_REAL_MIN)
            {
               diag[i] = 0.0;
            }
            else
            {
               diag[i] = 1.0 / diag[i];
            }
         }
      }
   }
   wall_time = time_getWallclockSeconds() - wall_time;

   *diag_ptr = diag;

   return hypre_error_flag;
}

// Extract the block diagonal part of a A or a principal submatrix of A defined by a marker (point_type)
// in an associated CF_marker array. The result is new block diagonal parCSR matrix.
// If CF marker array is NULL, it returns the block diagonal of the original matrix A.
// Options for diag_type are:
// diag_type = 1: return the inverse of the block diagonals
// otherwise : return the block diagonals
// ***Input***
// A - parCSR matrix
// blk_size - Size of diagonal blocks to extract
// CF_marker - Array prescribing submatrix from which to extract block diagonals. Ignored if NULL.
// point_type - marker tag in CF_marker array to extract diagonal
// diag_type - Type of block diagonal entries to return. Currently supports block diagonal or inverse block diagonal entries.
// ***Output***
// B_ptr: New block diagonal matrix
//
HYPRE_Int hypre_ParCSRMatrixBlockDiagMatrix(  hypre_ParCSRMatrix  *A,
                                              HYPRE_Int           blk_size,
                                              HYPRE_Int           point_type,
                                              HYPRE_Int           *CF_marker,
                                              hypre_ParCSRMatrix  **B_ptr,
                                              HYPRE_Int           diag_type)
{
   MPI_Comm          comm = hypre_ParCSRMatrixComm(A);
   HYPRE_Int         num_procs,  my_id;

   hypre_ParCSRMatrix   *B;

   hypre_CSRMatrix *B_diag;
   HYPRE_Real      *B_diag_data;
   HYPRE_Int       *B_diag_i;
   HYPRE_Int       *B_diag_j;

   hypre_CSRMatrix *B_offd;
   HYPRE_Int       i, j, k;

   HYPRE_Int n_block, left_size, diag_size;
   HYPRE_Int blk_diag_nlocal_rows;
   HYPRE_BigInt blk_diag_total_global_nrows;

   HYPRE_Int        bidx;
   HYPRE_Real       *diag = NULL;
   HYPRE_Real       *diag_local = NULL;
   HYPRE_BigInt     *blk_diag_row_starts;

   const HYPRE_Int nb2 = blk_size * blk_size;
   HYPRE_Real wall_time;

   HYPRE_MemoryLocation memory_location = hypre_ParCSRMatrixMemoryLocation(A);

   HYPRE_Int nrows = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));

   if (nrows > 0 && nrows < blk_size)
   {
      hypre_error_w_msg(HYPRE_ERROR_GENERIC, "Error!!! Input matrix is smaller than block size.");
      return hypre_error_flag;
   }

   // number of local rows of the new matrix is
   // the number of points in CF_marker matching point_type.
   if (CF_marker != NULL)
   {
      blk_diag_nlocal_rows = 0;
      for (i = 0; i < nrows; i++)
      {
         if (CF_marker[i] == point_type)
         {
            blk_diag_nlocal_rows++;
         }
      }
   }
   else
   {
      blk_diag_nlocal_rows = nrows;
   }

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);

   wall_time = time_getWallclockSeconds();
   hypre_ParCSRMatrixExtractBlockDiag(A, blk_size, point_type, CF_marker, &diag_size, &diag,
                                      diag_type);
   wall_time = time_getWallclockSeconds() - wall_time;
   n_block = blk_diag_nlocal_rows / blk_size;
   left_size = blk_diag_nlocal_rows - n_block * blk_size;

   /*-----------------------------------------------------------------------
   *  First Pass: Determine size of B and fill in
   *-----------------------------------------------------------------------*/

   B_diag_i    = hypre_CTAlloc(HYPRE_Int,  blk_diag_nlocal_rows + 1, memory_location);
   B_diag_j    = hypre_CTAlloc(HYPRE_Int,  diag_size, memory_location);
   B_diag_data = hypre_CTAlloc(HYPRE_Real,  diag_size, memory_location);

   B_diag_i[blk_diag_nlocal_rows] = diag_size;

   /*-----------------------------------------------------------------
   * Get all the diagonal sub-blocks
   *-----------------------------------------------------------------*/
   //printf("n_block = %d\n",n_block);
   for (i = 0; i < n_block; i++)
   {
      diag_local = &diag[i * nb2];
      for (k = 0; k < blk_size; k++)
      {
         B_diag_i[i * blk_size + k] = i * nb2 + k * blk_size;

         for (j = 0; j < blk_size; j++)
         {
            bidx = i * nb2 + k * blk_size + j;
            B_diag_j[bidx] = i * blk_size + j;
            B_diag_data[bidx] = diag_local[k * blk_size + j];
         }
      }
   }
   // treat the remaining points
   diag_local = &diag[n_block * nb2];
   for (k = 0; k < left_size; k++)
   {
      B_diag_i[n_block * blk_size + k] = n_block * nb2 + k * left_size;

      for (j = 0; j < left_size; j++)
      {
         bidx = n_block * nb2 + k * left_size + j;
         B_diag_j[bidx] = n_block * blk_size + j;
         B_diag_data[bidx] = diag_local[k * left_size + j];
      }
   }

   {
      HYPRE_BigInt scan_recv;
      HYPRE_BigInt nlocal_rows = blk_diag_nlocal_rows;

      blk_diag_row_starts = hypre_CTAlloc(HYPRE_BigInt, 2, memory_location);
      hypre_MPI_Scan(&nlocal_rows, &scan_recv, 1, HYPRE_MPI_BIG_INT, hypre_MPI_SUM, comm);
      /* first point in my range */
      blk_diag_row_starts[0] = scan_recv - nlocal_rows;
      /* first point in next proc's range */
      blk_diag_row_starts[1] = scan_recv;
      if (my_id == (num_procs - 1)) { blk_diag_total_global_nrows = blk_diag_row_starts[1]; }
      hypre_MPI_Bcast(&blk_diag_total_global_nrows, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);
   }

   B = hypre_ParCSRMatrixCreate(comm,
                                blk_diag_total_global_nrows,
                                blk_diag_total_global_nrows,
                                blk_diag_row_starts,
                                blk_diag_row_starts,
                                0,
                                diag_size,
                                0);
   //printf("After create\n");
   B_diag = hypre_ParCSRMatrixDiag(B);
   hypre_CSRMatrixData(B_diag) = B_diag_data;
   hypre_CSRMatrixI(B_diag) = B_diag_i;
   hypre_CSRMatrixJ(B_diag) = B_diag_j;
   //hypre_CSRMatrixReorder(B_diag);

   B_offd = hypre_ParCSRMatrixOffd(B);
   hypre_CSRMatrixData(B_offd) = NULL;
   hypre_CSRMatrixI(B_offd) = NULL;
   hypre_CSRMatrixJ(B_offd) = NULL;

   //   hypre_MatvecCommPkgCreate(B);
   *B_ptr = B;

   hypre_TFree(diag, memory_location);
   hypre_TFree(blk_diag_row_starts, memory_location);

   return hypre_error_flag;
}

/*Setup block smoother:
 * Computes the entries of the inverse of the block diagonal matrix with blk_size diagonal blocks.
 * Current implementation ignores reserved Cpoints and acts on whole matrix.
 */
HYPRE_Int
hypre_MGRBlockRelaxSetup(hypre_ParCSRMatrix *A,
                         HYPRE_Int          blk_size,
                         HYPRE_Real        **diaginvptr)
{
   HYPRE_Int blk_diag_size;
   hypre_ParCSRMatrixExtractBlockDiag(A, blk_size, 0, NULL, &blk_diag_size, diaginvptr, 1);

#if 0
   MPI_Comm      comm = hypre_ParCSRMatrixComm(A);
   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real     *A_diag_data  = hypre_CSRMatrixData(A_diag);
   HYPRE_Int            *A_diag_i     = hypre_CSRMatrixI(A_diag);
   HYPRE_Int            *A_diag_j     = hypre_CSRMatrixJ(A_diag);
   HYPRE_Int             n       = hypre_CSRMatrixNumRows(A_diag);

   HYPRE_Int             i, j, k;
   HYPRE_Int             ii, jj;
   HYPRE_Int             bidx, bidxm1, bidxp1;
   HYPRE_Int         num_procs, my_id;

   const HYPRE_Int     nb2 = blk_size * blk_size;
   HYPRE_Int           n_block;
   HYPRE_Int           left_size, inv_size;
   HYPRE_Real        *diaginv = *diaginvptr;


   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);
   //HYPRE_Int num_threads = hypre_NumThreads();

   if (my_id == num_procs)
   {
      n_block   = (n - reserved_coarse_size) / blk_size;
      left_size = n - blk_size * n_block;
   }
   else
   {
      n_block = n / blk_size;
      left_size = n - blk_size * n_block;
   }

   n_block = n / blk_size;
   left_size = n - blk_size * n_block;

   inv_size  = nb2 * n_block + left_size * left_size;

   if (diaginv != NULL)
   {
      hypre_TFree(diaginv, HYPRE_MEMORY_HOST);
      diaginv = hypre_CTAlloc(HYPRE_Real,  inv_size, HYPRE_MEMORY_HOST);
   }
   else
   {
      diaginv = hypre_CTAlloc(HYPRE_Real,  inv_size, HYPRE_MEMORY_HOST);
   }

   /*-----------------------------------------------------------------
   * Get all the diagonal sub-blocks
   *-----------------------------------------------------------------*/
   for (i = 0; i < n_block; i++)
   {
      bidxm1 = i * blk_size;
      bidxp1 = (i + 1) * blk_size;
      //printf("bidxm1 = %d,bidxp1 = %d\n",bidxm1,bidxp1);

      for (k = 0; k < blk_size; k++)
      {
         for (j = 0; j < blk_size; j++)
         {
            bidx = i * nb2 + k * blk_size + j;
            diaginv[bidx] = 0.0;
         }

         for (ii = A_diag_i[bidxm1 + k]; ii < A_diag_i[bidxm1 + k + 1]; ii++)
         {
            jj = A_diag_j[ii];
            if (jj >= bidxm1 && jj < bidxp1 && hypre_cabs(A_diag_data[ii]) > HYPRE_REAL_MIN)
            {
               bidx = i * nb2 + k * blk_size + jj - bidxm1;
               //printf("jj = %d,val = %e, bidx = %d\n",jj,A_diag_data[ii],bidx);
               diaginv[bidx] = A_diag_data[ii];
            }
         }
      }
   }

   for (i = 0; i < left_size; i++)
   {
      bidxm1 = n_block * nb2 + i * blk_size;
      bidxp1 = n_block * nb2 + (i + 1) * blk_size;
      for (j = 0; j < left_size; j++)
      {
         bidx = n_block * nb2 + i * blk_size + j;
         diaginv[bidx] = 0.0;
      }

      for (ii = A_diag_i[n_block * blk_size + i]; ii < A_diag_i[n_block * blk_size + i + 1]; ii++)
      {
         jj = A_diag_j[ii];
         if (jj > n_block * blk_size)
         {
            bidx = n_block * nb2 + i * blk_size + jj - n_block * blk_size;
            diaginv[bidx] = A_diag_data[ii];
         }
      }
   }

   /*-----------------------------------------------------------------
   * compute the inverses of all the diagonal sub-blocks
   *-----------------------------------------------------------------*/
   if (blk_size > 1)
   {
      for (i = 0; i < n_block; i++)
      {
         hypre_blas_mat_inv(diaginv + i * nb2, blk_size);
      }
      hypre_blas_mat_inv(diaginv + (HYPRE_Int)(blk_size * nb2), left_size);
   }
   else
   {
      for (i = 0; i < n; i++)
      {
         // FIX-ME: zero-diagonal should be tested previously
         if (fabs(diaginv[i]) < HYPRE_REAL_MIN)
         {
            diaginv[i] = 0.0;
         }
         else
         {
            diaginv[i] = 1.0 / diaginv[i];
         }
      }
   }

   *diaginvptr = diaginv;
#endif
   return hypre_error_flag;
}
#if 0
HYPRE_Int
hypre_blockRelax(hypre_ParCSRMatrix *A,
                 hypre_ParVector    *f,
                 hypre_ParVector    *u,
                 HYPRE_Int          blk_size,
                 HYPRE_Int          reserved_coarse_size,
                 HYPRE_Int          method,
                 hypre_ParVector    *Vtemp,
                 hypre_ParVector    *Ztemp)
{
   MPI_Comm      comm = hypre_ParCSRMatrixComm(A);
   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real     *A_diag_data  = hypre_CSRMatrixData(A_diag);
   HYPRE_Int            *A_diag_i     = hypre_CSRMatrixI(A_diag);
   HYPRE_Int            *A_diag_j     = hypre_CSRMatrixJ(A_diag);
   HYPRE_Int             n       = hypre_CSRMatrixNumRows(A_diag);

   HYPRE_Int             i, j, k;
   HYPRE_Int             ii, jj;

   HYPRE_Int             bidx, bidxm1, bidxp1;

   HYPRE_Int         num_procs, my_id;

   const HYPRE_Int     nb2 = blk_size * blk_size;
   HYPRE_Int           n_block;
   HYPRE_Int           left_size, inv_size;
   HYPRE_Real          *diaginv;

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);

   //HYPRE_Int num_threads = hypre_NumThreads();

   if (my_id == num_procs)
   {
      n_block   = (n - reserved_coarse_size) / blk_size;
      left_size = n - blk_size * n_block;
   }
   else
   {
      n_block = n / blk_size;
      left_size = n - blk_size * n_block;
   }

   inv_size  = nb2 * n_block + left_size * left_size;

   diaginv = hypre_CTAlloc(HYPRE_Real,  inv_size, HYPRE_MEMORY_HOST);
   /*-----------------------------------------------------------------
   * Get all the diagonal sub-blocks
   *-----------------------------------------------------------------*/
   for (i = 0; i < n_block; i++)
   {
      bidxm1 = i * blk_size;
      bidxp1 = (i + 1) * blk_size;
      //printf("bidxm1 = %d,bidxp1 = %d\n",bidxm1,bidxp1);

      for (k = 0; k < blk_size; k++)
      {
         for (j = 0; j < blk_size; j++)
         {
            bidx = i * nb2 + k * blk_size + j;
            diaginv[bidx] = 0.0;
         }

         for (ii = A_diag_i[bidxm1 + k]; ii < A_diag_i[bidxm1 + k + 1]; ii++)
         {
            jj = A_diag_j[ii];

            if (jj >= bidxm1 && jj < bidxp1 && fabs(A_diag_data[ii]) > HYPRE_REAL_MIN)
            {
               bidx = i * nb2 + k * blk_size + jj - bidxm1;
               //printf("jj = %d,val = %e, bidx = %d\n",jj,A_diag_data[ii],bidx);
               diaginv[bidx] = A_diag_data[ii];
            }
         }
      }
   }

   for (i = 0; i < left_size; i++)
   {
      bidxm1 = n_block * nb2 + i * blk_size;
      bidxp1 = n_block * nb2 + (i + 1) * blk_size;
      for (j = 0; j < left_size; j++)
      {
         bidx = n_block * nb2 + i * blk_size + j;
         diaginv[bidx] = 0.0;
      }

      for (ii = A_diag_i[n_block * blk_size + i]; ii < A_diag_i[n_block * blk_size + i + 1]; ii++)
      {
         jj = A_diag_j[ii];
         if (jj > n_block * blk_size)
         {
            bidx = n_block * nb2 + i * blk_size + jj - n_block * blk_size;
            diaginv[bidx] = A_diag_data[ii];
         }
      }
   }
   /*
   for (i = 0;i < n_block; i++)
   {
     for (j = 0;j < blk_size; j++)
     {
       for (k = 0;k < blk_size; k ++)
       {
         bidx = i*nb2 + j*blk_size + k;
         printf("%e\t",diaginv[bidx]);
       }
       printf("\n");
     }
     printf("\n");
   }
   */
   /*-----------------------------------------------------------------
   * compute the inverses of all the diagonal sub-blocks
   *-----------------------------------------------------------------*/
   if (blk_size > 1)
   {
      for (i = 0; i < n_block; i++)
      {
         hypre_blas_mat_inv(diaginv + i * nb2, blk_size);
      }
      hypre_blas_mat_inv(diaginv + (HYPRE_Int)(blk_size * nb2), left_size);
      /*
      for (i = 0;i < n_block; i++)
      {
        for (j = 0;j < blk_size; j++)
        {
          for (k = 0;k < blk_size; k ++)
          {
            bidx = i*nb2 + j*blk_size + k;
            printf("%e\t",diaginv[bidx]);
          }
          printf("\n");
        }
        printf("\n");
      }
      */
   }
   else
   {
      for (i = 0; i < n; i++)
      {
         // FIX-ME: zero-diagonal should be tested previously
         if (fabs(diaginv[i]) < HYPRE_REAL_MIN)
         {
            diaginv[i] = 0.0;
         }
         else
         {
            diaginv[i] = 1.0 / diaginv[i];
         }
      }
   }

   hypre_MGRBlockRelaxSolve(A, f, u, blk_size, n_block, left_size, method, diaginv, Vtemp);

   /*-----------------------------------------------------------------
   * Free temperary memeory
   *-----------------------------------------------------------------*/
   hypre_TFree(diaginv, HYPRE_MEMORY_HOST);

   return (hypre_error_flag);
}
#endif
/* set F-relaxation solver */
HYPRE_Int
hypre_MGRSetFSolver( void  *mgr_vdata,
                     HYPRE_Int  (*fine_grid_solver_solve)(void*, void*, void*, void*),
                     HYPRE_Int  (*fine_grid_solver_setup)(void*, void*, void*, void*),
                     void       *fsolver )
{
   hypre_ParMGRData *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   if (!mgr_data)
   {
      hypre_error_in_arg(1);
      return hypre_error_flag;
   }
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   HYPRE_Solver **aff_solver = (mgr_data -> aff_solver);

   if (aff_solver == NULL)
   {
      aff_solver = hypre_CTAlloc(HYPRE_Solver*, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   }

   /* only allow to set F-solver for the first level */
   aff_solver[0] = (HYPRE_Solver *) fsolver;

   (mgr_data -> fine_grid_solver_solve) = fine_grid_solver_solve;
   (mgr_data -> fine_grid_solver_setup) = fine_grid_solver_setup;
   (mgr_data -> aff_solver) = aff_solver;
   (mgr_data -> fsolver_mode) = 0;

   return hypre_error_flag;
}

/* set coarse grid solver */
HYPRE_Int
hypre_MGRSetCoarseSolver( void  *mgr_vdata,
                          HYPRE_Int  (*coarse_grid_solver_solve)(void*, void*, void*, void*),
                          HYPRE_Int  (*coarse_grid_solver_setup)(void*, void*, void*, void*),
                          void  *coarse_grid_solver )
{
   hypre_ParMGRData *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   if (!mgr_data)
   {
      hypre_error_in_arg(1);
      return hypre_error_flag;
   }

   (mgr_data -> coarse_grid_solver_solve) = coarse_grid_solver_solve;
   (mgr_data -> coarse_grid_solver_setup) = coarse_grid_solver_setup;
   (mgr_data -> coarse_grid_solver)       = (HYPRE_Solver) coarse_grid_solver;

   (mgr_data -> use_default_cgrid_solver) = 0;

   return hypre_error_flag;
}

/* Set the maximum number of coarse levels.
 * maxcoarselevs = 1 yields the default 2-grid scheme.
*/
HYPRE_Int
hypre_MGRSetMaxCoarseLevels( void *mgr_vdata, HYPRE_Int maxcoarselevs )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> max_num_coarse_levels) = maxcoarselevs;
   return hypre_error_flag;
}
/* Set the system block size */
HYPRE_Int
hypre_MGRSetBlockSize( void *mgr_vdata, HYPRE_Int bsize )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> block_size) = bsize;
   return hypre_error_flag;
}
/* Set the relaxation type for the fine levels of the reduction.
 * Currently supports the following flavors of relaxation types
 * as described in the documentation:
 * relax_types 0 - 8, 13, 14, 18, 19, 98.
 * See par_relax.c and par_relax_more.c for more details.
 *
*/
HYPRE_Int
hypre_MGRSetRelaxType( void *mgr_vdata, HYPRE_Int relax_type )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> relax_type) = relax_type;
   return hypre_error_flag;
}

/* Set the number of relaxation sweeps */
HYPRE_Int
hypre_MGRSetNumRelaxSweeps( void *mgr_vdata, HYPRE_Int nsweeps )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree(mgr_data -> num_relax_sweeps, HYPRE_MEMORY_HOST);
   HYPRE_Int *num_relax_sweeps = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   for (i = 0; i < max_num_coarse_levels; i++)
   {
      num_relax_sweeps[i] = nsweeps;
   }
   (mgr_data -> num_relax_sweeps) = num_relax_sweeps;

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRSetLevelNumRelaxSweeps( void *mgr_vdata, HYPRE_Int *level_nsweeps )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree(mgr_data -> num_relax_sweeps, HYPRE_MEMORY_HOST);

   HYPRE_Int *num_relax_sweeps = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (level_nsweeps != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         num_relax_sweeps[i] = level_nsweeps[i];
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         num_relax_sweeps[i] = 0;
      }
   }
   (mgr_data -> num_relax_sweeps) = num_relax_sweeps;

   return hypre_error_flag;
}

/* Set the order of the global smoothing step at each level
 * 1=Down cycle/ Pre-smoothing (default)
 * 2=Up cycle/ Post-smoothing
 */
HYPRE_Int
hypre_MGRSetGlobalSmoothCycle( void *mgr_vdata, HYPRE_Int smooth_cycle )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> global_smooth_cycle) = smooth_cycle;
   return hypre_error_flag;
}

/* Set the F-relaxation strategy: 0=single level, 1=multi level */
HYPRE_Int
hypre_MGRSetFRelaxMethod( void *mgr_vdata, HYPRE_Int relax_method )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree(mgr_data -> Frelax_method, HYPRE_MEMORY_HOST);
   HYPRE_Int *Frelax_method = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   for (i = 0; i < max_num_coarse_levels; i++)
   {
      Frelax_method[i] = relax_method;
   }
   (mgr_data -> Frelax_method) = Frelax_method;
   return hypre_error_flag;
}

/* Set the F-relaxation strategy: 0=single level, 1=multi level */
/* This will be removed later. Use SetLevelFrelaxType */
HYPRE_Int
hypre_MGRSetLevelFRelaxMethod( void *mgr_vdata, HYPRE_Int *relax_method )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree(mgr_data -> Frelax_method, HYPRE_MEMORY_HOST);

   HYPRE_Int *Frelax_method = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (relax_method != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         Frelax_method[i] = relax_method[i];
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         Frelax_method[i] = 0;
      }
   }
   (mgr_data -> Frelax_method) = Frelax_method;
   return hypre_error_flag;
}

/* Set the F-relaxation type:
 * 0: Jacobi
 * 1: Vcycle smoother
 * 2: AMG
 * Otherwise: use standard BoomerAMGRelax options
*/
HYPRE_Int
hypre_MGRSetLevelFRelaxType( void *mgr_vdata, HYPRE_Int *relax_type )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree(mgr_data -> Frelax_type, HYPRE_MEMORY_HOST);

   HYPRE_Int *Frelax_type = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (relax_type != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         Frelax_type[i] = relax_type[i];
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         Frelax_type[i] = 0;
      }
   }
   (mgr_data -> Frelax_type) = Frelax_type;
   return hypre_error_flag;
}

/* Coarse grid method: 0=Galerkin RAP, 1=non-Galerkin with dropping*/
HYPRE_Int
hypre_MGRSetCoarseGridMethod( void *mgr_vdata, HYPRE_Int *cg_method )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);

   hypre_TFree(mgr_data -> mgr_coarse_grid_method, HYPRE_MEMORY_HOST);
   HYPRE_Int *mgr_coarse_grid_method = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels,
                                                     HYPRE_MEMORY_HOST);
   if (cg_method != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         mgr_coarse_grid_method[i] = cg_method[i];
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         mgr_coarse_grid_method[i] = 0;
      }
   }
   (mgr_data -> mgr_coarse_grid_method) = mgr_coarse_grid_method;
   return hypre_error_flag;
}

/* Set the F-relaxation number of functions for each level */
HYPRE_Int
hypre_MGRSetLevelFRelaxNumFunctions( void *mgr_vdata, HYPRE_Int *num_functions )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);

   hypre_TFree(mgr_data -> Frelax_num_functions, HYPRE_MEMORY_HOST);

   HYPRE_Int *Frelax_num_functions = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels,
                                                   HYPRE_MEMORY_HOST);
   if (num_functions != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         Frelax_num_functions[i] = num_functions[i];
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         Frelax_num_functions[i] = 1;
      }
   }
   (mgr_data -> Frelax_num_functions) = Frelax_num_functions;
   return hypre_error_flag;
}

/* Set the type of the restriction type
 * for computing restriction operator
*/
HYPRE_Int
hypre_MGRSetLevelRestrictType( void *mgr_vdata, HYPRE_Int *restrict_type)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree((mgr_data -> restrict_type), HYPRE_MEMORY_HOST);

   HYPRE_Int *level_restrict_type = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (restrict_type != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         level_restrict_type[i] = *(restrict_type + i);
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         level_restrict_type[i] = 0;
      }
   }
   (mgr_data -> restrict_type) = level_restrict_type;
   return hypre_error_flag;
}

/* Set the type of the restriction type
 * for computing restriction operator
*/
HYPRE_Int
hypre_MGRSetRestrictType( void *mgr_vdata, HYPRE_Int restrict_type)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   if ((mgr_data -> restrict_type) != NULL)
   {
      hypre_TFree((mgr_data -> restrict_type), HYPRE_MEMORY_HOST);
      (mgr_data -> restrict_type) = NULL;
   }
   HYPRE_Int *level_restrict_type = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   for (i = 0; i < max_num_coarse_levels; i++)
   {
      level_restrict_type[i] = restrict_type;
   }
   (mgr_data -> restrict_type) = level_restrict_type;
   return hypre_error_flag;
}

/* Set the number of Jacobi interpolation iterations
 * for computing interpolation operator
*/
HYPRE_Int
hypre_MGRSetNumRestrictSweeps( void *mgr_vdata, HYPRE_Int nsweeps )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> num_restrict_sweeps) = nsweeps;
   return hypre_error_flag;
}

/* Set the type of the interpolation
 * for computing interpolation operator
*/
HYPRE_Int
hypre_MGRSetInterpType( void *mgr_vdata, HYPRE_Int interpType)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   if ((mgr_data -> interp_type) != NULL)
   {
      hypre_TFree((mgr_data -> interp_type), HYPRE_MEMORY_HOST);
      (mgr_data -> interp_type) = NULL;
   }
   HYPRE_Int *level_interp_type = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   for (i = 0; i < max_num_coarse_levels; i++)
   {
      level_interp_type[i] = interpType;
   }
   (mgr_data -> interp_type) = level_interp_type;
   return hypre_error_flag;
}

/* Set the type of the interpolation
 * for computing interpolation operator
*/
HYPRE_Int
hypre_MGRSetLevelInterpType( void *mgr_vdata, HYPRE_Int *interpType)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree((mgr_data -> interp_type), HYPRE_MEMORY_HOST);

   HYPRE_Int *level_interp_type = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (interpType != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         level_interp_type[i] = *(interpType + i);
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         level_interp_type[i] = 2;
      }
   }
   (mgr_data -> interp_type) = level_interp_type;
   return hypre_error_flag;
}

/* Set the number of Jacobi interpolation iterations
 * for computing interpolation operator
*/
HYPRE_Int
hypre_MGRSetNumInterpSweeps( void *mgr_vdata, HYPRE_Int nsweeps )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> num_interp_sweeps) = nsweeps;
   return hypre_error_flag;
}

/* Set the threshold to truncate the coarse grid at each
 * level of reduction
*/
HYPRE_Int
hypre_MGRSetTruncateCoarseGridThreshold( void *mgr_vdata, HYPRE_Real threshold)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> truncate_coarse_grid_threshold) = threshold;
   return hypre_error_flag;
}

/* Set block size for block Jacobi Interp/Relax */
HYPRE_Int
hypre_MGRSetBlockJacobiBlockSize( void *mgr_vdata, HYPRE_Int blk_size)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> block_jacobi_bsize) = blk_size;
   return hypre_error_flag;
}

/* Set print level for F-relaxation solver */
HYPRE_Int
hypre_MGRSetFrelaxPrintLevel( void *mgr_vdata, HYPRE_Int print_level )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> frelax_print_level) = print_level;
   return hypre_error_flag;
}

/* Set print level for coarse grid solver */
HYPRE_Int
hypre_MGRSetCoarseGridPrintLevel( void *mgr_vdata, HYPRE_Int print_level )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> cg_print_level) = print_level;
   return hypre_error_flag;
}

/* Set print level for mgr solver */
HYPRE_Int
hypre_MGRSetPrintLevel( void *mgr_vdata, HYPRE_Int print_level )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> print_level) = print_level;
   return hypre_error_flag;
}

/* Set logging level for mgr solver */
HYPRE_Int
hypre_MGRSetLogging( void *mgr_vdata, HYPRE_Int logging )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> logging) = logging;
   return hypre_error_flag;
}

/* Set max number of iterations for mgr solver */
HYPRE_Int
hypre_MGRSetMaxIter( void *mgr_vdata, HYPRE_Int max_iter )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> max_iter) = max_iter;
   return hypre_error_flag;
}

/* Set convergence tolerance for mgr solver */
HYPRE_Int
hypre_MGRSetTol( void *mgr_vdata, HYPRE_Real tol )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> tol) = tol;
   return hypre_error_flag;
}

/* Set max number of iterations for mgr global smoother */
HYPRE_Int
hypre_MGRSetMaxGlobalSmoothIters( void *mgr_vdata, HYPRE_Int max_iter )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   if ((mgr_data -> level_smooth_iters) != NULL)
   {
      hypre_TFree((mgr_data -> level_smooth_iters), HYPRE_MEMORY_HOST);
      (mgr_data -> level_smooth_iters) = NULL;
   }
   HYPRE_Int *level_smooth_iters = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (max_num_coarse_levels > 0)
   {
      level_smooth_iters[0] = max_iter;
   }
   (mgr_data -> level_smooth_iters) = level_smooth_iters;

   return hypre_error_flag;
}

/* Set global smoothing type for mgr solver */
HYPRE_Int
hypre_MGRSetGlobalSmoothType( void *mgr_vdata, HYPRE_Int gsmooth_type )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   if ((mgr_data -> level_smooth_type) != NULL)
   {
      hypre_TFree((mgr_data -> level_smooth_type), HYPRE_MEMORY_HOST);
      (mgr_data -> level_smooth_type) = NULL;
   }
   HYPRE_Int *level_smooth_type = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (max_num_coarse_levels > 0)
   {
      level_smooth_type[0] = gsmooth_type;
   }
   (mgr_data -> level_smooth_type) = level_smooth_type;

   return hypre_error_flag;
}

/* Set global smoothing type for mgr solver */
HYPRE_Int
hypre_MGRSetLevelSmoothType( void *mgr_vdata, HYPRE_Int *gsmooth_type )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree((mgr_data -> level_smooth_type), HYPRE_MEMORY_HOST);

   HYPRE_Int *level_smooth_type = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (gsmooth_type != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         level_smooth_type[i] = gsmooth_type[i];
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         level_smooth_type[i] = 0;
      }
   }
   (mgr_data -> level_smooth_type) = level_smooth_type;
   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRSetLevelSmoothIters( void *mgr_vdata, HYPRE_Int *gsmooth_iters )
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_TFree((mgr_data -> level_smooth_iters), HYPRE_MEMORY_HOST);

   HYPRE_Int *level_smooth_iters = hypre_CTAlloc(HYPRE_Int, max_num_coarse_levels, HYPRE_MEMORY_HOST);
   if (gsmooth_iters != NULL)
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         level_smooth_iters[i] = gsmooth_iters[i];
      }
   }
   else
   {
      for (i = 0; i < max_num_coarse_levels; i++)
      {
         level_smooth_iters[i] = 0;
      }
   }
   (mgr_data -> level_smooth_iters) = level_smooth_iters;
   return hypre_error_flag;
}

/* Set the maximum number of non-zero entries for restriction
   and interpolation operator if classical AMG interpolation is used */
HYPRE_Int
hypre_MGRSetPMaxElmts( void *mgr_vdata, HYPRE_Int P_max_elmts)
{
   hypre_ParMGRData   *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   (mgr_data -> P_max_elmts) = P_max_elmts;
   return hypre_error_flag;
}

/* Get number of iterations for MGR solver */
HYPRE_Int
hypre_MGRGetNumIterations( void *mgr_vdata, HYPRE_Int *num_iterations )
{
   hypre_ParMGRData  *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   if (!mgr_data)
   {
      hypre_error_in_arg(1);
      return hypre_error_flag;
   }
   *num_iterations = mgr_data->num_iterations;

   return hypre_error_flag;
}

/* Get residual norms for MGR solver */
HYPRE_Int
hypre_MGRGetFinalRelativeResidualNorm( void *mgr_vdata, HYPRE_Real *res_norm )
{
   hypre_ParMGRData  *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   if (!mgr_data)
   {
      hypre_error_in_arg(1);
      return hypre_error_flag;
   }
   *res_norm = mgr_data->final_rel_residual_norm;

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRGetCoarseGridConvergenceFactor( void *mgr_vdata, HYPRE_Real *conv_factor )
{
   hypre_ParMGRData  *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   if (!mgr_data)
   {
      hypre_error_in_arg(1);
      return hypre_error_flag;
   }
   *conv_factor = (mgr_data -> cg_convergence_factor);

   return hypre_error_flag;
}


/* Build A_FF matrix from A given a CF_marker array */
HYPRE_Int
hypre_MGRGetSubBlock( hypre_ParCSRMatrix   *A,
                      HYPRE_Int            *row_cf_marker,
                      HYPRE_Int            *col_cf_marker,
                      HYPRE_Int             debug_flag,
                      hypre_ParCSRMatrix  **A_block_ptr )
{
   MPI_Comm        comm = hypre_ParCSRMatrixComm(A);
   hypre_ParCSRCommPkg     *comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   hypre_ParCSRCommHandle  *comm_handle;
   HYPRE_MemoryLocation memory_location = hypre_ParCSRMatrixMemoryLocation(A);

   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   HYPRE_Real      *A_diag_data = hypre_CSRMatrixData(A_diag);
   HYPRE_Int             *A_diag_i = hypre_CSRMatrixI(A_diag);
   HYPRE_Int             *A_diag_j = hypre_CSRMatrixJ(A_diag);

   hypre_CSRMatrix *A_offd         = hypre_ParCSRMatrixOffd(A);
   HYPRE_Real      *A_offd_data    = hypre_CSRMatrixData(A_offd);
   HYPRE_Int             *A_offd_i = hypre_CSRMatrixI(A_offd);
   HYPRE_Int             *A_offd_j = hypre_CSRMatrixJ(A_offd);
   HYPRE_Int              num_cols_A_offd = hypre_CSRMatrixNumCols(A_offd);
   //HYPRE_Int             *col_map_offd = hypre_ParCSRMatrixColMapOffd(A);

   hypre_IntArray          *coarse_dof_func_ptr = NULL;
   HYPRE_BigInt            num_row_cpts_global[2];
   HYPRE_BigInt            num_col_cpts_global[2];

   hypre_ParCSRMatrix    *Ablock;
   HYPRE_BigInt         *col_map_offd_Ablock;
   HYPRE_Int       *tmp_map_offd = NULL;

   HYPRE_Int             *CF_marker_offd = NULL;

   hypre_CSRMatrix    *Ablock_diag;
   hypre_CSRMatrix    *Ablock_offd;

   HYPRE_Real      *Ablock_diag_data;
   HYPRE_Int             *Ablock_diag_i;
   HYPRE_Int             *Ablock_diag_j;
   HYPRE_Real      *Ablock_offd_data;
   HYPRE_Int             *Ablock_offd_i;
   HYPRE_Int             *Ablock_offd_j;

   HYPRE_Int              Ablock_diag_size, Ablock_offd_size;

   HYPRE_Int             *Ablock_marker;

   HYPRE_Int              ii_counter;
   HYPRE_Int              jj_counter, jj_counter_offd;
   HYPRE_Int             *jj_count, *jj_count_offd;

   HYPRE_Int              start_indexing = 0; /* start indexing for Aff_data at 0 */

   HYPRE_Int              n_fine = hypre_CSRMatrixNumRows(A_diag);

   HYPRE_Int             *fine_to_coarse;
   HYPRE_Int             *coarse_counter;
   HYPRE_Int             *col_coarse_counter;
   HYPRE_Int              coarse_shift;
   HYPRE_BigInt              total_global_row_cpts;
   HYPRE_BigInt              total_global_col_cpts;
   HYPRE_Int              num_cols_Ablock_offd;
   //  HYPRE_BigInt              my_first_row_cpt, my_first_col_cpt;

   HYPRE_Int              i, i1;
   HYPRE_Int              j, jl, jj;
   HYPRE_Int              start;

   HYPRE_Int              my_id;
   HYPRE_Int              num_procs;
   HYPRE_Int              num_threads;
   HYPRE_Int              num_sends;
   HYPRE_Int              index;
   HYPRE_Int              ns, ne, size, rest;
   HYPRE_Int             *int_buf_data;
   HYPRE_Int              local_numrows = hypre_CSRMatrixNumRows(A_diag);

   hypre_IntArray        *wrap_cf;

   //  HYPRE_Real       wall_time;  /* for debugging instrumentation  */

   hypre_MPI_Comm_size(comm, &num_procs);
   hypre_MPI_Comm_rank(comm, &my_id);
   //num_threads = hypre_NumThreads();
   // Temporary fix, disable threading
   // TODO: enable threading
   num_threads = 1;

   /* get the number of coarse rows */
   wrap_cf = hypre_IntArrayCreate(local_numrows);
   hypre_IntArrayMemoryLocation(wrap_cf) = HYPRE_MEMORY_HOST;
   hypre_IntArrayData(wrap_cf) = row_cf_marker;
   hypre_BoomerAMGCoarseParms(comm, local_numrows, 1, NULL, wrap_cf, &coarse_dof_func_ptr,
                              num_row_cpts_global);
   hypre_IntArrayDestroy(coarse_dof_func_ptr);
   coarse_dof_func_ptr = NULL;

   //hypre_printf("my_id = %d, cpts_this = %d, cpts_next = %d\n", my_id, num_row_cpts_global[0], num_row_cpts_global[1]);

   //  my_first_row_cpt = num_row_cpts_global[0];
   if (my_id == (num_procs - 1)) { total_global_row_cpts = num_row_cpts_global[1]; }
   hypre_MPI_Bcast(&total_global_row_cpts, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);

   /* get the number of coarse rows */
   hypre_IntArrayData(wrap_cf) = col_cf_marker;
   hypre_BoomerAMGCoarseParms(comm, local_numrows, 1, NULL, wrap_cf, &coarse_dof_func_ptr,
                              num_col_cpts_global);
   hypre_IntArrayDestroy(coarse_dof_func_ptr);
   coarse_dof_func_ptr = NULL;

   //hypre_printf("my_id = %d, cpts_this = %d, cpts_next = %d\n", my_id, num_col_cpts_global[0], num_col_cpts_global[1]);

   //  my_first_col_cpt = num_col_cpts_global[0];
   if (my_id == (num_procs - 1)) { total_global_col_cpts = num_col_cpts_global[1]; }
   hypre_MPI_Bcast(&total_global_col_cpts, 1, HYPRE_MPI_BIG_INT, num_procs - 1, comm);

   /*-------------------------------------------------------------------
    * Get the CF_marker data for the off-processor columns
    *-------------------------------------------------------------------*/
   if (debug_flag < 0)
   {
      debug_flag = -debug_flag;
   }

   //  if (debug_flag==4) wall_time = time_getWallclockSeconds();

   if (num_cols_A_offd) { CF_marker_offd = hypre_CTAlloc(HYPRE_Int, num_cols_A_offd, HYPRE_MEMORY_HOST); }

   if (!comm_pkg)
   {
      hypre_MatvecCommPkgCreate(A);
      comm_pkg = hypre_ParCSRMatrixCommPkg(A);
   }

   num_sends = hypre_ParCSRCommPkgNumSends(comm_pkg);
   int_buf_data = hypre_CTAlloc(HYPRE_Int, hypre_ParCSRCommPkgSendMapStart(comm_pkg,
                                                                           num_sends), HYPRE_MEMORY_HOST);

   index = 0;
   for (i = 0; i < num_sends; i++)
   {
      start = hypre_ParCSRCommPkgSendMapStart(comm_pkg, i);
      for (j = start; j < hypre_ParCSRCommPkgSendMapStart(comm_pkg, i + 1); j++)
         int_buf_data[index++]
            = col_cf_marker[hypre_ParCSRCommPkgSendMapElmt(comm_pkg, j)];
   }

   comm_handle = hypre_ParCSRCommHandleCreate( 11, comm_pkg, int_buf_data,
                                               CF_marker_offd);
   hypre_ParCSRCommHandleDestroy(comm_handle);

   /*-----------------------------------------------------------------------
    *  First Pass: Determine size of Ablock and fill in fine_to_coarse mapping.
    *-----------------------------------------------------------------------*/

   /*-----------------------------------------------------------------------
    *  Intialize counters and allocate mapping vector.
    *-----------------------------------------------------------------------*/

   coarse_counter = hypre_CTAlloc(HYPRE_Int, num_threads, HYPRE_MEMORY_HOST);
   col_coarse_counter = hypre_CTAlloc(HYPRE_Int, num_threads, HYPRE_MEMORY_HOST);
   jj_count = hypre_CTAlloc(HYPRE_Int, num_threads, HYPRE_MEMORY_HOST);
   jj_count_offd = hypre_CTAlloc(HYPRE_Int, num_threads, HYPRE_MEMORY_HOST);

   fine_to_coarse = hypre_CTAlloc(HYPRE_Int, n_fine, HYPRE_MEMORY_HOST);
#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (i = 0; i < n_fine; i++) { fine_to_coarse[i] = -1; }

   jj_counter = start_indexing;
   jj_counter_offd = start_indexing;

   /*-----------------------------------------------------------------------
    *  Loop over fine grid.
    *-----------------------------------------------------------------------*/

   /* RDF: this looks a little tricky, but doable */
#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,j,i1,jj,ns,ne,size,rest) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (j = 0; j < num_threads; j++)
   {
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;

      if (j < rest)
      {
         ns = j * size + j;
         ne = (j + 1) * size + j + 1;
      }
      else
      {
         ns = j * size + rest;
         ne = (j + 1) * size + rest;
      }
      for (i = ns; i < ne; i++)
      {
         /*--------------------------------------------------------------------
          *  If i is a F-point, we loop through the columns and select
          *  the F-columns. Also set up mapping vector.
          *--------------------------------------------------------------------*/

         if (col_cf_marker[i] > 0)
         {
            fine_to_coarse[i] = col_coarse_counter[j];
            col_coarse_counter[j]++;
         }

         if (row_cf_marker[i] > 0)
         {
            //fine_to_coarse[i] = coarse_counter[j];
            coarse_counter[j]++;
            for (jj = A_diag_i[i]; jj < A_diag_i[i + 1]; jj++)
            {
               i1 = A_diag_j[jj];
               if (col_cf_marker[i1] > 0)
               {
                  jj_count[j]++;
               }
            }

            if (num_procs > 1)
            {
               for (jj = A_offd_i[i]; jj < A_offd_i[i + 1]; jj++)
               {
                  i1 = A_offd_j[jj];
                  if (CF_marker_offd[i1] > 0)
                  {
                     jj_count_offd[j]++;
                  }
               }
            }
         }
      }
   }

   /*-----------------------------------------------------------------------
    *  Allocate  arrays.
    *-----------------------------------------------------------------------*/
   for (i = 0; i < num_threads - 1; i++)
   {
      jj_count[i + 1] += jj_count[i];
      jj_count_offd[i + 1] += jj_count_offd[i];
      coarse_counter[i + 1] += coarse_counter[i];
      col_coarse_counter[i + 1] += col_coarse_counter[i];
   }
   i = num_threads - 1;
   jj_counter = jj_count[i];
   jj_counter_offd = jj_count_offd[i];
   ii_counter = coarse_counter[i];

   Ablock_diag_size = jj_counter;

   Ablock_diag_i    = hypre_CTAlloc(HYPRE_Int, ii_counter + 1, memory_location);
   Ablock_diag_j    = hypre_CTAlloc(HYPRE_Int, Ablock_diag_size, memory_location);
   Ablock_diag_data = hypre_CTAlloc(HYPRE_Real, Ablock_diag_size, memory_location);

   Ablock_diag_i[ii_counter] = jj_counter;


   Ablock_offd_size = jj_counter_offd;

   Ablock_offd_i    = hypre_CTAlloc(HYPRE_Int, ii_counter + 1, memory_location);
   Ablock_offd_j    = hypre_CTAlloc(HYPRE_Int, Ablock_offd_size, memory_location);
   Ablock_offd_data = hypre_CTAlloc(HYPRE_Real, Ablock_offd_size, memory_location);

   /*-----------------------------------------------------------------------
    *  Intialize some stuff.
    *-----------------------------------------------------------------------*/

   jj_counter = start_indexing;
   jj_counter_offd = start_indexing;

   //-----------------------------------------------------------------------
   //  Send and receive fine_to_coarse info.
   //-----------------------------------------------------------------------

   //  if (debug_flag==4) wall_time = time_getWallclockSeconds();
#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,j,ns,ne,size,rest,coarse_shift) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (j = 0; j < num_threads; j++)
   {
      coarse_shift = 0;
      if (j > 0) { coarse_shift = col_coarse_counter[j - 1]; }
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;
      if (j < rest)
      {
         ns = j * size + j;
         ne = (j + 1) * size + j + 1;
      }
      else
      {
         ns = j * size + rest;
         ne = (j + 1) * size + rest;
      }
      for (i = ns; i < ne; i++)
      {
         fine_to_coarse[i] += coarse_shift;
      }
   }

   //  if (debug_flag==4) wall_time = time_getWallclockSeconds();
#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
   //  for (i = 0; i < n_fine; i++) fine_to_coarse[i] -= my_first_col_cpt;

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i,jl,i1,jj,ns,ne,size,rest,jj_counter,jj_counter_offd,ii_counter) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (jl = 0; jl < num_threads; jl++)
   {
      size = n_fine / num_threads;
      rest = n_fine - size * num_threads;
      if (jl < rest)
      {
         ns = jl * size + jl;
         ne = (jl + 1) * size + jl + 1;
      }
      else
      {
         ns = jl * size + rest;
         ne = (jl + 1) * size + rest;
      }
      jj_counter = 0;
      if (jl > 0) { jj_counter = jj_count[jl - 1]; }
      jj_counter_offd = 0;
      if (jl > 0) { jj_counter_offd = jj_count_offd[jl - 1]; }
      ii_counter = 0;
      for (i = ns; i < ne; i++)
      {
         /*--------------------------------------------------------------------
          *  If i is a F-point, we loop through the columns and select
          *  the F-columns. Also set up mapping vector.
          *--------------------------------------------------------------------*/
         if (row_cf_marker[i] > 0)
         {
            // Diagonal part of Ablock //
            Ablock_diag_i[ii_counter] = jj_counter;
            for (jj = A_diag_i[i]; jj < A_diag_i[i + 1]; jj++)
            {
               i1 = A_diag_j[jj];
               if (col_cf_marker[i1] > 0)
               {
                  Ablock_diag_j[jj_counter]    = fine_to_coarse[i1];
                  Ablock_diag_data[jj_counter] = A_diag_data[jj];
                  jj_counter++;
               }
            }

            // Off-Diagonal part of Ablock //
            Ablock_offd_i[ii_counter] = jj_counter_offd;
            if (num_procs > 1)
            {
               for (jj = A_offd_i[i]; jj < A_offd_i[i + 1]; jj++)
               {
                  i1 = A_offd_j[jj];
                  if (CF_marker_offd[i1] > 0)
                  {
                     Ablock_offd_j[jj_counter_offd]  = i1;
                     Ablock_offd_data[jj_counter_offd] = A_offd_data[jj];
                     jj_counter_offd++;
                  }
               }
            }
            ii_counter++;
         }
      }
      Ablock_offd_i[ii_counter] = jj_counter_offd;
      Ablock_diag_i[ii_counter] = jj_counter;
   }
   Ablock = hypre_ParCSRMatrixCreate(comm,
                                     total_global_row_cpts,
                                     total_global_col_cpts,
                                     num_row_cpts_global,
                                     num_col_cpts_global,
                                     0,
                                     Ablock_diag_i[ii_counter],
                                     Ablock_offd_i[ii_counter]);

   Ablock_diag = hypre_ParCSRMatrixDiag(Ablock);
   hypre_CSRMatrixData(Ablock_diag) = Ablock_diag_data;
   hypre_CSRMatrixI(Ablock_diag) = Ablock_diag_i;
   hypre_CSRMatrixJ(Ablock_diag) = Ablock_diag_j;
   Ablock_offd = hypre_ParCSRMatrixOffd(Ablock);
   hypre_CSRMatrixData(Ablock_offd) = Ablock_offd_data;
   hypre_CSRMatrixI(Ablock_offd) = Ablock_offd_i;
   hypre_CSRMatrixJ(Ablock_offd) = Ablock_offd_j;

   num_cols_Ablock_offd = 0;

   if (Ablock_offd_size)
   {
      Ablock_marker = hypre_CTAlloc(HYPRE_Int, num_cols_A_offd, HYPRE_MEMORY_HOST);
#if 0
#ifdef HYPRE_USING_OPENMP
      #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
      for (i = 0; i < num_cols_A_offd; i++)
      {
         Ablock_marker[i] = 0;
      }
      num_cols_Ablock_offd = 0;
      for (i = 0; i < Ablock_offd_size; i++)
      {
         index = Ablock_offd_j[i];
         if (!Ablock_marker[index])
         {
            num_cols_Ablock_offd++;
            Ablock_marker[index] = 1;
         }
      }

      col_map_offd_Ablock = hypre_CTAlloc(HYPRE_BigInt, num_cols_Ablock_offd, memory_location);
      tmp_map_offd = hypre_CTAlloc(HYPRE_Int, num_cols_Ablock_offd, HYPRE_MEMORY_HOST);
      index = 0;
      for (i = 0; i < num_cols_Ablock_offd; i++)
      {
         while (Ablock_marker[index] == 0) { index++; }
         tmp_map_offd[i] = index++;
      }
#if 0
#ifdef HYPRE_USING_OPENMP
      #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
      for (i = 0; i < Ablock_offd_size; i++)
         Ablock_offd_j[i] = hypre_BinarySearch(tmp_map_offd,
                                               Ablock_offd_j[i],
                                               num_cols_Ablock_offd);
      hypre_TFree(Ablock_marker, HYPRE_MEMORY_HOST);
   }

   if (num_cols_Ablock_offd)
   {
      hypre_ParCSRMatrixColMapOffd(Ablock) = col_map_offd_Ablock;
      hypre_CSRMatrixNumCols(Ablock_offd) = num_cols_Ablock_offd;
   }

   hypre_GetCommPkgRTFromCommPkgA(Ablock, A, fine_to_coarse, tmp_map_offd);

   /* Create the assumed partition */
   if (hypre_ParCSRMatrixAssumedPartition(Ablock) == NULL)
   {
      hypre_ParCSRMatrixCreateAssumedPartition(Ablock);
   }

   *A_block_ptr = Ablock;

   hypre_TFree(tmp_map_offd, HYPRE_MEMORY_HOST);
   hypre_TFree(CF_marker_offd, HYPRE_MEMORY_HOST);
   hypre_TFree(int_buf_data, HYPRE_MEMORY_HOST);
   hypre_TFree(fine_to_coarse, HYPRE_MEMORY_HOST);
   hypre_TFree(coarse_counter, HYPRE_MEMORY_HOST);
   hypre_TFree(col_coarse_counter, HYPRE_MEMORY_HOST);
   hypre_TFree(jj_count, HYPRE_MEMORY_HOST);
   hypre_TFree(jj_count_offd, HYPRE_MEMORY_HOST);
   hypre_IntArrayData(wrap_cf) = NULL;
   hypre_IntArrayDestroy(wrap_cf);

   return hypre_error_flag;
}

/* Build A_FF matrix from A given a CF_marker array */
HYPRE_Int
hypre_MGRBuildAff( hypre_ParCSRMatrix   *A,
                   HYPRE_Int            *CF_marker,
                   HYPRE_Int             debug_flag,
                   hypre_ParCSRMatrix  **A_ff_ptr )
{
   HYPRE_Int i;
   HYPRE_Int local_numrows = hypre_CSRMatrixNumRows(hypre_ParCSRMatrixDiag(A));
   /* create a copy of the CF_marker array and switch C-points to F-points */
   HYPRE_Int *CF_marker_copy = hypre_CTAlloc(HYPRE_Int, local_numrows, HYPRE_MEMORY_HOST);

#if 0
#ifdef HYPRE_USING_OPENMP
   #pragma omp parallel for private(i) HYPRE_SMP_SCHEDULE
#endif
#endif
   for (i = 0; i < local_numrows; i++)
   {
      CF_marker_copy[i] = -CF_marker[i];
   }

   hypre_MGRGetSubBlock(A, CF_marker_copy, CF_marker_copy, debug_flag, A_ff_ptr);

   /* Free copy of CF marker */
   hypre_TFree(CF_marker_copy, HYPRE_MEMORY_HOST);
   return (0);
}

/*********************************************************************************
 * This routine assumes that the 'toVector' is larger than the 'fromVector' and
 * the CF_marker is of the same length as the toVector. There must be n 'point_type'
 * values in the CF_marker, where n is the length of the 'fromVector'.
 * It adds the values of the 'fromVector' to the 'toVector' where the marker is the
 * same as the 'point_type'
 *********************************************************************************/
HYPRE_Int
hypre_MGRAddVectorP ( hypre_IntArray *CF_marker,
                      HYPRE_Int        point_type,
                      HYPRE_Real       a,
                      hypre_ParVector  *fromVector,
                      HYPRE_Real       b,
                      hypre_ParVector  **toVector )
{
   hypre_Vector    *fromVectorLocal = hypre_ParVectorLocalVector(fromVector);
   HYPRE_Real      *fromVectorData  = hypre_VectorData(fromVectorLocal);
   hypre_Vector    *toVectorLocal   = hypre_ParVectorLocalVector(*toVector);
   HYPRE_Real      *toVectorData    = hypre_VectorData(toVectorLocal);
   HYPRE_Int       *CF_marker_data = hypre_IntArrayData(CF_marker);

   //HYPRE_Int       n = hypre_ParVectorActualLocalSize(*toVector);
   HYPRE_Int       n = hypre_IntArraySize(CF_marker);
   HYPRE_Int       i, j;

   j = 0;
   for (i = 0; i < n; i++)
   {
      if (CF_marker_data[i] == point_type)
      {
         toVectorData[i] = b * toVectorData[i] + a * fromVectorData[j];
         j++;
      }
   }
   return 0;
}

/*************************************************************************************
 * This routine assumes that the 'fromVector' is larger than the 'toVector' and
 * the CF_marker is of the same length as the fromVector. There must be n 'point_type'
 * values in the CF_marker, where n is the length of the 'toVector'.
 * It adds the values of the 'fromVector' where the marker is the
 * same as the 'point_type' to the 'toVector'
 *************************************************************************************/
HYPRE_Int
hypre_MGRAddVectorR ( hypre_IntArray *CF_marker,
                      HYPRE_Int        point_type,
                      HYPRE_Real       a,
                      hypre_ParVector  *fromVector,
                      HYPRE_Real       b,
                      hypre_ParVector  **toVector )
{
   hypre_Vector    *fromVectorLocal = hypre_ParVectorLocalVector(fromVector);
   HYPRE_Real      *fromVectorData  = hypre_VectorData(fromVectorLocal);
   hypre_Vector    *toVectorLocal   = hypre_ParVectorLocalVector(*toVector);
   HYPRE_Real      *toVectorData    = hypre_VectorData(toVectorLocal);
   HYPRE_Int       *CF_marker_data = hypre_IntArrayData(CF_marker);

   //HYPRE_Int       n = hypre_ParVectorActualLocalSize(*toVector);
   HYPRE_Int       n = hypre_IntArraySize(CF_marker);
   HYPRE_Int       i, j;

   j = 0;
   for (i = 0; i < n; i++)
   {
      if (CF_marker_data[i] == point_type)
      {
         toVectorData[j] = b * toVectorData[j] + a * fromVectorData[i];
         j++;
      }
   }
   return 0;
}

/*
HYPRE_Int
hypre_MGRBuildAffRAP( MPI_Comm comm, HYPRE_Int local_num_variables, HYPRE_Int num_functions,
  HYPRE_Int *dof_func, HYPRE_Int *CF_marker, HYPRE_Int **coarse_dof_func_ptr, HYPRE_BigInt **coarse_pnts_global_ptr,
  hypre_ParCSRMatrix *A, HYPRE_Int debug_flag, hypre_ParCSRMatrix **P_f_ptr, hypre_ParCSRMatrix **A_ff_ptr )
{
  HYPRE_Int *CF_marker_copy = hypre_CTAlloc(HYPRE_Int,  local_num_variables, HYPRE_MEMORY_HOST);
  HYPRE_Int i;
  for (i = 0; i < local_num_variables; i++) {
    CF_marker_copy[i] = -CF_marker[i];
  }

  hypre_BoomerAMGCoarseParms(comm, local_num_variables, 1, NULL, CF_marker_copy, coarse_dof_func_ptr, coarse_pnts_global_ptr);
  hypre_MGRBuildP(A, CF_marker_copy, (*coarse_pnts_global_ptr), 0, debug_flag, P_f_ptr);
  hypre_BoomerAMGBuildCoarseOperator(*P_f_ptr, A, *P_f_ptr, A_ff_ptr);

  hypre_TFree(CF_marker_copy, HYPRE_MEMORY_HOST);
  return 0;
}
*/

/* Get pointer to coarse grid matrix for MGR solver */
HYPRE_Int
hypre_MGRGetCoarseGridMatrix( void *mgr_vdata, hypre_ParCSRMatrix **RAP )
{
   hypre_ParMGRData  *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   if (!mgr_data)
   {
      hypre_error_in_arg(1);
      return hypre_error_flag;
   }
   if (mgr_data -> RAP == NULL)
   {
      hypre_error_w_msg(HYPRE_ERROR_GENERIC,
                        " Coarse grid matrix is NULL. Please make sure MGRSetup() is called \n");
      return hypre_error_flag;
   }
   *RAP = mgr_data->RAP;

   return hypre_error_flag;
}

/* Get pointer to coarse grid solution for MGR solver */
HYPRE_Int
hypre_MGRGetCoarseGridSolution( void *mgr_vdata, hypre_ParVector **sol )
{
   hypre_ParMGRData  *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   if (!mgr_data)
   {
      hypre_error_in_arg(1);
      return hypre_error_flag;
   }
   if (mgr_data -> U_array == NULL)
   {
      hypre_error_w_msg(HYPRE_ERROR_GENERIC,
                        " MGR solution array is NULL. Please make sure MGRSetup() and MGRSolve() are called \n");
      return hypre_error_flag;
   }
   *sol = mgr_data->U_array[mgr_data->num_coarse_levels];

   return hypre_error_flag;
}

/* Get pointer to coarse grid solution for MGR solver */
HYPRE_Int
hypre_MGRGetCoarseGridRHS( void *mgr_vdata, hypre_ParVector **rhs )
{
   hypre_ParMGRData  *mgr_data = (hypre_ParMGRData*) mgr_vdata;

   if (!mgr_data)
   {
      hypre_error_in_arg(1);
      return hypre_error_flag;
   }
   if (mgr_data -> F_array == NULL)
   {
      hypre_error_w_msg(HYPRE_ERROR_GENERIC,
                        " MGR RHS array is NULL. Please make sure MGRSetup() and MGRSolve() are called \n");
      return hypre_error_flag;
   }
   *rhs = mgr_data->F_array[mgr_data->num_coarse_levels];

   return hypre_error_flag;
}

/* Print coarse grid linear system (for debugging)*/
HYPRE_Int
hypre_MGRPrintCoarseSystem( void *mgr_vdata, HYPRE_Int print_flag)
{
   hypre_ParMGRData  *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   mgr_data->print_coarse_system = print_flag;

   return hypre_error_flag;
}

/* Print solver params */
HYPRE_Int
hypre_MGRWriteSolverParams(void *mgr_vdata)
{
   hypre_ParMGRData  *mgr_data = (hypre_ParMGRData*) mgr_vdata;
   HYPRE_Int i, j;
   HYPRE_Int max_num_coarse_levels = (mgr_data -> max_num_coarse_levels);
   hypre_printf("MGR Setup parameters: \n");
   hypre_printf("Block size: %d\n", (mgr_data -> block_size));
   hypre_printf("Max number of coarse levels: %d\n", (mgr_data -> max_num_coarse_levels));
   //   hypre_printf("Relax type: %d\n", (mgr_data -> relax_type));
   hypre_printf("Set non-Cpoints to F-points: %d\n", (mgr_data -> set_non_Cpoints_to_F));
   hypre_printf("Set Cpoints method: %d\n", (mgr_data -> set_c_points_method));
   for (i = 0; i < max_num_coarse_levels; i++)
   {
      hypre_printf("Lev = %d, Interpolation type: %d\n", i, (mgr_data -> interp_type)[i]);
      hypre_printf("Lev = %d, Restriction type: %d\n", i, (mgr_data -> restrict_type)[i]);
      hypre_printf("Lev = %d, F-relaxation type: %d\n", i, (mgr_data -> Frelax_type)[i]);
      hypre_printf("lev = %d, Number of relax sweeps: %d\n", i, (mgr_data -> num_relax_sweeps)[i]);
      hypre_printf("Lev = %d, Use non-Galerkin coarse grid: %d\n", i,
                   (mgr_data -> mgr_coarse_grid_method)[i]);
      HYPRE_Int lvl_num_coarse_points = (mgr_data -> block_num_coarse_indexes)[i];
      hypre_printf("Lev = %d, Number of Cpoints: %d\n", i, lvl_num_coarse_points);
      hypre_printf("Cpoints indices: ");
      for (j = 0; j < lvl_num_coarse_points; j++)
      {
         if ((mgr_data -> block_cf_marker)[i][j] == 1)
         {
            hypre_printf("%d ", j);
         }
      }
      hypre_printf("\n");
   }
   hypre_printf("Number of Reserved Cpoints: %d\n", (mgr_data -> reserved_coarse_size));
   hypre_printf("Keep reserved Cpoints to level: %d\n", (mgr_data -> lvl_to_keep_cpoints));

   hypre_printf("\n MGR Solver Parameters: \n");
   hypre_printf("Number of interpolation sweeps: %d\n", (mgr_data -> num_interp_sweeps));
   hypre_printf("Number of restriction sweeps: %d\n", (mgr_data -> num_restrict_sweeps));
   if (mgr_data -> level_smooth_type != NULL)
   {
      hypre_printf("Global smoother type: %d\n", (mgr_data -> level_smooth_type)[0]);
      hypre_printf("Number of global smoother sweeps: %d\n", (mgr_data -> level_smooth_iters)[0]);
   }
   hypre_printf("Max number of iterations: %d\n", (mgr_data -> max_iter));
   hypre_printf("Stopping tolerance: %e\n", (mgr_data -> tol));
   hypre_printf("Use default coarse grid solver: %d\n", (mgr_data -> use_default_cgrid_solver));
   /*
      if ((mgr_data -> fsolver_mode) >= 0)
      {
         hypre_printf("Use AMG solver for full AMG F-relaxation: %d\n", (mgr_data -> fsolver_mode));
      }
   */
   return hypre_error_flag;
}

#ifdef HYPRE_USING_DSUPERLU
void *
hypre_MGRDirectSolverCreate()
{
   //   hypre_DSLUData *dslu_data = hypre_CTAlloc(hypre_DSLUData, 1, HYPRE_MEMORY_HOST);
   //   return (void *) dslu_data;
   return NULL;
}

HYPRE_Int
hypre_MGRDirectSolverSetup( void                *solver,
                            hypre_ParCSRMatrix  *A,
                            hypre_ParVector     *f,
                            hypre_ParVector     *u )
{
   HYPRE_Int ierr;
   ierr = hypre_SLUDistSetup( solver, A, 0);

   return ierr;
}
HYPRE_Int
hypre_MGRDirectSolverSolve( void                *solver,
                            hypre_ParCSRMatrix  *A,
                            hypre_ParVector     *f,
                            hypre_ParVector     *u )
{
   hypre_SLUDistSolve(solver, f, u);

   return hypre_error_flag;
}

HYPRE_Int
hypre_MGRDirectSolverDestroy( void *solver )
{
   hypre_SLUDistDestroy(solver);

   return hypre_error_flag;
}
#endif
