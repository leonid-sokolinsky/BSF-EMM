/*==============================================================================
Project: LiFe - New Linear Programming Solvers - New Linear Programming Solvers
Theme: AlEM method - Along Edges Movement (No MPI)
Module: Problems05-1.h (LP problems of dimension 5 with 1 randome inequality: LP-Rnd-Problems Set)
Prefix: PP
Authors: Alexandr E. Zhulev & Leonid B. Sokolinsky
This include file is part of Problem-Parameters.h
LP problems were obtained using BSF-LPP-Generator.
Initial surface points for these problems were calculated using BSF-Apex-Quest.
==============================================================================*/
#pragma once

//=========================== Method Parameters =========================
#define PP_KK							500		// Maximal number of edges that include surface point (compilator limit: 2 147 483 647)
#define PP_EPS_ZERO						1E-8	// Accuracy for comparison with zero
#define PP_EPS_U0_ROUND					0		// Precision of rounding starting point
#define PP_EPS_TINY_PPROJ_VEC			1E-10	// Tiny pseudoprojecting vector
#define PP_EPS_POINT_IN_HALFSPACE		1E-10	// Precision for point to be in halfspace
#define PP_OBJECTIVE_VECTOR_LENGTH		100000	// Length of Objective Vector
#define PP_EPS_MAKE_H_PLANE_LIST		1E-5	// Precision for MakeHyperplaneList()
#define PP_PROBE_LENGTH					1		// length of probe shift
//-------------------------------------------------------------------------------
#define PP_MAX_B_NO_CORRECT				200		// Maximum b that does not require correction
#define PP_EPS_POINT_IN_RND_HALFSPACE	1E-6	// Precision for random inequality in PointInPolytope()
//-------------------------------------------------------------------------------

/*============================== simple1 LP problem =============================*
#define PP_PROBLEM_NAME	"simple1"
#define PP_M 4		// Number of equations (number of rows in *.mtx)
#define PP_N 7		// Number of variables (number of cols in *.mtx)
#define PP_MAX_OBJ_VALUE 		55000
//-------------------------------------------------------------------------------

/*============================== simple1.1 LP problem =============================*
// Simple LP problem with alternating objective function
#define PP_PROBLEM_NAME	"simple1.1"
#define PP_M 4		// Number of equations (number of rows in *.mtx)
#define PP_N 7		// Number of variables (number of cols in *.mtx)
#define PP_MAX_OBJ_VALUE 		40000
//-------------------------------------------------------------------------------

/*============================== simple2 LP problem =============================*/
// Simple LP problem & x_4=200; x_2>=110; x_1<=190
#define PP_PROBLEM_NAME	"simple2"
#define PP_M 5		// Number of equations (number of rows in *.mtx)
#define PP_N 8		// Number of variables (number of cols in *.mtx)
#define PP_MAX_OBJ_VALUE 		63500
//-------------------------------------------------------------------------------

/*============================== simple3 LP problem =============================*
#define PP_PROBLEM_NAME	"simple3"
#define PP_M 6		// Number of equations (number of rows in *.mtx)
#define PP_N 8		// Number of variables (number of cols in *.mtx)
#define PP_MAX_OBJ_VALUE 		60000
//-------------------------------------------------------------------------------

/*============================== simple1min LP problem =============================*
#define PP_PROBLEM_NAME	"simple1min"
#define PP_M 5		// Number of equations (number of rows in *.mtx)
#define PP_N 8		// Number of variables (number of cols in *.mtx)
#define PP_MAX_OBJ_VALUE 		-5000
//-------------------------------------------------------------------------------

/*============================== simple_zcv LP problem =============================*
#define PP_PROBLEM_NAME	"simple_zcv"
#define PP_M 4		// Number of equations (number of rows in *.mtx)
#define PP_N 7		// Number of variables (number of cols in *.mtx)
#define PP_MAX_OBJ_VALUE 		50000
//-------------------------------------------------------------------------------

/*============================== simple_lcv LP problem =============================*
#define PP_PROBLEM_NAME	"simple_lcv"
#define PP_M 4		// Number of equations (number of rows in *.mtx)
#define PP_N 7		// Number of variables (number of cols in *.mtx)
#define PP_MAX_OBJ_VALUE 		50000.2
//-------------------------------------------------------------------------------

/*============================== simple_lcv_neg LP problem =============================*
#define PP_PROBLEM_NAME	"simple_lcv_neg"
#define PP_M 4		// Number of equations (number of rows in *.mtx)
#define PP_N 7		// Number of variables (number of cols in *.mtx)
#define PP_MAX_OBJ_VALUE 		49998
//-------------------------------------------------------------------------------

/*===============================================================================*/