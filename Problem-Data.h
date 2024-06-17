/*==============================================================================
Project: LiFe - New Linear Programming Solvers
Theme: AlEM method - Along Edges Movement (MPI)
Module: Problem-Data.h (Problem Data)
Prefix: PD
Authors: Alexandr E. Zhulev & Leonid B. Sokolinsky
This source code has been produced with using BSF-skeleton
==============================================================================*/
#include "Problem-Types.h"		// Problem Parameters 
using namespace std;
//========================== Problem variables ====================================
static int PD_m;					// Current number of inequalities
static int PD_n;					// Space dimension
static int PD_mh;					// Number of hyperplanes that include surface point
static int PD_ma;					// Number of hyperplanes used for pseudoprojection
static int PD_me;					// Number of edges that include surface point
static int PD_iterNo;				// Number of iterations
static double PD_objF_initialValue; // Initial value of objective function
static double PD_objF_u;			// Current value of objective function
static double PD_shiftLength;		// Shift length
static bool PD_TWIDDLE_done;		// Done indicator for TWIDDLE algorithm
static int PD_TWIDDLE_nextEdgeI;	// Next index of edge in TWIDDLE algorithm
static int PD_TWIDDLE_x;			// Auxiliary TWIDDLE variable
static int PD_TWIDDLE_y;			// Auxiliary TWIDDLE variable
static int PD_TWIDDLE_z;			// Auxiliary TWIDDLE variable
//========================== Problem structures ====================================
static double PD_A[PP_MM][PP_N];	// Matrix of coefficients of inequalities
static PT_column_T PD_b;			// Column of the constant terms of the system Ax <= PD_b
static PT_vector_T PD_c;			// Gradient of Objective Function
static PT_vector_T PD_e_c;			// PD_e_c = PD_c / ||PD_c||
static PT_vector_T PD_u;			// Current surface point
static PT_vector_T PD_hi;			// Higher bound
static PT_vector_T PD_lo;			// Lower bound
static PT_column_T PD_norm_a;		// Column of norms of matrix rows
static PT_vector_T PD_objVector;	// Used for pseudoprojecting
static int PD_index_includingHyperplanes[PP_MM];	// Index of hyperplanes that include surface point u
static int PD_edgeCodeList[PP_KK];					// Edge codes
static int PD_index_activeHyperplanes[PP_N - 1];	// Index of hyperplanes used for pseudoprojection
static int PD_TWIDDLE_p[PP_MM + 2];					// Auxiliary array for TWIDDLE algorithm
//========================== Input/Output ====================================
static string PD_problemName;