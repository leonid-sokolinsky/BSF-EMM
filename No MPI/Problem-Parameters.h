/*==============================================================================
Project: LiFe - New Linear Programming Solvers
Theme: AlEM - Along Edges Movement Method (No MPI)
Module: Problem-Parameters.h (Problem Parameters)
Prefix: PP
Authors: Alexander E. Zhulev & Leonid B. Sokolinsky
This source code has been produced with using BSF-skeleton
==============================================================================*/

/*----------------------------- Problems -------------------------------*/

/**/
#include "../_Problems05-1.h"
#define PP_PATH "D:/YandexDisk/_private/Programming/Set-of-LP-Problems/Rnd-LP/"

/**
#include "../_Problems10-1.h"
#define PP_PATH "D:/YandexDisk/_private/Programming/Set-of-LP-Problems/Rnd-LP/"

/**
#include "../_Problems15-1.h"
#define PP_PATH "D:/YandexDisk/_private/Programming/Set-of-LP-Problems/Rnd-LP/"

/**
#include "../_Problems20-1.h"
#define PP_PATH "D:/YandexDisk/_private/Programming/Set-of-LP-Problems/Rnd-LP/"

/**
#include "../_Problems-Klee-Minty-Cube.h"
#define PP_PATH "D:/YandexDisk/_private/Programming/Set-of-LP-Problems/Klee-Minty-Cube/"

/**
#include "../_Problems-Miscellaneous.h"
#define PP_PATH "D:/YandexDisk/_private/Programming/Set-of-LP-Problems/Miscellaneous-LP/"

/*---------------------------------------------------------------------*/

#define PP_METHOD_NAME "AlEM"

//-------------------------- Compilation Modes -----------------------
#define PP_DEBUG
#define PP_MATRIX_OUTPUT
#define PP_SAVE_RESULT
#define PP_NO_MPI
//#define PP_PATH "Problems/"

//================================ Problem Paramrters ===========================
#define PP_MM					(2*PP_M+2*PP_N)	// Maximal number of inequalities
#define PP_MAX_ITER_COUNT		10000000000		// Maximal count of iterations
#define PP_DBL_MAX				1E+308			// Highest value
//-------------------------- Input/Outpoot Parameters ---------------------------
#define PP_OUTPUT_LIMIT	30	// Number of Elements to output
#define PP_SETW 16
//------------------------- Matrix format ----------------
#define PP_INFINITY			1E+308		// Highest bound in *_hi.mtx
#define PP_MTX_PREFIX		"lp_"
#define PP_MTX_POSTFIX_A	".mtx"
#define PP_MTX_POSTFIX_B	"_b.mtx"
#define PP_MTX_POSTFIX_C	"_c.mtx"
#define PP_MTX_POSTFIX_HI	"_hi.mtx"
#define PP_MTX_POSTFIX_LO	"_lo.mtx"
#define PP_MTX_POSTFIX_SO	"_so.mtx"	// Solution point
#define PP_MTX_POSTFIX_U0	"_u0.mtx"	// Starting point
//-------------------------- Jobs  -----------------------
#define PP_JOB_GET_BEST_DIRECTION	0 
//------------- Vector Projection Onto Halfspace exit codes -------------
#define PP_EXITCODE_DEGENERATE_INEQUALITY		0
#define PP_EXITCODE_ON_HYPERPLANE				1
#define PP_EXITCODE_INSIDE_HALFSPACE			2
#define PP_EXITCODE_PARALLEL					4
#define PP_EXITCODE_RECESSIVE					5
#define PP_EXITCODE_NONDEGENERATE_PROJECTING	9