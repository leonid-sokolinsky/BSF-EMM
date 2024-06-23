/*==============================================================================
Project: LiFe - New Linear Programming Solvers
Theme: AlEM - Along Edges Movement method (No MPI)
Module: Problems05-1.h (LP problems of dimension 5 with 1 randome inequality: LP-Rnd-Problems Set)
Prefix: PP
Authors: Alexander E. Zhulev & Leonid B. Sokolinsky
This include file is part of Problem-Parameters.h
LP problems were obtained using BSF-LPP-Generator.
Initial surface points for these problems were calculated using BSF-Apex-Quest.
==============================================================================*/
#pragma once

//=========================== Problem Parameters ========================
// PP_OBJECTIVE_VECTOR_LENGTH - direct dependence on dimension PD_n.
// P_EPS_ZERO - inverse dependence on PP_OBJECTIVE_VECTOR_LENGTH.
// PP_EPS_PROJECTION_ROUND - inverse dependence on PP_OBJECTIVE_VECTOR_LENGTH. 
//						This parameter affects terminate condition when 
//						calculating pseudoprojection.
//-----------------------------------------------------------------------
#define PP_EPS_ZERO					1E-10		// Accuracy for comparison with zero
#define PP_EPS_POINT_IN_HALFSPACE	1E-9		// Precision for point to be in halfspace
#define PP_EPS_MOVING_ON_POLYTOPE	1E-12	// Precision for moving on polytope (affects Shift = 0)
#define PP_EPS_PROJECTION_ROUND		PP_EPS_ZERO	// Precision of rounding pseudoprojecting vectors
#define PP_OBJECTIVE_VECTOR_LENGTH	1E+9		// Length of Objective Vector
//-----------------------------------------------------------------------
#define PP_KK						5		// Maximal number of edges that include surface point (compilator limit: 2 147 483 647)
#define PP_M						6		// Number of equations (number of rows in *.mtx)
#define PP_N						11		// Number of variables (number of cols in *.mtx)
#define PP_MAX_PROJECTING_ITER		1E+7	// Maximum acceptable number of iterations in PseudoprojectionOnFace()
#define PP_PROBE_LENGTH				0.1		// Length of probe shift
//-------------------------------------------------------------------------------

/*============================== rnd5-0 LP problem ==============================*
// Exact solution:	100   200   200   200   200
#define PP_PROBLEM_NAME	"rnd5-0"
#define PP_MAX_OBJ_VALUE 		2900
//-------------------------------------------------------------------------------

/*============================== rnd5-1-1 LP problem ============================*
#define PP_PROBLEM_NAME	"rnd5-1-1"
#define PP_MAX_OBJ_VALUE 2584.349489709195
//-------------------------------------------------------------------------------

/*============================== rnd5-1-2 LP problem ============================*
#define PP_PROBLEM_NAME	"rnd5-1-2"
#define PP_MAX_OBJ_VALUE 2657.525612539954
//-------------------------------------------------------------------------------

/*============================== rnd5-1-3 LP problem ============================*
#define PP_PROBLEM_NAME	"rnd5-1-3"
#define PP_MAX_OBJ_VALUE 2424.919153811822
//-------------------------------------------------------------------------------

/*============================== rnd5-1-4 LP problem ============================*
#define PP_PROBLEM_NAME	"rnd5-1-4"
#define PP_MAX_OBJ_VALUE 2300.11275869781

//-------------------------------------------------------------------------------

/*============================== rnd5-1-5 LP problem ============================*
#define PP_PROBLEM_NAME	"rnd5-1-5"
#define PP_MAX_OBJ_VALUE 2626.473236206933
//-------------------------------------------------------------------------------

/*============================== rnd5-1-6 LP problem ============================*/
#define PP_PROBLEM_NAME	"rnd5-1-6"
#define PP_MAX_OBJ_VALUE 2675.351994186427
//-------------------------------------------------------------------------------

/*===============================================================================*/