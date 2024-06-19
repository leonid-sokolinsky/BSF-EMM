﻿/*==============================================================================
Project: LiFe - New Linear Programming Solvers
Theme: AlEM method - Along Edges Movement (No MPI)
Module: Problem-bsfCode.cpp (Implementation of Problem Code)
Prefix:	PC_bsf	- BSF Predefined Problem Functions
		CF		- Common Functionc
		PF		- Private Functions
Authors: Alexandr E. Zhulev & Leonid B. Sokolinsky
This source code has been produced with using BSF-skeleton
==============================================================================*/
#include "Problem-Data.h"			// Problem Types 
#include "Problem-Forwards.h"		// Problem Function Forwards
#include "Problem-bsfParameters.h"	// BSF-skeleton parameters
#include "BSF-SkeletonVariables.h"	// Skeleton Variables
using namespace std;

void PC_bsf_CopyParameter(PT_bsf_parameter_T parameterIn, PT_bsf_parameter_T* parameterOutP) {
	SF_Vector_Copy(parameterIn.x, parameterOutP->x);
}

void PC_bsf_Init(bool* success) {
	PD_problemName = PP_PROBLEM_NAME;

	*success = SF_MTX_Load__Problem();
	if (*success == false)
		return;

	*success = SF_MTX_LoadPoint(PD_u, PP_MTX_POSTFIX_U0);
	if (*success == false)
		return;

	SF_MakeColumnOfNorms(PD_A, PD_norm_a);

	if (!SF_PointBelongsPolytope(PD_u, PP_EPS_ZERO)) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Starting point does not belong to the feasible polytope with precision PP_EPS_ZERO = "
			<< PP_EPS_ZERO << "!!!\n";
		*success = false;
		return;
	}
	PD_iterNo = 0;
	SF_Vector_MakeLike(PD_c, 1, PD_e_c);
	PF_PreparationForIteration(PD_u);
}

void PC_bsf_IterOutput(PT_bsf_reduceElem_T* reduceResult, int reduceCounter, PT_bsf_parameter_T parameter,
	double elapsedTime, int nextJob) {

	cout << "# " << BSF_sv_iterCounter << "\tTime " << round(elapsedTime);
	cout << "\tx =";
	SF_Print_Vector(parameter.x);
	cout << "\tF(x) = " << setw(PP_SETW) << SF_ObjF(parameter.x);
}

void PC_bsf_IterOutput_1(PT_bsf_reduceElem_T_1* reduceResult, int reduceCounter, PT_bsf_parameter_T parameter,
	double elapsedTime, int nextJob)
{
	// Not used
}

void PC_bsf_IterOutput_2(PT_bsf_reduceElem_T_2* reduceResult, int reduceCounter, PT_bsf_parameter_T parameter,
	double elapsedTime, int nextJob)
{
	// Not used
}

void PC_bsf_IterOutput_3(PT_bsf_reduceElem_T_3* reduceResult, int reduceCounter, PT_bsf_parameter_T parameter,
	double elapsedTime, int nextJob)
{
	// Not used
}

void PC_bsf_JobDispatcher(PT_bsf_parameter_T* parameter, int* job, bool* exit, double t) {
	// Not used
}

void PC_bsf_MainArguments(int argc, char* argv[]) {
	// Not used
}

void PC_bsf_MapF(PT_bsf_mapElem_T* mapElem, PT_bsf_reduceElem_T* reduceElem, int* success) {
	int edgeCode = *mapElem->edgeCode;
	PT_vector_T u;		// current surface point
	PT_vector_T v;		// v = u + PD_objVector (objVector = PP_OBJECTIVE_VECTOR_LENGTH*e_c)
	PT_vector_T w;		// pseudiprojection of v
	double objF_w = -PP_DBL_MAX; // F(w)

	if (edgeCode == -1) {
		SF_Vector_Zeroing((*reduceElem).d);
		reduceElem->objF_p = -PP_DBL_MAX;
		reduceElem->edgeCode = -1;
		return;
	}

	/*DEBUG PC_bsf_MapF**
#ifdef PP_DEBUG
	cout << "------------------------------------ Map(" << PF_MAP_LIST_INDEX << ") ------------------------------------" << endl;
#endif // PP_DEBUG /**/

// Condition for breakpoint: PD_iterNo == 2 && (BSF_sv_addressOffset + BSF_sv_numberInSublist == 2)
	SF_Vector_Copy(BSF_sv_parameter.x, u);
	double objF_u = SF_ObjF(u);
	reduceElem->edgeCode = edgeCode;

	PF_TWIDDLE_CodeToSubset(edgeCode, PD_index_includingHyperplanes, PD_index_activeHyperplanes, PD_mh, PD_ma);

	/*DEBUG PC_bsf_MapF**
	#ifdef PP_DEBUG
		cout << "Edge code: " << edgeCode << ".\tHyperplanes: {";
		for (int i = 0; i < PD_ma - 1; i++) {
			cout << PD_index_activeHyperplanes[i] << ", ";
		}
		cout << PD_index_activeHyperplanes[PD_ma - 1] << "}.\n";
	#endif // PP_DEBUG /**/

	SF_Vector_MultiplyByNumber(PD_e_c, PP_OBJECTIVE_VECTOR_LENGTH, PD_objVector);

	SF_Vector_Addition(u, PD_objVector, v);

	PF_PseudoprojectionOnEdge(v, w, PP_EPS_PROJECTION_ROUND, success);
	if (!*success) {
		cout << "\n\nProcess " << BSF_sv_mpiRank
			<< ". Error in PC_bsf_MapF: Exceeded the maximum number of iterations when calculating pseudoprojection (PP_MAX_PROJECTING_ITER = "
			<< PP_MAX_PROJECTING_ITER << "). It is impossible to calculate Map function for element "
			<< BSF_sv_addressOffset + BSF_sv_numberInSublist << "!\n Perhaps you should decrease parameter PP_EPS_PROJECTION_ROUND.";
		return;
	}

	SF_Vector_Round(w, PP_EPS_PROJECTION_ROUND * 10);

	objF_w = SF_ObjF(w);

	SF_Vector_Subtraction(w, u, (*reduceElem).d);

	double norm_d = SF_Vector_Norm((*reduceElem).d);
	if (norm_d < PP_EPS_ZERO) {
		/*DEBUG PC_bsf_MapF**
		#ifdef PP_DEBUG
				cout << "\t\t\t\t\t\t\t\t\t\t\t\t\t||d|| = ||w-u|| < PP_EPS_ZERO ===>>> movement is impossible.\n";
		#endif // PP_DEBUG /**/
		SF_Vector_Zeroing((*reduceElem).d);
		reduceElem->objF_p = objF_u;
		return;
	}

	SF_Vector_MultiplyEquals((*reduceElem).d, PP_PROBE_LENGTH / norm_d);
	SF_Vector_Round((*reduceElem).d, PP_EPS_PROJECTION_ROUND);

	PT_vector_T p;
	SF_Vector_Addition(u, (*reduceElem).d, p);

	if (!SF_PointBelongsPolytope(p, PP_EPS_POINT_IN_HALFSPACE)) {
		/*DEBUG PC_bsf_MapF**
		#ifdef PP_DEBUG
				cout << "Shifted point p = ";
				SF_Print_Vector(p);
				cout << "\tnot in feasible polytope ===>>> movement is impossible." << endl;
		#endif // PP_DEBUG /**/
		SF_Vector_Zeroing((*reduceElem).d);
		reduceElem->objF_p = objF_u;
		return;
	}

	reduceElem->objF_p = SF_ObjF(p);

	if (SF_RelativeError(objF_u, reduceElem->objF_p) < PP_EPS_ZERO) {
		/*DEBUG PC_bsf_MapF**
		#ifdef PP_DEBUG
				cout << "u =\t    ";
				SF_Print_Vector(u);
				cout << "\tF(u) = " << setw(PP_SETW) << objF_u << endl;
				cout << "|F(u1)-F(u2)|/|F(u1)| = " << SF_RelativeError(objF_u, reduceElem->objF_p) << " < PP_EPS_ZERO = " << PP_EPS_ZERO << " ===>>> movement is impossible.\n";
		#endif // PP_DEBUG /**/

		SF_Vector_Zeroing((*reduceElem).d);
		reduceElem->objF_p = objF_u;
		return;
	}

	if (reduceElem->objF_p < objF_u) {
		/*DEBUG PC_bsf_MapF**
		#ifdef PP_DEBUG
				cout << "u =\t    ";
				SF_Print_Vector(u);
				cout << "\tF(w) = " << setw(PP_SETW) << reduceElem->objF_p << " < F(u) = " << objF_u << " ===>>> movement is impossible.\n";
		#endif // PP_DEBUG /**/
		SF_Vector_Zeroing((*reduceElem).d);
		reduceElem->objF_p = objF_u;
		return;
	}

	/*DEBUG PC_bsf_MapF**
	#ifdef PP_DEBUG
		cout << "d = ";
		SF_Print_Vector((*reduceElem).d);
		cout << endl;
		cout << "Shifted point p = ";
		SF_Print_Vector(p);
		cout << "\tF(p) ="
			<< setw(PP_SETW) << reduceElem->objF_p << "\t\t---> Movement is possible." << endl;
	#endif // PP_DEBUG /**/
} // end PC_bsf_MapF

void PC_bsf_MapF_1(PT_bsf_mapElem_T* mapElem, PT_bsf_reduceElem_T_1* reduceElem, int* success) {
	// Not used
}

void PC_bsf_MapF_2(PT_bsf_mapElem_T* mapElem, PT_bsf_reduceElem_T_2* reduceElem, int* success) {
	// Not used
}

void PC_bsf_MapF_3(PT_bsf_mapElem_T* mapElem, PT_bsf_reduceElem_T_3* reduceElem, int* success) {
	// Not used
}

void PC_bsf_MapInit(PT_bsf_parameter_T parameter) {
	PF_PreparationForIteration(parameter.x);
}

void PC_bsf_ParametersOutput(PT_bsf_parameter_T parameter) {
	cout << "=================================================== EMM ====================================================" << endl;
	cout << "Problem name: " << PD_problemName << endl;
	cout << "No MPI" << endl;
#ifdef PP_BSF_OMP
#ifdef PP_BSF_NUM_THREADS
	cout << "Number of Threads: " << PP_BSF_NUM_THREADS << endl;
#else
	cout << "Number of Threads: " << omp_get_num_procs() << endl;
#endif // PP_BSF_NUM_THREADS
#else
	cout << "OpenMP is turned off!" << endl;
#endif // PP_BSF_OMP

#ifdef PP_BSF_FRAGMENTED_MAP_LIST
	cout << "Map List is Fragmented." << endl;
#else
	cout << "Map List is not Fragmented." << endl;
#endif
	cout << "Before conversion: m =\t" << PP_M << "\tn = " << PP_N << endl;
	cout << "After conversion:  m =\t" << PD_m << "\tn = " << PD_n << endl;
	cout << "PP_EPS_ZERO\t" << PP_EPS_ZERO << endl;
	cout << "PP_EPS_POINT_IN_HALFSPACE\t" << PP_EPS_POINT_IN_HALFSPACE << endl;
	cout << "PP_EPS_MOVING_ON_POLYTOPE\t" << PP_EPS_MOVING_ON_POLYTOPE << endl;
	cout << "PP_EPS_PROJECTION_ROUND\t\t" << PP_EPS_PROJECTION_ROUND << endl;
	cout << "PP_OBJECTIVE_VECTOR_LENGTH\t" << PP_OBJECTIVE_VECTOR_LENGTH << endl;
	cout << "PP_PROBE_LENGTH\t\t\t" << PP_PROBE_LENGTH << endl;
	cout << "--------------- Data ---------------\n";
#ifdef PP_MATRIX_OUTPUT
	cout << "------- Matrix PD_A & Column PD_b -------" << endl;
	SF_Print_Inequalities();
#endif // PP_MATRIX_OUTPUT
	cout << "Obj Function:\t";
	SF_Print_Vector(PD_c);
	cout << endl;
	cout << "u0 =\t\t";
	SF_Print_Vector(PD_u); cout << "\tF(x) = " << setw(PP_SETW) << SF_ObjF(PD_u) << endl;

#ifdef PP_DEBUG
	if (!SF_PointBelongsPolytope(PD_u, PP_EPS_POINT_IN_HALFSPACE))
		cout << "u0 is outside feasible polytope!!!\n";
	else
		cout << "u0 is belongs feasible polytope.\n";
	cout << "Including hyperplanes:\t"; SF_Print_HyperplanesIncludingPoint(PD_u, PP_EPS_POINT_IN_HALFSPACE); cout << endl;
	cout << "Including edges:\t"; PF_Print_Number_of_edges(PD_u);
#endif // PP_DEBUG
}

void PC_bsf_ProblemOutput(PT_bsf_reduceElem_T* reduceResult, int reduceCounter, PT_bsf_parameter_T parameter, double t) {
	cout << setprecision(PP_SETW / 2);

	PD_objF_u = SF_ObjF(PD_u);

	cout << "================================================" << endl;
	cout << "// Elapsed time: " << t << endl;
	cout << "// Number of iterations: " << PD_iterNo << endl;
	cout << "// Computed objective value: " << setprecision(16) << PD_objF_u << endl;
	cout << "// Maximal objective value:  " << PP_MAX_OBJ_VALUE << endl;
	cout << "// Relative error = " << setprecision(3) << SF_RelativeError(PP_MAX_OBJ_VALUE, PD_objF_u) << setprecision(PP_SETW / 2) << endl;
	cout << "================================================" << endl;

#ifdef PP_SAVE_RESULT
	if (SF_MTX_SavePoint(PD_u, PP_MTX_POSTFIX_SO))
		cout << "Calculated solution point is saved into file *.so" << endl;
#endif // PP_SAVE_RESULT

	cout << "Solution point:\t";
	SF_Print_Vector(PD_u);	cout << endl;
#ifdef PP_DEBUG
	cout << "Distance to polytope: " << SF_Distance_PointToPolytope(PD_u) << endl;
#endif // PP_DEBUG

} // end PC_bsf_ProblemOutput

void PC_bsf_ProblemOutput_1(PT_bsf_reduceElem_T_1* reduceResult, int reduceCounter, PT_bsf_parameter_T parameter, double t) {
	// Not used
}

void PC_bsf_ProblemOutput_2(PT_bsf_reduceElem_T_2* reduceResult, int reduceCounter, PT_bsf_parameter_T parameter, double t) {
	// Not used
}

void PC_bsf_ProblemOutput_3(PT_bsf_reduceElem_T_3* reduceResult, int reduceCounter, PT_bsf_parameter_T parameter, double t) {
	// Not used
}

void PC_bsf_ProcessResults(PT_bsf_reduceElem_T* reduceResult, int reduceCounter, PT_bsf_parameter_T* parameter, int* nextJob, bool* exit) {
	PT_vector_T u_moved;
	double shiftLength;

	if (SF_Vector_Norm(reduceResult->d) < PP_EPS_ZERO) {
		*exit = true;
		return;
	}

	SF_MovingOnPolytope(PD_u, reduceResult->d, u_moved, PP_EPS_MOVING_ON_POLYTOPE);
	shiftLength = SF_Distance_PointToPoint(PD_u, u_moved);

	SF_Vector_Copy(u_moved, PD_u);

	PD_iterNo++;

#ifdef PP_DEBUG
	PF_MakeHyperplaneList(PD_u, PD_index_includingHyperplanes, &PD_mh, PP_EPS_POINT_IN_HALFSPACE);
	PD_TWIDDLE_done = false;
	PD_TWIDDLE_nextEdgeI = 0;
	PF_TWIDDLE_Make_p(PD_TWIDDLE_p, PD_mh, PD_n - 1);
	PF_TWIDDLE_CodeToSubset(reduceResult->edgeCode, PD_index_includingHyperplanes, PD_index_activeHyperplanes, PD_mh, PD_ma);
	cout << "_________________________________________________ " << PD_iterNo << " _____________________________________________________" << endl;
	PD_objF_u = SF_ObjF(PD_u);
	cout << "Edge code: " << reduceResult->edgeCode << ".\tHyperplanes: {";
	for (int i = 0; i < PD_ma - 1; i++)
		cout << PD_index_activeHyperplanes[i] << ", ";
	cout << PD_index_activeHyperplanes[PD_ma - 1]
		<< "}.\tShift = " << shiftLength << "\tF(x) = " << PD_objF_u << endl;

	cout << "New vertex:\t";
	SF_Print_Vector(PD_u);
	cout << endl;
	cout << "Including hyperplanes:\t"; SF_Print_HyperplanesIncludingPoint(PD_u, PP_EPS_POINT_IN_HALFSPACE); cout << endl;
#endif

	SF_Vector_Copy(PD_u, parameter->x);
}

void PC_bsf_ProcessResults_1(PT_bsf_reduceElem_T_1* reduceResult, int reduceCounter, PT_bsf_parameter_T* parameter, int* nextJob, bool* exit) {
	// Not used
}

void PC_bsf_ProcessResults_2(PT_bsf_reduceElem_T_2* reduceResult, int reduceCounter, PT_bsf_parameter_T* parameter, int* nextJob, bool* exit) {
	// Not used
}

void PC_bsf_ProcessResults_3(PT_bsf_reduceElem_T_3* reduceResult, int reduceCounter, PT_bsf_parameter_T* parameter, int* nextJob, bool* exit) {
	// Not used
}

void PC_bsf_ReduceF(PT_bsf_reduceElem_T* x, PT_bsf_reduceElem_T* y, PT_bsf_reduceElem_T* z) { // z = x + y
	if (x->objF_p > y->objF_p) {
		z->objF_p = x->objF_p;
		for (int j = 0; j < PD_n; j++)
			(*z).d[j] = (*x).d[j];
		z->edgeCode = x->edgeCode;
	}
	else {
		z->objF_p = y->objF_p;
		for (int j = 0; j < PD_n; j++)
			(*z).d[j] = (*y).d[j];
		z->edgeCode = y->edgeCode;
	}
}

void PC_bsf_ReduceF_1(PT_bsf_reduceElem_T_1* x, PT_bsf_reduceElem_T_1* y, PT_bsf_reduceElem_T_1* z) {
	// Not used
}

void PC_bsf_ReduceF_2(PT_bsf_reduceElem_T_2* x, PT_bsf_reduceElem_T_2* y, PT_bsf_reduceElem_T_2* z) {
	// Not used
}

void PC_bsf_ReduceF_3(PT_bsf_reduceElem_T_3* x, PT_bsf_reduceElem_T_3* y, PT_bsf_reduceElem_T_3* z) {
	// Not used
}

void PC_bsf_SetInitParameter(PT_bsf_parameter_T* parameter) {
	SF_Vector_Copy(PD_u, parameter->x);
}

void PC_bsf_SetListSize(int* listSize) {
	*listSize = PP_KK;
}

void PC_bsf_SetMapListElem(PT_bsf_mapElem_T* elem, int i) {
	elem->edgeCode = &(PD_edgeCodeList[i]);
}

//----------------------- Assigning Values to BSF-skeleton Variables (Do not modify!) -----------------------
void PC_bsfAssignAddressOffset(int value) { BSF_sv_addressOffset = value; }
void PC_bsfAssignIterCounter(int value) { BSF_sv_iterCounter = value; }
void PC_bsfAssignJobCase(int value) { BSF_sv_jobCase = value; }
void PC_bsfAssignMpiMaster(int value) { BSF_sv_mpiMaster = value; }
void PC_bsfAssignMpiRank(int value) { BSF_sv_mpiRank = value; }
void PC_bsfAssignNumberInSublist(int value) { BSF_sv_numberInSublist = value; }
void PC_bsfAssignNumOfWorkers(int value) { BSF_sv_numOfWorkers = value; }
void PC_bsfAssignParameter(PT_bsf_parameter_T parameter) { PC_bsf_CopyParameter(parameter, &BSF_sv_parameter); }
void PC_bsfAssignSublistLength(int value) { BSF_sv_sublistLength = value; }

//---------------------------------- Shared Functions -------------------------

inline double SF_Distance_PointToHalfspace_i(PT_vector_T x, int i) {
	double a_dot_z_minus_b = SF_Vector_DotProduct(PD_A[i], x) - PD_b[i];

	if (PD_norm_a[i] < PP_EPS_ZERO) //Degenerate equation
		return 0;

	if (a_dot_z_minus_b < 0) // Point belongs to halfspace
		return 0;

	return a_dot_z_minus_b / PD_norm_a[i];
}

inline double SF_Distance_PointToHyperplane_i(PT_vector_T x, int i) {
	if (PD_norm_a[i] < PP_EPS_ZERO) //Degenerate equation
		return 0;
	else
		return fabs(SF_Vector_DotProduct(PD_A[i], x) - PD_b[i]) / PD_norm_a[i];
}

inline double SF_Distance_PointToPoint(PT_vector_T x, PT_vector_T y) {
	PT_vector_T z;
	SF_Vector_Subtraction(x, y, z);
	return SF_Vector_Norm(z);
}

inline double SF_Distance_PointToPolytope(PT_vector_T x) { // Measure of distance from point to polytope
	double maxDistance = 0;
	double distance;

	for (int i = 0; i < PD_m; i++) {
		distance = SF_Distance_PointToHalfspace_i(x, i);
		if (distance > 0)
			maxDistance = PF_MAX(maxDistance, distance);
	}
	return maxDistance;
}

inline void SF_MakeColumnOfNorms(PT_matrix_T A, PT_column_T norm_a) {
	for (int i = 0; i < PD_m; i++)
		norm_a[i] = SF_Vector_Norm(A[i]);
}

inline void SF_MovingOnPolytope(PT_vector_T startPoint, PT_vector_T directionVector, PT_vector_T finishPoint, double epsMoving) {
	double leftBound = 0;
	double rightBound = PP_DBL_MAX;
	double factor = 1;
	double delta;

	assert(SF_Vector_Norm(directionVector) >= PP_EPS_ZERO);

	delta = factor / 2;

	while (rightBound - leftBound >= PP_EPS_ZERO && delta > 0) {
		SF_Shift(startPoint, directionVector, factor, finishPoint);
		if (SF_PointBelongsPolytope(finishPoint, PP_EPS_POINT_IN_HALFSPACE)) {
			leftBound = factor;
			delta *= 2;
			factor += delta;
		}
		else {
			rightBound = factor;
			delta /= 2;
			factor -= delta;
		}
	}

	SF_Shift(startPoint, directionVector, factor, finishPoint);
	delta = epsMoving;
	while (!SF_PointBelongsPolytope(finishPoint, epsMoving) && delta > 0) {
		factor -= delta;
		delta *= 2;
		SF_Shift(startPoint, directionVector, factor, finishPoint);
	}
}

inline void SF_MovingToPolytope(PT_vector_T startPoint, PT_vector_T directionVector, PT_vector_T finishPoint, double epsMoving) {
	double leftBound = 0;
	double rightBound = PP_DBL_MAX;
	double factor = 1;
	double delta;
	static int outerHalspace_i[PP_MM];	// Index of out half-spaces
	int mo;								// Number of out half-spaces
	bool pointInsideCone;

	assert(SF_Vector_Norm(directionVector) >= PP_EPS_ZERO);

	mo = 0;
	for (int i = 0; i < PD_m; i++)
		if (!SF_PointBelongsHalfspace_i(startPoint, i, PP_EPS_POINT_IN_HALFSPACE)) {
			outerHalspace_i[mo] = i;
			mo++;
		}

	delta = factor / 2;

	while (rightBound - leftBound >= PP_EPS_ZERO && delta > 0) {
		SF_Shift(startPoint, directionVector, factor, finishPoint);

		pointInsideCone = true;
		for (int i = 0; i < mo; i++)
			if (SF_PointBelongsHalfspace_i(finishPoint, outerHalspace_i[i], PP_EPS_POINT_IN_HALFSPACE)) {
				pointInsideCone = false;
				break;
			}
		if (pointInsideCone) {
			leftBound = factor;
			delta *= 2;
			factor += delta;
		}
		else {
			rightBound = factor;
			delta /= 2;
			factor -= delta;
			assert(factor > 0);
		}
	}

	SF_Shift(startPoint, directionVector, factor, finishPoint);
	delta = epsMoving;
	do {
		pointInsideCone = false;
		for (int i = 0; i < mo; i++)
			if (!SF_PointBelongsHalfspace_i(finishPoint, outerHalspace_i[i], epsMoving)) {
				pointInsideCone = true;
				factor -= delta;
				delta *= 2;
				assert(factor > 0);
				SF_Shift(startPoint, directionVector, factor, finishPoint);
				break;
			}
	} while (pointInsideCone && delta > 0 && delta > 0);
}

static void SF_MTX_Conversion() { // Transformation to inequalities & dimensionality reduction
	int m_equation = PD_m;
	int m_inequality;
	int m_lowerBound;
	int m_higherBound;

	for (int i = 0; i < m_equation; i++) { // Conversion to inequalities
		for (int j = 0; j < PD_n; j++)
			PD_A[PD_m][j] = -PD_A[i][j];
		PD_b[PD_m] = -PD_b[i];
		PD_m++; assert(PD_m <= PP_MM);
	}

	for (int i = 0; i < PD_m; i++) // Remove negative sign for zero value
		for (int j = 0; j < PD_n; j++)
			if (PD_A[i][j] == 0)
				PD_A[i][j] = 0;
	m_inequality = PD_m;

	for (int i = 0; i < PD_n; i++) { // Adding lower bound conditions
		for (int j = 0; j < PD_n; j++)
			PD_A[i + PD_m][j] = 0;
		PD_A[i + PD_m][i] = -1;
		if (PD_lo[i] == 0)
			PD_b[i + PD_m] = 0;
		else
			PD_b[i + PD_m] = -PD_lo[i];
	}
	PD_m += PD_n; assert(PD_m <= PP_MM);
	m_lowerBound = PD_m;

	for (int i = 0; i < PD_n; i++) { // Adding higher bound conditions
		if (PD_hi[i] != PP_INFINITY) {
			for (int j = 0; j < PD_n; j++)
				PD_A[PD_m][j] = 0;
			PD_A[PD_m][i] = 1;
			PD_b[PD_m] = PD_hi[i];
			PD_m++; assert(PD_m <= PP_MM);
		}
	}
	m_higherBound = PD_m;

	/**
	cout << "-----------------------------------------------------\n";
	SF_Print_Inequalities();
	cout << "-----------------------------------------------------\n";
	cout << "PD_c: "; SF_Print_Vector(PD_c); cout << endl;/**/

	SF_MTX_RemoveFreeVariables(m_equation, m_inequality, m_lowerBound, m_higherBound);

	/**
	cout << "-----------------------------------------------------\n";
	SF_Print_Inequalities();
	cout << "-----------------------------------------------------\n";/**/
}

inline void SF_MTX_ConversionSimple() { // Transformation to inequalities
	int m_equation = PD_m;

	for (int i = 0; i < m_equation; i++) { // Conversion to inequalities
		for (int j = 0; j < PD_n; j++)
			PD_A[PD_m][j] = -PD_A[i][j];
		PD_b[PD_m] = -PD_b[i];
		PD_m++; assert(PD_m <= PP_MM);
	}

	for (int i = 0; i < PD_m; i++) // Remove negative sign for zero value
		for (int j = 0; j < PD_n; j++)
			if (PD_A[i][j] == 0)
				PD_A[i][j] = 0;

	for (int i = 0; i < PD_n; i++) { // Adding lower bound conditions
		for (int j = 0; j < PD_n; j++)
			PD_A[i + PD_m][j] = 0;
		PD_A[i + PD_m][i] = -1;
		if (PD_lo[i] == 0)
			PD_b[i + PD_m] = 0;
		else
			PD_b[i + PD_m] = -PD_lo[i];
	}
	PD_m += PD_n; assert(PD_m <= PP_MM);

	for (int i = 0; i < PD_n; i++) { // Adding higher bound conditions
		if (PD_hi[i] != PP_INFINITY) {
			for (int j = 0; j < PD_n; j++)
				PD_A[PD_m][j] = 0;
			PD_A[PD_m][i] = 1;
			PD_b[PD_m] = PD_hi[i];
			PD_m++; assert(PD_m <= PP_MM);
		}
	}
}

static bool SF_MTX_Load__Problem() {

	//--------------- Reading A ------------------
	if (!SF_MTX_Load_A())
		return false;

	//--------------- Reading b ------------------
	if (!SF_MTX_Load_b())
		return false;

	//--------------- Reading lo ------------------
	if (!SF_MTX_Load_lo())
		return false;

	//--------------- Reading c ------------------
	if (!SF_MTX_Load_c())
		return false;

	//--------------- Reading hi ------------------
	if (!SF_MTX_Load_hi())
		return false;

	//---------- Conversion to inequalities -----------
#ifdef PP_SIMPLE_CONVERSION
	SF_MTX_ConversionSimple();
#else
	SF_MTX_Conversion();
#endif

	/**
	cout << "-----------------------------------------------------\n";
	SF_Print_Inequalities();
	cout << "-----------------------------------------------------\n";
	cout << "PD_c: "; SF_Print_Vector(PD_c); cout << endl;/**/

	return true;
}

inline bool SF_MTX_Load_A() {
	int nor;	// Number of matrix rows
	int noc;	// Number of matrix columns
	int non;	// Number of non-zero elements
	const char* mtxFile;
	FILE* stream;// Input stream
	char str[80] = { '\0' };
	char* chr = str;
	string PD_MTX_File;

	PD_MTX_File = PP_PATH;
	PD_MTX_File += PP_MTX_PREFIX;
	PD_MTX_File += PD_problemName;
	PD_MTX_File += PP_MTX_POSTFIX_A;
	mtxFile = PD_MTX_File.c_str();
	stream = fopen(mtxFile, "r+b");

	if (stream == NULL) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Failure of opening file '" << mtxFile << "'.\n";
		return false;
	}

	SF_MTX_SkipComments(stream);
	if (fscanf(stream, "%d%d%d", &nor, &noc, &non) < 3) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Unexpected end of file " << mtxFile << endl;
		return false;
	}

	if (nor >= noc) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Number of rows m = " << nor << " must be < " << "Number of columns n = " << noc << "\n";
		return false;
	}

	if (noc != PP_N) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Invalid input data: PP_N must be = " << noc << "\n";
		return false;
	}

	if (nor != PP_M) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Invalid input data: PP_M must be = " << nor << "\n";
		return false;
	}

	PD_m = nor;
	PD_n = noc;

	if (2 * nor + noc > PP_MM) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Invalid input data: number of inequalities m = " << 2 * nor + noc
			<< " must be < PP_MM + 1 =" << PP_MM + 1 << "\n";
		return false;
	}

	for (int k = 0; k < non; k++) {
		int i, j;

		if (fscanf(stream, "%d%d%s", &i, &j, str) < 3) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout
				<< "Unexpected end of file'" << mtxFile << "'." << endl;
			return false;
		}

		i -= 1;
		j -= 1;
		if (i < 0) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout
				<< "Negative row index in'" << mtxFile << "'.\n" << endl;
			return false;
		}
		if (j < 0) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout
				<< "Negative column index in'" << mtxFile << "'.\n" << endl;
			return false;
		}
		PD_A[i][j] = strtod(str, &chr);
	}

	fclose(stream);

	return true;
}

inline bool SF_MTX_Load_b() {
	int nor;	// Number of matrix rows
	int noc;	// Number of matrix columns
	const char* mtxFile;
	FILE* stream;// Input stream
	char str[80] = { '\0' };
	char* chr = str;
	string PD_MTX_File;

	PD_MTX_File = PP_PATH;
	PD_MTX_File += PP_MTX_PREFIX;
	PD_MTX_File += PD_problemName;
	PD_MTX_File += PP_MTX_POSTFIX_B;
	mtxFile = PD_MTX_File.c_str();
	stream = fopen(mtxFile, "r+b");

	if (stream == NULL) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Failure of opening file '" << mtxFile << "'.\n";
		return false;
	}

	SF_MTX_SkipComments(stream);
	if (fscanf(stream, "%d%d", &nor, &noc) < 2) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Unexpected end of file'" << mtxFile << "'." << endl;
		return false;
	}
	if (PD_m != nor) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of rows in'" << mtxFile << "'.\n";
		return false;
	}
	if (noc != 1) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of columnws in'" << mtxFile << "'.\n";
		return false;
	}

	for (int i = 0; i < PD_m; i++) {
		if (fscanf(stream, "%s", str) < 1) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout
				<< "Unexpected end of file '" << mtxFile << "'." << endl;
			return false;
		}
		PD_b[i] = strtod(str, &chr);
	}
	fclose(stream);

	return true;
}

inline bool SF_MTX_Load_c() {
	int nor;	// Number of matrix rows
	int noc;	// Number of matrix columns
	const char* mtxFile;
	FILE* stream;// Input stream
	char str[80] = { '\0' };
	char* chr = str;
	string PD_MTX_File;

	PD_MTX_File = PP_PATH;
	PD_MTX_File += PP_MTX_PREFIX;
	PD_MTX_File += PD_problemName;
	PD_MTX_File += PP_MTX_POSTFIX_C;
	mtxFile = PD_MTX_File.c_str();
	stream = fopen(mtxFile, "r+b");

	if (stream == NULL) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Failure of opening file '" << mtxFile << "'.\n";
		return false;
	}

	SF_MTX_SkipComments(stream);
	if (fscanf(stream, "%d%d", &nor, &noc) < 2) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Unexpected end of file'" << mtxFile << "'." << endl;
		return false;
	}
	if (nor != PD_n) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of rows in'" << mtxFile << "'.\n";
		return false;
	}
	if (noc != 1) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of columnws in'" << mtxFile << "'.\n";
		return false;
	}

	for (int j = 0; j < PD_n; j++) {
		if (fscanf(stream, "%s", str) < 0) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout
				<< "Unexpected end of file" << endl;
			return false;
		}
		PD_c[j] = -strtod(str, &chr);
	}
	fclose(stream);

	return true;
}

inline bool SF_MTX_Load_hi() {
	int nor;	// Number of matrix rows
	int noc;	// Number of matrix columns
	const char* mtxFile;
	FILE* stream;// Input stream
	char str[80] = { '\0' };
	char* chr = str;
	string PD_MTX_File;

	PD_MTX_File = PP_PATH;
	PD_MTX_File += PP_MTX_PREFIX;
	PD_MTX_File += PD_problemName;
	PD_MTX_File += PP_MTX_POSTFIX_HI;
	mtxFile = PD_MTX_File.c_str();
	stream = fopen(mtxFile, "r+b");

	if (stream == NULL) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Failure of opening file '" << mtxFile << "'.\n";
		return false;
	}

	SF_MTX_SkipComments(stream);
	if (fscanf(stream, "%d%d", &nor, &noc) < 2) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Unexpected end of file'" << mtxFile << "'." << endl;
		return false;
	}
	if (nor != PD_n) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of rows in'" << mtxFile << "'.\n";
		return false;
	}
	if (noc != 1) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of columnws in'" << mtxFile << "'.\n";
		return false;
	}

	for (int j = 0; j < PD_n; j++) {
		if (fscanf(stream, "%s", str) < 1) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout << "Unexpected end of file '" << mtxFile << "'." << endl;
			return false;
		}
		PD_hi[j] = strtod(str, &chr);
	}
	fclose(stream);
	return true;
}

inline bool SF_MTX_Load_lo() {
	int nor;	// Number of matrix rows
	int noc;	// Number of matrix columns
	const char* mtxFile;
	FILE* stream;// Input stream
	char str[80] = { '\0' };
	char* chr = str;
	string PD_MTX_File;

	PD_MTX_File = PP_PATH;
	PD_MTX_File += PP_MTX_PREFIX;
	PD_MTX_File += PD_problemName;
	PD_MTX_File += PP_MTX_POSTFIX_LO;
	mtxFile = PD_MTX_File.c_str();
	stream = fopen(mtxFile, "r+b");

	if (stream == NULL) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Failure of opening file '" << mtxFile << "'.\n";
		return false;
	}

	SF_MTX_SkipComments(stream);
	if (fscanf(stream, "%d%d", &nor, &noc) < 2) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Unexpected end of file'" << mtxFile << "'." << endl;
		return false;
	}
	if (nor != PD_n) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of rows in'" << mtxFile << "'.\n";
		return false;
	}
	if (noc != 1) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of columnws in'" << mtxFile << "'.\n";
		return false;
	}

	for (int j = 0; j < PD_n; j++) {
		if (fscanf(stream, "%s", str) < 1) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout
				<< "Unexpected end of file '" << mtxFile << "'." << endl;
			return false;
		}
		PD_lo[j] = strtod(str, &chr);
	}

	fclose(stream);

	return true;
}

inline bool SF_MTX_LoadPoint(PT_vector_T x, string postfix) {
	int nor;	// Number of matrix rows
	int noc;	// Number of matrix columns
	const char* mtxFile;
	FILE* stream;// Input stream
	char str[80] = { '\0' };
	char* chr = str;
	string PD_MTX_File;

	PD_MTX_File = PP_PATH;
	PD_MTX_File += PP_MTX_PREFIX;
	PD_MTX_File += PD_problemName;
	PD_MTX_File += postfix;
	mtxFile = PD_MTX_File.c_str();
	stream = fopen(mtxFile, "r+b");

	if (stream == NULL) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Failure of opening file '" << mtxFile << "'.\n";
		return false;
	}

	SF_MTX_SkipComments(stream);
	if (fscanf(stream, "%d%d", &nor, &noc) < 2) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Unexpected end of file'" << mtxFile << "'." << endl;
		return false;
	}
	if (nor != PD_n) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of rows in'" << mtxFile << "'.\n";
		return false;
	}
	if (noc != 1) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Incorrect number of columnws in'" << mtxFile << "'.\n";
		return false;
	}

	for (int j = 0; j < PD_n; j++) {
		if (fscanf(stream, "%s", str) < 0) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout
				<< "Unexpected end of file" << endl;
			return false;
		}
		x[j] = strtod(str, &chr);
	}
	fclose(stream);

	return true;
}

inline void SF_MTX_RemoveFreeVariables(int m_equation, int m_inequality, int m_lowerBound, int m_higherBound) {
	int freeVariable_j_col[PP_M];
	int m_freeVariable_j_col = 0;
	int rowToDelete_i[PP_MM];
	int m_rowToDelete = 0;
	bool unique;
	bool ok;

	// Find free variables
	for (int j_col = 0; j_col < PD_n; j_col++) { // Find zero element in PD_c
		if (PD_c[j_col] != 0)
			continue;
		for (int i_row = 0; i_row < m_equation; i_row++) { // Find PD_A i_row with nonzero element in column j_col
			if (PD_A[i_row][j_col] == 0)
				continue;

			// Check uniqueness in column j_col
			unique = true;
			for (int i = 0; i < m_equation; i++) {
				if (i == i_row)
					continue;
				if (PD_A[i][j_col] != 0) {
					unique = false;
					break;
				}
			}
			if (!unique)
				continue;

			// Check lower bound
			ok = true;
			for (int i_lowerBound = m_inequality; i_lowerBound < m_lowerBound; i_lowerBound++) {
				if (PD_A[i_lowerBound][j_col] == 0)
					continue;
				if (PD_A[i_lowerBound][j_col] != -1) {
					ok = false;
					break;
				}
				if (PD_b[i_lowerBound] != 0) {
					ok = false;
					break;
				}
				rowToDelete_i[m_rowToDelete] = i_lowerBound;
				m_rowToDelete++; assert(m_rowToDelete <= PP_MM);
				break;
			}
			if (!ok)
				continue;

			// Check higher bound
			ok = true;
			for (int i_higherBound = m_lowerBound; i_higherBound < m_higherBound; i_higherBound++) {
				if (PD_A[i_higherBound][j_col] != 0) {
					ok = false;
					m_rowToDelete--;
					break;
				}
			}
			if (!ok)
				continue;

			freeVariable_j_col[m_freeVariable_j_col] = j_col;
			m_freeVariable_j_col++; assert(m_freeVariable_j_col <= PP_M);
			rowToDelete_i[m_rowToDelete] = m_equation + i_row;
			m_rowToDelete++; assert(m_rowToDelete <= PP_MM);
			// Check sign of free variable
			if (PD_A[i_row][j_col] < 0) {
				// Change sign of inequality
				for (int j = 0; j < PD_n; j++)
					PD_A[i_row][j] = -PD_A[i_row][j];
				PD_b[i_row] = -PD_b[i_row];
			}
			break;
		}
	}

	{// Eliminate columns with free variables
		static bool colToDeleteLable[PP_N];
		for (int j = 0; j < m_freeVariable_j_col; j++) {
			assert(freeVariable_j_col[j] < PP_N);
			colToDeleteLable[freeVariable_j_col[j]] = true;
		}

		for (int j = 0; j < PD_n; j++) {
			if (colToDeleteLable[j]) {
				for (int i = 0; i < PD_m; i++)
					PD_A[i][j] = PD_A[i][PD_n - 1];
				PD_c[j] = PD_c[PD_n - 1];
				colToDeleteLable[j] = colToDeleteLable[PD_n - 1];
				j--; assert(j >= 0);
				PD_n--; assert(PD_n >= 0);

				/**
				cout << "-----------------------------------------------------\n";
				SF_Print_Inequalities();
				cout << "-----------------------------------------------------\n";/**/

			}
		}
	}

	{// Eliminate rows corresponding to free variables
		static bool rowToDeleteLable[PP_MM];
		for (int i = 0; i < m_rowToDelete; i++) {
			rowToDeleteLable[rowToDelete_i[i]] = true;
		}
		for (int i = 0; i < PD_m; i++) {
			if (rowToDeleteLable[i]) {
				for (int j = 0; j < PD_n; j++)
					PD_A[i][j] = PD_A[PD_m - 1][j];
				PD_b[i] = PD_b[PD_m - 1];
				rowToDeleteLable[i] = rowToDeleteLable[PD_m - 1];
				i--; assert(i >= 0);
				PD_m--; assert(PD_m >= 0);

				/**
				cout << "-----------------------------------------------------\n";
				SF_Print_Inequalities();
				cout << "-----------------------------------------------------\n";/**/
			}
		}
	}
}

static bool SF_MTX_SavePoint(PT_vector_T x, string postfix) {
	const char* mtxFile;
	FILE* stream;// Input stream
	string PD_MTX_File;

	PD_MTX_File = PP_PATH;
	PD_MTX_File += PP_MTX_PREFIX;
	PD_MTX_File += PD_problemName;
	PD_MTX_File += postfix;
	mtxFile = PD_MTX_File.c_str();
	stream = fopen(mtxFile, "w");
	if (stream == NULL) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Failure of opening file '" << mtxFile << "'.\n";
		return false;
	}

	fprintf(stream, "%d %d\n", PD_n, 1);

	for (int j = 0; j < PD_n; j++)
		fprintf(stream, "%.16f\n", x[j]);

	fclose(stream);
	return true;
}

inline void SF_MTX_SkipComments(FILE* stream) {
	fpos_t pos;	// Position in the input stream
	int res;
	res = fscanf(stream, "\n");
	fgetpos(stream, &pos);
	while (getc(stream) == '%') {
		while (getc(stream) != 10);
		res = fscanf(stream, "\n");
		fgetpos(stream, &pos);
	}
	fsetpos(stream, &pos);
}

inline double SF_ObjF(PT_vector_T x) {
	double s = 0;
	for (int j = 0; j < PD_n; j++)
		s += PD_c[j] * x[j];
	return s;
}

inline void	SF_ObliqueProjectingVectorOntoHalfspace_i(PT_vector_T z, int i, PT_vector_T g, PT_vector_T o, double eps, int* exitCode) {
	// Oblique projecting vector o of point z onto Half-space H_i with respect to vector g
	double a_dot_g;	// <a,g>
	double a_dot_z_minus_b;	// <a,z> - b
	double factor;	// (b - <a,z>) / <a,g>

	if (PD_norm_a[i] < PP_EPS_ZERO) {
		SF_Vector_Zeroing(o);
		*exitCode = 0; // #define PP_EXITCODE_DEGENERATE_INEQUALITY		0
		return;
	}

	a_dot_z_minus_b = SF_Vector_DotProduct(PD_A[i], z) - PD_b[i]; // <a,z> - b

	if (fabs(a_dot_z_minus_b) < PP_EPS_ZERO) { // <a,z> - b < 0
		*exitCode = 1; // #define PP_EXITCODE_ON_HYPERPLANE				1
		SF_Vector_Zeroing(o);
		return;
	}

	if (a_dot_z_minus_b <= -PP_EPS_ZERO) { // <a,z> - b < 0
		*exitCode = 2; // #define PP_EXITCODE_INSIDE_HALFSPACE			2
		SF_Vector_Zeroing(o);
		return;
	}

	a_dot_g = SF_Vector_DotProduct(PD_A[i], g); // <a,g>


	if (fabs(a_dot_g) < PP_EPS_ZERO) {
		*exitCode = 4; // #define PP_EXITCODE_PARALLEL				4
		SF_Vector_Zeroing(o);
		return;
	}

	if (a_dot_g >= PP_EPS_ZERO) {
		*exitCode = 5; // #define PP_EXITCODE_RECESSIVE				5
		SF_Vector_Zeroing(o);
		return;
	}

	factor = a_dot_z_minus_b / a_dot_g; // (<a,z> - b) / <a,g>

	// Oblique projection vector: o = -(<a,z> - b)g/<a, g> = -factor * g
	SF_Vector_MultiplyByNumber(g, -factor, o);

	*exitCode = 9; // #define PP_EXITCODE_NONDEGENERATE_PROJECTING	9
	return;
}

inline void SF_OrthogonalProjectingVectorOntoHalfspace_i(PT_vector_T z, int i, PT_vector_T r, double eps, int* exitCode) {
	double factor;
	double a_dot_z_minus_b = SF_Vector_DotProduct(PD_A[i], z) - PD_b[i]; // <a,z> - b
	double distance = fabs(a_dot_z_minus_b) / PD_norm_a[i];

	if (PD_norm_a[i] < PP_EPS_ZERO) {
		SF_Vector_Zeroing(r);
		*exitCode = 0; // #define PP_EXITCODE_DEGENERATE_INEQUALITY		0
		return;
	}

	if (distance < eps) {
		SF_Vector_Zeroing(r);
		*exitCode = 1; // #define PP_EXITCODE_ON_HYPERPLANE				1
		return;
	}

	if (a_dot_z_minus_b < 0) { // <a,z> - b < 0
		SF_Vector_Zeroing(r);
		*exitCode = 2; // #define PP_EXITCODE_INSIDE_HALFSPACE			2
		return;
	}

	factor = -a_dot_z_minus_b / (PD_norm_a[i] * PD_norm_a[i]); // (b - <z,a>) / ||a||^2
	SF_Vector_MultiplyByNumber(PD_A[i], factor, r); // r = a(b - <z,a>) / ||a||^2
	*exitCode = 9; // #define PP_EXITCODE_NONDEGENERATE_PROJECTING	9
}

inline void SF_OrthogonalProjectingVectorOntoHyperplane_i(PT_vector_T x, int i, PT_vector_T p) {
	assert(SF_Vector_NormSquare(PD_A[i]));
	SF_Vector_MultiplyByNumber(PD_A[i], -(SF_Vector_DotProduct(PD_A[i], x) - PD_b[i]) / SF_Vector_NormSquare(PD_A[i]), p);
}

inline bool SF_PointBelongsHalfspace_i(PT_vector_T x, int i, double eps) {
	if (PD_norm_a[i] < eps) //Degenerate equation
		return true;
	double a_dot_x_minus_b = SF_Vector_DotProduct(PD_A[i], x) - PD_b[i];
	double distanceToHyperplane = fabs(a_dot_x_minus_b) / PD_norm_a[i];
	if (distanceToHyperplane < eps)
		return true;
	if (a_dot_x_minus_b < 0)
		return true;
	return false;
}

inline bool SF_PointBelongsHyperplane_i(PT_vector_T x, int i, double eps) {
	if (SF_Distance_PointToHyperplane_i(x, i) < eps)
		return true;
	else
		return false;
}

inline bool SF_PointBelongsPolytope(PT_vector_T x, double eps) { // If the point belongs to the polytope with prescigion of eps
	for (int i = 0; i < PD_m; i++)
		if (!SF_PointBelongsHalfspace_i(x, i, eps))
			return false;
	return true;
}

inline void SF_PointHomothety(PT_vector_T x, PT_vector_T center, double ratio) { // https://en.wikipedia.org/wiki/Homothety
	if (ratio == 1)
		return;
	assert(ratio > 0);
	for (int j = 0; j < PD_n; j++)
		x[j] = ratio * x[j] - (ratio - 1) * center[j];
}

inline bool SF_PointInsideHalfspace_i(PT_vector_T x, int i, double eps) {
	if (PD_norm_a[i] < PP_EPS_ZERO) //Degenerate equation
		return true;
	double a_dot_x_minus_b = SF_Vector_DotProduct(PD_A[i], x) - PD_b[i];
	double distanceToHyperplane = fabs(a_dot_x_minus_b) / PD_norm_a[i];
	if (distanceToHyperplane < eps)
		return false;
	if (a_dot_x_minus_b < 0)
		return true;
	return false;
}

inline void SF_PolytopeHomothety(PT_vector_T center, double ratio) { // https://en.wikipedia.org/wiki/Homothety
	if (ratio == 1)
		return;
	assert(ratio > 0);

	for (int i = 0; i < PD_m; i++) {
		PD_b[i] = ratio * PD_b[i] - (ratio - 1) * SF_Vector_DotProduct(PD_A[i], center);
	}
}

inline void SF_Print_Inequalities() {
	for (int i = 0; i < PD_m; i++) {
		cout << i << ")";
		for (int j = 0; j < PD_n; j++)
			cout << setw(PP_SETW) << PD_A[i][j];
		cout << "\t<=" << setw(PP_SETW) << PD_b[i] << endl;
	}
}

inline void SF_Print_HyperplanesIncludingPoint(PT_vector_T x, double eps) {
	bool comma = false;

	cout << "{";

	for (int i = 0; i < PD_m; i++) {
		if (SF_PointBelongsHyperplane_i(x, i, eps)) {
			if (comma)
				cout << ", ";
			else
				comma = true;
			cout << i;
		}
	}

	cout << "}";
}

inline void SF_Print_Vector(PT_vector_T x) {
	for (int j = 0; j < PF_MIN(PP_OUTPUT_LIMIT, PD_n); j++) cout << setw(PP_SETW) << x[j];
	if (PP_OUTPUT_LIMIT < PD_n) cout << "	...";
}

inline double SF_RelativeError(double trueValue, double calculatedValue) {
	if (fabs(trueValue) >= PP_EPS_ZERO)
		return fabs(calculatedValue - trueValue) / fabs(trueValue);
	else
		return fabs(calculatedValue - trueValue);
}

inline void SF_Shift(PT_vector_T point, PT_vector_T shiftVector, double factor, PT_vector_T shiftedPoint) {
	for (int j = 0; j < PD_n; j++)
		shiftedPoint[j] = point[j] + shiftVector[j] * factor;
}

inline void SF_Vector_Addition(PT_vector_T x, PT_vector_T y, PT_vector_T z) {  // z = x + y
	for (int j = 0; j < PD_n; j++)
		z[j] = x[j] + y[j];
}

inline void SF_Vector_Copy(PT_vector_T fromPoint, PT_vector_T toPoint) { // toPoint = fromPoint
	for (int j = 0; j < PD_n; j++)
		toPoint[j] = fromPoint[j];
}

inline void SF_Vector_DivideByNumber(PT_vector_T x, double r, PT_vector_T y) {  // y = x/r
	for (int j = 0; j < PD_n; j++)
		y[j] = x[j] / r;
}

inline void SF_Vector_DivideEquals(PT_vector_T x, double r) {  // x = x/r
	for (int j = 0; j < PD_n; j++)
		x[j] /= r;
}

inline double SF_Vector_DotProduct(PT_vector_T x, PT_vector_T y) {
	double sum = 0;
	for (int j = 0; j < PD_n; j++)
		sum += x[j] * y[j];
	return sum;
}

inline bool SF_Vector_Is_Tiny(PT_vector_T x, double eps) {
	return SF_Vector_Norm(x) < eps;
}

inline void SF_Vector_MakeLike(PT_vector_T x, double lengthOfLikeVector, PT_vector_T likeVector) {
	double norm_x = SF_Vector_Norm(x);
	if (norm_x == 0)
		SF_Vector_Zeroing(likeVector);
	else
		SF_Vector_MultiplyByNumber(x, lengthOfLikeVector / norm_x, likeVector);
}

void SF_Vector_MakeMinus_e(PT_vector_T minus_e) {
	for (int j = 0; j < PD_n; j++)
		minus_e[j] = -1;
}

inline void SF_Vector_MinusEquals(PT_vector_T equalPoint, PT_vector_T minusVector) { // equalPoint += minusVector
	for (int j = 0; j < PD_n; j++)
		equalPoint[j] -= minusVector[j];
}

inline void SF_Vector_MultiplyByNumber(PT_vector_T x, double r, PT_vector_T y) {  // y = r*x
	for (int j = 0; j < PD_n; j++)
		y[j] = x[j] * r;
}

inline void SF_Vector_MultiplyEquals(PT_vector_T x, double r) {  // x = r*x
	for (int j = 0; j < PD_n; j++)
		x[j] *= r;
}

inline double SF_Vector_Norm(PT_vector_T x) {
	double norm_x = sqrt(SF_Vector_NormSquare(x));
	return norm_x;
}

inline double SF_Vector_NormSquare(PT_vector_T x) {
	double sum = 0;

	for (int j = 0; j < PD_n; j++) {
		sum += x[j] * x[j];
	}
	return sum;
}

inline void SF_Vector_PlusEquals(PT_vector_T equalVector, PT_vector_T plusVector) { // equalVector += plusVector
	for (int j = 0; j < PD_n; j++)
		equalVector[j] += plusVector[j];
}

inline void SF_Vector_Round(PT_vector_T x, double eps) {
	double floorValue;
	double fractionalPart;
	double sign;
	double absValue;

	for (int j = 0; j < PD_n; j++) {
		if (fabs(x[j]) < eps) {
			x[j] = 0;
			continue;
		}
		absValue = fabs(x[j]);
		sign = x[j] > 0 ? 1 : -1;
		floorValue = floor(absValue);
		fractionalPart = absValue - floorValue;
		if (1 - fractionalPart < eps) {
			x[j] = sign * (floorValue + 1);
			continue;
		}
		if (fractionalPart < eps)
			x[j] = sign * floorValue;
	}
}

inline void SF_Vector_Subtraction(PT_vector_T x, PT_vector_T y, PT_vector_T z) {  // z = x - y
	for (int j = 0; j < PD_n; j++)
		z[j] = x[j] - y[j];
}

inline void SF_Vector_Zeroing(PT_vector_T x) {  // x = 0
	for (int j = 0; j < PD_n; j++) x[j] = 0;
}

//---------------------------------- Private Functions -------------------------

inline unsigned long long PF_BinomialCoefficient(int n, int k) {
	unsigned long long res = 1;
	if (k > n / 2) k = n - k;
	if (k == 1)  return n;
	if (k == 0)  return 1;
	for (int i = 1; i <= k; ++i) {
		res *= n - k + i;
		res /= i;
	}
	return res;
}

inline void PF_CalculateNumberOfEdges(int n, int mh, int* me) {
	if (mh == n) {
		*me = mh;
		if (*me > PP_KK) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout << "Parameter PP_KK = " << PP_KK << " must be greater than or equal to " << *me << "\n";
			abort();
		}
	}
	else {
		if (mh > 62) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout << "It is impossible to calculate binomial coefficient for number of including hyperplanes mh = "
				<< mh << " > 62" << endl;
			abort();
		}

		unsigned long long long_me = PF_BinomialCoefficient(mh, n - 1);

		if (long_me > PP_KK) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout << "Parameter PP_KK = " << PP_KK << " must be greater than or equal to " << long_me << "\n";
			abort();
		}
		*me = (int)long_me;
	}
}

inline void PF_MakeEdgeList(int* edgeCodeList, int me) {
	int index;

	if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
		cout << "List of edges in random order is generated...\n";

	for (int k = 0; k < PP_KK; k++) {
		edgeCodeList[k] = -1;
	}

	if (PP_KK <= RAND_MAX) {
		for (int k = 1; k <= me; k++) {
			index = rand() % PP_KK;
			if (edgeCodeList[index] == -1)
				edgeCodeList[index] = 0;
			else {
				for (int ki = 1; ki < PP_KK; ki++) {
					if (edgeCodeList[(index + ki) % PP_KK] == -1) {
						edgeCodeList[(index + ki) % PP_KK] = 0;
						break;
					}
				}
			}
		}

		index = 0;
		for (int k = 0; k < PP_KK; k++) {
			if (edgeCodeList[k] == 0) {
				edgeCodeList[k] = index;
				index++;
			}
		}
		return;
	}

	assert(PP_KK % RAND_MAX == 0);
	assert(me <= PP_KK);

	int segmentCount = PP_KK / RAND_MAX;
	int segment_me = me / segmentCount;
	int remainder_me = me % segmentCount;

	for (int l = 0; l < segmentCount; l++) {
		for (int k = 1 + l * segment_me; k <= (l + 1) * segment_me; k++) {
			index = rand() + l * RAND_MAX;
			if (edgeCodeList[index] == -1)
				edgeCodeList[index] = 0;
			else {
				for (int ki = 1; ki < RAND_MAX; ki++) {
					if (edgeCodeList[(index + ki) % RAND_MAX] == -1) {
						edgeCodeList[(index + ki) % RAND_MAX] = 0;
						break;
					}
				}
			}
		}
	}

	if (remainder_me != 0) {

		assert(1 + segmentCount * segment_me <= me);
		assert(segmentCount * RAND_MAX + 1 == PP_KK);

		for (int k = 1 + segmentCount * segment_me; k <= me; k++) {
			index = rand() + segmentCount * RAND_MAX;
			if (edgeCodeList[index] == -1)
				edgeCodeList[index] = 0;
			else {
				for (int ki = 1; ki < RAND_MAX; ki++) {
					if (edgeCodeList[(index + ki) % RAND_MAX] == -1) {
						edgeCodeList[(index + ki) % RAND_MAX] = 0;
						break;
					}
				}
			}
		}
	}

	index = 0;
	for (int k = 0; k < PP_KK; k++) {
		if (edgeCodeList[k] == 0) {
			edgeCodeList[k] = index;
			index++;
		}
	}
}

inline void PF_MakeHyperplaneList(PT_vector_T u, int* index_includingHyperplanes, int* mh, double eps) {
	*mh = 0;
	for (int i = 0; i < PD_m; i++) {
		if (SF_PointBelongsHyperplane_i(u, i, eps)) {
			index_includingHyperplanes[*mh] = i;
			(*mh)++;
		}
	}
}

inline void PF_PreparationForIteration(PT_vector_T u) {
	PF_MakeHyperplaneList(u, PD_index_includingHyperplanes, &PD_mh, PP_EPS_POINT_IN_HALFSPACE);
	assert(PD_mh <= PP_MM);

	if (PD_mh < PD_n) {
		if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
			cout << "Starting point u is not vertex with prescision " << PP_EPS_POINT_IN_HALFSPACE << ". Number of including hyperplanes = " << PD_mh
			<< " < n = " << PD_n << "\n";
		cout << "PD_u: ";
		SF_Print_Vector(PD_u);
		cout << "\tSF_ObjF(u) = " << SF_ObjF(PD_u) << endl;
		cout << "Perhaps you should increase the parameter PP_OBJECTIVE_VECTOR_LENGTH or decrease the parameter PP_EPS_POINT_IN_HALFSPACE.\n";
		abort();
	}
	PF_CalculateNumberOfEdges(PD_n, PD_mh, &PD_me);
	PF_MakeEdgeList(PD_edgeCodeList, PD_me);
	PD_TWIDDLE_done = false;
	PD_TWIDDLE_nextEdgeI = 0;
	PF_TWIDDLE_Make_p(PD_TWIDDLE_p, PD_mh, PD_n - 1);
	PD_ma = PD_n - 1;

	PD_objF_u = SF_ObjF(PD_u);
	PD_objF_initialValue = PD_objF_u;
}

inline void PF_Print_Number_of_edges(PT_vector_T x) {
	int mh;
	unsigned long long me;

	mh = 0;
	for (int i = 0; i < PD_m; i++) {
		if (SF_PointBelongsHyperplane_i(x, i, PP_EPS_POINT_IN_HALFSPACE))
			mh++;
	}

	if (mh == PD_n)
		me = (unsigned long long) mh;
	else {
		if (mh > 62) {
			if (BSF_sv_mpiRank == BSF_sv_mpiMaster)
				cout << "\nIt is impossible to calculate binomial coefficient for number of including hyperplanes mh = "
				<< mh << " > 62" << endl;
			abort();
		}
		me = PF_BinomialCoefficient(mh, PD_n - 1);
	}
	cout << me << endl;
}

static inline void PF_PseudoprojectionOnEdge(PT_vector_T v, PT_vector_T w, double eps, int* success) {
	PT_vector_T r;
	PT_vector_T w_previous;
	double dist;
	int iterCount = 0;

	SF_Vector_Copy(v, w);

	do {
		SF_Vector_Zeroing(r);
		SF_Vector_Copy(w, w_previous);

		for (int i = 0; i < PD_ma; i++) {
			int ia = PD_index_activeHyperplanes[i];
			PT_vector_T p;
			SF_OrthogonalProjectingVectorOntoHyperplane_i(w, ia, p);
			SF_Vector_PlusEquals(r, p);
		}

		SF_Vector_DivideEquals(r, PD_ma);
		SF_Vector_Round(r, eps);
		SF_Vector_PlusEquals(w, r);

		dist = SF_Distance_PointToPoint(w, w_previous);
		iterCount++;
		if (iterCount > PP_MAX_PROJECTING_ITER) {
			*success = false;
			break;
		}
	} while (dist >= eps / 10);
}

inline void PF_TWIDDLE // https://doi.org/10.1145/362384.362502
(int* x, int* y, int* z, int p[PP_N + 2], bool* done) {
	int i, j, k;
	j = 0;
	*done = false;

	do {
		j++;
	} while (p[j] <= 0);

	if (p[j - 1] == 0) {
		i = j - 1;
		while (i != 1) {
			p[i] = -1;
			i -= 1;
		}
		p[j] = 0;
		p[1] = *x = *z = 1;
		*y = j;
		return;
	}

	if (j > 1)
		p[j - 1] = 0;

	do {
		j++;
	} while (p[j] > 0);

	i = k = j - 1;

	i++;
	while (p[i] == 0) {
		p[i] = -1;
		i++;
	}

	if (p[i] == -1) {
		p[i] = *z = p[k];
		*x = i;
		*y = k;
		p[k] = -1;
		return;
	}

	if (i == p[0]) {
		*done = true;
		return;
	}

	*z = p[j] = p[i];
	p[i] = 0;
	*x = j;
	*y = i;
}

inline void PF_TWIDDLE_CodeToSubset(int code, int a[PP_MM], int c[PP_N - 1], int n, int m) {
	if (PD_TWIDDLE_nextEdgeI == 0) {
		for (int k = 0; k < m; k++)
			c[k] = a[n - m + k];
		if (code == 0) {
			PD_TWIDDLE_nextEdgeI++;
			return;
		}
	}

	do {
		PF_TWIDDLE(&PD_TWIDDLE_x, &PD_TWIDDLE_y, &PD_TWIDDLE_z, PD_TWIDDLE_p, &PD_TWIDDLE_done);
		//assert(!PD_TWIDDLE_done);
		c[PD_TWIDDLE_z - 1] = a[PD_TWIDDLE_x - 1];
		PD_TWIDDLE_nextEdgeI++;
	} while (PD_TWIDDLE_nextEdgeI < code);
}

inline void PF_TWIDDLE_Make_p(int p[PP_MM + 2], int n, int m) {
	// p - auxiliary integer array for generating all combinations of m out of n objects.
	assert(n >= m && m > 0);
	p[0] = n + 1;
	p[n + 1] = -2;
	for (int j = 1; j <= n - m; j++)
		p[j] = 0;
	for (int j = n - m + 1; j <= n; j++)
		p[j] = j - n + m;
}