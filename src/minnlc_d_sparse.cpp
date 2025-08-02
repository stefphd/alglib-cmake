/*
minnlc_d_sparse example from 
https://www.alglib.net/translator/man/manual.cpp.html#example_minnlc_d_sparse
*/

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "optimization.h"

/** This is to enable the possibility to link a shared-library version of ALGLIB in Windows
 * Basically, the idea is to compile a shared library of ALGLIB with all symbols exported.
 * However, global data symbols need to be declared with __declspec(dllimport) when consuming the 
 * library in Windows; see https://cmake.org/cmake/help/latest/prop_tgt/WINDOWS_EXPORT_ALL_SYMBOLS.html
 * This would require changes in the ALGLIB headers, which is quite annoying.
 * The current workaround is to create instances of the required symbols, and use them in the code 
 * instead of the default ones.
 * Note that this works also when the static library is linked, although not strictly necessary.
 */
static const alglib_impl::ae_uint64_t _i64_xdefault = 0x0;
static const alglib::xparams &xdefault_internal = *((const alglib::xparams *)(&_i64_xdefault));

using namespace alglib;
void  nlcfunc2_sjac(const real_1d_array &x, real_1d_array &fi, sparsematrix &sjac, void *ptr)
{
    //
    // this callback calculates
    //
    //     f0(x0,x1,x2) = x0+x1
    //     f1(x0,x1,x2) = x2-exp(x0)
    //     f2(x0,x1,x2) = x0^2+x1^2-1
    //
    // and Jacobian matrix J = [dfi/dxj].
    //
    // This callback returns Jacobian as a sparse CRS-based matrix. This format is intended
    // for large-scale problems, it allows to solve otherwise intractable tasks with hundreds
    // of thousands of variables. It will also work for our toy problem with just three variables,
    // though.
    //
    //
    // First, we calculate function vector fi[].
    //
    fi[0] = x[0]+x[1];
    fi[1] = x[2]-exp(x[0]);
    fi[2] = x[0]*x[0] + x[1]*x[1] - 1.0;
    
    //
    // After that we initialize sparse Jacobian. On entry to this function sjac is a sparse
    // CRS matrix in a special initial state with N columns but no rows (such matrices can
    // be created with the sparsecreatecrsempty() function ).
    //
    // Such matrices can be used only for sequential addition of rows and nonzero elements.
    // You should add all rows that are expected (one for an objective and one per each
    // nonlinear constraint). Insufficient or excessive rows will be treated as an error.
    // Row elements must be added from left to right, i.e. column indexes must monotonically
    // increase.
    //
    // NOTE: you should NOT reinitialize sjac with sparsecreate() or any other function. It
    //       is important that you append rows/cols to the matrix, but do not create a new
    //       instance of the matrix object. Doing so may cause hard-to-detect errors in
    //       the present or future ALGLIB versions.
    //
    sparseappendemptyrow(sjac, xdefault_internal);
    sparseappendelement(sjac, 0, 1.0, xdefault_internal);
    sparseappendelement(sjac, 1, 1.0, xdefault_internal);
    sparseappendemptyrow(sjac, xdefault_internal);
    sparseappendelement(sjac, 0, -exp(x[0]), xdefault_internal);
    sparseappendelement(sjac, 2, 1.0, xdefault_internal);
    sparseappendemptyrow(sjac, xdefault_internal);
    sparseappendelement(sjac, 0, 2.0*x[0], xdefault_internal);
    sparseappendelement(sjac, 1, 2.0*x[1], xdefault_internal);
}
int main(int argc, char **argv)
{
    try
    {
        //
        // This example demonstrates minimization of
        //
        //     f(x0,x1) = x0+x1
        //
        // subject to nonlinear inequality constraint
        //
        //    x0^2 + x1^2 - 1 <= 0
        //
        // and nonlinear equality constraint
        //
        //    x2-exp(x0) = 0
        //
        // with their Jacobian being a sparse matrix.
        //
        // IMPORTANT: the   MINNLC   optimizer    supports    parallel   numerical
        //            differentiation  ('callback   parallelism').  This  feature,
        //            which  is present  in  commercial  ALGLIB  editions, greatly
        //            accelerates optimization with numerical  differentiation  of
        //            an expensive target functions.
        //
        //            Callback parallelism is usually  beneficial when computing a
        //            numerical gradient requires more than several  milliseconds.
        //            This particular  example,  of  course,  is  not  suited  for
        //            callback parallelism.
        //
        //            See ALGLIB Reference Manual, 'Working with commercial version'
        //            section,  and  comments  on   minnlcoptimize() function  for
        //            more information.
        //
        real_1d_array x0 = "[0,0,0]";
        real_1d_array s = "[1,1,1]";
        double epsx = 0.000001;
        ae_int_t maxits = 0;
        minnlcstate state;
        minnlcreport rep;
        real_1d_array x1;

        //
        // Create optimizer object and tune its settings:
        // * epsx=0.000001  stopping condition for inner iterations
        // * s=[1,1]        all variables have unit scale
        // * upper limit on step length is specified (to avoid probing locations where exp() is large)
        //
        minnlccreate(3, x0, state, xdefault_internal);
        minnlcsetcond(state, epsx, maxits, xdefault_internal);
        minnlcsetscale(state, s, xdefault_internal);
        minnlcsetstpmax(state, 10.0, xdefault_internal);

        //
        // Choose  one  of  nonlinear  programming  solvers  supported  by  MINNLC
        // optimizer.
        //
        // As of ALGLIB 4.02, the only solver which is fully  sparse-capable  is a
        // large-scale filter-based SQP solver, which can utilize sparsity of  the
        // problem and uses a limited-memory BFGS update in order to  be  able  to
        // deal with thousands of variables.
        //
        minnlcsetalgosqp(state, xdefault_internal);

        //
        // Set constraints:
        //
        // Since  version  4.01,  ALGLIB  supports  the  most  general  form of
        // nonlinear constraints: two-sided   constraints  NL<=C(x)<=NU,   with
        // elements being possibly infinite (means that this specific bound  is
        // ignored). It includes equality constraints,  upper/lower  inequality
        // constraints, range constraints. In particular, a pair of constraints
        //
        //        x2-exp(x0)       = 0
        //        x0^2 + x1^2 - 1 <= 0
        //
        // can be specified by passing NL=[0,-INF], NU=[0,0] to minnlcsetnlc2().
        //
        // Constraining functions themselves are passed as part  of  a  problem
        // Jacobian (see below).
        //
        real_1d_array nl = "[0,-inf]";
        real_1d_array nu = "[0,0]";
        minnlcsetnlc2(state, nl, nu, xdefault_internal);

        //
        // Optimize and test results.
        //
        // Optimizer object accepts vector function and its Jacobian, with first
        // component (Jacobian row) being target function, and next components
        // (Jacobian rows) being nonlinear equality and inequality constraints.
        //
        // So, our vector function has form
        //
        //     {f0,f1,f2} = { x0+x1 , x2-exp(x0) , x0^2+x1^2-1 }
        //
        // with Jacobian
        //
        //         [  +1      +1       0 ]
        //     J = [-exp(x0)  0        1 ]
        //         [ 2*x0    2*x1      0 ]
        //
        // with f0 being target function, f1 being equality constraint "f1=0",
        // f2 being inequality constraint "f2<=0". The Jacobian is store as a
        // sparse matrix. See comments on the callback for  more  information
        // about working with sparse Jacobians.
        //
        alglib::minnlcoptimize(state, nlcfunc2_sjac, NULL, NULL, xdefault_internal);
        minnlcresults(state, x1, rep, xdefault_internal);
        printf("%s\n", x1.tostring(2).c_str()); // EXPECTED: [-0.70710,-0.70710,0.49306]
    }
    catch(alglib::ap_error alglib_exception)
    {
        printf("ALGLIB exception with message '%s'\n", alglib_exception.msg.c_str());
        return 1;
    }
    return 0;
}