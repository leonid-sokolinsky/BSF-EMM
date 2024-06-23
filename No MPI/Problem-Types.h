/*==============================================================================
Project: LiFe - New Linear Programming Solvers
Theme: AlEM - Along Edges Movement Method (No MPI)
Module: Problem-Types.h (BSF Types)
Prefix: PT
Authors: Alexander E. Zhulev & Leonid B. Sokolinsky
This source code has been produced with using BSF-skeleton
==============================================================================*/			
#pragma once
#include "Problem-Include.h"		// Problem "Include" Files
#include "Problem-Parameters.h"		// Problem Parameters 
//=========================== Problem Types =========================
typedef double PT_matrix_T[PP_MM][PP_N];
typedef double PT_vector_T[PP_N];
typedef double	PT_column_T[PP_MM];