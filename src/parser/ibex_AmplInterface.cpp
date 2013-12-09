//============================================================================
//                                  I B E X
// File        : ibex_AmplInterface.cpp Adapted from CouenneAmplInterface
// Author      : Jordan Ninin
// License     : See the LICENSE file
// Created     : Nov 5, 2013
// Last Update : Nov 5, 2013
//============================================================================

#include "ibex_AmplInterface.h"
#include "ibex_Exception.h"

#include "asl.h"
#include "nlp.h"
#include "getstub.h"
#include "opcode.hd"
#include <stdint.h>


#define OBJ_DE    ((const ASL_fg *) asl) -> I.obj_de_
#define VAR_E     ((const ASL_fg *) asl) -> I.var_e_
#define CON_DE    ((const ASL_fg *) asl) -> I.con_de_
#define OBJ_sense ((const ASL_fg *) asl) -> i.objtype_
#define CEXPS1 ((const ASL_fg *) asl) -> I.cexps1_
#define CEXPS ((const ASL_fg *) asl) -> I.cexps_

#include "r_opn.hd" /* for N_OPS */

static fint timing = 0;

static
keyword keywds[] = { // must be alphabetical
		KW(const_cast<char*>("timing"), L_val, &timing, const_cast<char*>("display timings for the run")),
};

static
Option_Info Oinfo = { const_cast<char*>("testampl"), const_cast<char*>("ANALYSIS TEST"),
		const_cast<char*>("concert_options"), keywds, nkeywds, 0, const_cast<char*>("ANALYSIS TEST") };


/////////////////////////////////////////////////////////////////////////////////////////
/* $Id: invmap.cpp 577 2011-05-21 20:38:48Z pbelotti $
 *
 * Name:    invmap.cpp
 * Author:  Pietro Belotti
 * Purpose: create a bijection between ASL's efunc and integer to
 *          inversely map e->op fields into constant operators
 *
 * (C) Carnegie-Mellon University, 2006-11.
 * This file is licensed under the Eclipse Public License (EPL)
 */

/* couples an ASL function pointer with the relative operator constant */

typedef struct {
	efunc *fp;
	int    op;
} AslCouPair;


/* compare two AslCoupair's, used in qsort and bsearch below */

/* AW: 2007-06-11: changed b/c of problems with MSVC++ */
/* inline int pair_compare (const void *p1, const void *p2) { */
static int pair_compare (const void *p1, const void *p2) {

	/* FIX! weak cast for 64 bit machines */

	register size_t f1 = Intcast (((AslCouPair *) p1) -> fp);
	register size_t f2 = Intcast (((AslCouPair *) p2) -> fp);

	if      (f1 < f2) return -1;
	else if (f1 > f2) return  1;
	else return 0;
}


/* array of pairs (efunc2*, int) that relates all operators */

AslCouPair opmap [N_OPS];


/* binary search to get operator number from its efunc2* (the type of e->op) */

size_t getOperator (efunc *f) {

	static char first_call = 1;
	AslCouPair key, *res;

	/* FIX cast for 64 bit machines */

	if ((Intcast f <  N_OPS) &&
			(Intcast f > -N_OPS))
		return Intcast f;

	key.fp = f;

	if (first_call) { /* opmap is still empty, fill it using values from r_ops [] */

		register int i=0;
		register AslCouPair *ops = opmap;

		/* fill opmap vector with inverse correspondence pairs efunc -> int */
		while (i<N_OPS) {
			ops -> fp = r_ops [ops -> op = i++];
			ops++;
		}

		/* sort opmap for later use with bsearch */
		qsort (opmap, N_OPS, sizeof (AslCouPair), pair_compare);
		first_call = 0;
	}

	/* find int operator through binary search */
	res = (AslCouPair *) bsearch (&key, opmap, N_OPS, sizeof (AslCouPair), pair_compare);

	if (!res)
		return -1;

	return res -> op;
}



// (C++) code starts here ///////////////////////////////////////////////////////////////////////////

namespace ibex {

AmplInterface::AmplInterface(std::string nlfile) :
															_problem(NULL),	_bound_init(NULL), asl(NULL), _nlfile(nlfile), _x(NULL){

	if (!readASLfg())
		ibex_error("Fail to read the ampl file.\n");

	_problem = new SystemFactory();

	if (!readnl()) {
		delete _problem;
		_problem = NULL;
		ibex_error("Fail to read the nl file.\n");
	}

}

AmplInterface::~AmplInterface() {
	delete _problem;
	delete _bound_init;
	delete _x;

	if (asl) {
		ASL_free(&asl);
	}
}

bool AmplInterface::writeSolution(double * sol, bool found) {
	const char* message;

	//TODO setup a nicer message
	if (found) {
		message = "IBEX found a solution.\n";
	} else {
		message = "IBEX could not found a solution.\n";
	}

	write_sol(const_cast<char*>(message), sol, NULL, NULL);

	return true;
}

// create an AMPL problem by using ASL interface to the .nl file
System AmplInterface::getSystem() {
	if (_problem)
		return System(*_problem);
	else
		return NULL;
}




// Reads a NLP from an AMPL .nl file through the ASL methods
bool AmplInterface::readASLfg() {
	assert(asl == NULL);

	if (_nlfile == "")
		return false;

	char** argv = new char*[3];
	argv[0] = const_cast<char*>("dummy");
	argv[1] = strdup(_nlfile.c_str());
	argv[2] = NULL;

	// Create the ASL structure
	asl = (ASL*) ASL_alloc (ASL_read_fg);

	char* stub = getstub (&argv, &Oinfo);

	// Although very intuitive, we shall explain why the second argument
	// is passed with a minus sign: it is to tell the ASL to retrieve
	// the nonlinear information too.
	FILE* nl = jac0dim (stub, - (fint) strlen (stub));

	// Set options in the asl structure
	want_xpi0 = 1 | 2;  // allocate initial values for primal and dual if available
	obj_no = 0;         // always want to work with the first (and only?) objective

	// read the rest of the nl file
	fg_read (nl, ASL_return_read_err | ASL_findgroups);

	//FIXME freeing argv and argv[1] gives segfault !!!
	//  free(argv[1]);
	//  delete[] argv;

	return true;
}




// Reads a NLP from an AMPL .nl file through the ASL methods
bool AmplInterface::readnl() {


	if (!_problem) {return false;}

	_x =new Variable(n_var);
	_bound_init = new IntervalVector(n_var, Interval::ALL_REALS);
	_problem->add_var(*_x);


	// Each has a linear and a nonlinear part (thanks to Dominique
	// Orban: http://www.gerad.ca/~orban/drampl/def-vars.html)

	try {

		// objective functions /////////////////////////////////////////////////////////////
		if (n_obj>1) {ibex_error("too much objective function"); return false;}

		for (int i = 0; i < n_obj; i++) {

			///////////////////////////////////////////////////
			//  the nonlinear part
			const ExprNode *body = &(nl2expr (OBJ_DE [i] . e));

			////////////////////////////////////////////////
			// The linear part
			// count nonzero terms in linear part

			int nterms = 0;
			for (ograd *objgrad = Ograd [i];
					objgrad;
					objgrad = objgrad -> next)
				if (fabs (objgrad -> coef) > 1.e-18) // FIXME enlever la precision en dure
					nterms++;

			if (nterms) { // have linear terms
				for (ograd *objgrad = Ograd [i]; objgrad; objgrad = objgrad -> next) {
					if (fabs (objgrad -> coef) > 1.e-18) {// FIXME enlever la precision en dure

						int coeff = objgrad -> coef;
						int index = objgrad -> varno;
						if (coeff==1) {
							body = &(*body + (*_x)[index]);
						} else {
							body = &(*body +coeff * (*_x)[index]);
						}
					}
				}
			}

			// 3rd/ASL/solvers/asl.h, line 336: 0 is minimization, 1 is maximization
			if (OBJ_sense [i] == 0) {
				_problem->add_goal(*body);
			} else {
				_problem->add_goal(-(*body));
			}
		}


		// constraints ///////////////////////////////////////////////////////////////////

		// count the linear part of the constraints

		int *nterms = new int [n_con];
		const ExprNode **body_con = new const ExprNode*[n_con];
		// allocate space for argument list of all constraints' summations
		// of linear and nonlinear terms

		// init array of each constraint with the nonlinear part
		for (int i = 0; i<n_con;i++)
			body_con[i]=&(nl2expr (CON_DE [i] . e));

		// count all linear terms
		if (A_colstarts && A_vals)    {      // Constraints' linear info is stored in A_vals

			for (int j = 0; j < n_var; j++){
				for (register int i = A_colstarts [j], k = A_colstarts [j+1] - i; k--; i++) {
					if (A_vals[i]==1) {
						body_con[A_rownos [i]] = &(*(body_con[A_rownos [i]]) + (*_x)[j]);
					} else {
						body_con[A_rownos [i]] = &(*(body_con[A_rownos [i]]) + (A_vals[i]) * (*_x)[j]);
					}
				}
			}
		} else {
			cgrad *congrad;                  // Constraints' linear info is stored in Cgrad
			for ( int i = 0; i < n_con; i++)
				for (congrad = Cgrad [i]; congrad; congrad = congrad -> next) {
					if (fabs (congrad -> coef) > 1.e-18) {// FIXME enlever la precision en dure
						int coeff = congrad -> coef;
						int index = congrad -> varno;
						if (coeff==1) {
							body_con[i] = &(*(body_con[i]) + (*_x)[index]);
						} else {
							body_con[i] = &(*(body_con[i]) + (coeff) * (*_x)[index]);
						}
					}
				}
		}
		//
		for (int i = 0; i < n_con; i++) {

			CmpOp sign;
			double lb, ub;

			/* LUrhs is the constraint lower bound if Urhsx!=0, and the constraint lower and upper bound if Uvx == 0 */
			if (Urhsx) {
				lb = LUrhs [i];
				ub = Urhsx [i];
			} else {
				int j = 2*i;
				lb = LUrhs [j];
				ub = LUrhs [j+1];
			}

			// set constraint sign
			if (negInfinity < lb)
				if (ub < Infinity) {sign = IBEX_EQ; }
				else                sign = IBEX_GEQ;
			else                    sign = IBEX_LEQ;


			// add them (and set lower-upper bound)
			switch (sign) {

			case  IBEX_EQ :  _problem->add_ctr(ExprCtr(*(body_con[i])-Interval(lb,ub),IBEX_EQ)); break;
			case  IBEX_LEQ:  _problem->add_ctr(ExprCtr(*(body_con[i])-ub,IBEX_LEQ)); break;
			case  IBEX_GEQ:  _problem->add_ctr(ExprCtr(*(body_con[i])-lb,IBEX_GEQ)); break;
			default: ibex_error("Error: could not recognize a constraint\n"); return false;
			}
		}
		delete[] body_con;

	} catch (...) {
		return false;
	}


	// lower and upper bounds ///////////////////////////////////////////////////////////////

	if (LUv) {
		real *Uvx_copy = Uvx;
		/* LUv is the variable lower bound if Uvx!=0, and the variable lower and upper bound if Uvx == 0 */
		if (!Uvx_copy)
			for (register int i=0; i<n_var; i++) {

				register int j = 2*i;
				(*_bound_init)[i] = Interval(  ((LUv[j]   <= NEG_INFINITY) ? NEG_INFINITY : LUv[j]  ),
						((LUv[j+1] >= POS_INFINITY) ? POS_INFINITY : LUv[j+1]) );
			}
		else
			for (register int i=n_var; i--;) {
				(*_bound_init)[i] = Interval(	(LUv [i]      <= NEG_INFINITY ? NEG_INFINITY : LUv[i]     ),
						(Uvx_copy [i] >= POS_INFINITY ? POS_INFINITY : Uvx_copy[i]) );
			}

	} // else it is [-oo,+oo]


	return true;
}

// converts an AMPL expression (sub)tree into an expression* (sub)tree
// thank to Dominique Orban for the explication of the DAG inside AMPL:
// http://www.gerad.ca/~orban/drampl/dag.html
const ExprNode& AmplInterface::nl2expr(expr *e) {

	switch (getOperator (e -> op)) {

	case OPPLUS:  return   ((nl2expr (e -> L.e)) + (nl2expr (e -> R.e)));
	case OPMINUS: return   ((nl2expr (e -> L.e)) - (nl2expr (e -> R.e)));
	case OPMULT:  return   ((nl2expr (e -> L.e)) * (nl2expr (e -> R.e)));
	case OPDIV:   return   ((nl2expr (e -> L.e)) / (nl2expr (e -> R.e)));
	//case OPREM:   notimpl ("remainder");
	case OPPOW:   return   pow(nl2expr (e -> L.e), nl2expr (e -> R.e));
	//case OPLESS:  notimpl ("less");
	case MINLIST: {
		expr **ep = e->L.ep;
		const ExprNode* ee=&(nl2expr (*ep));
		ep++;
		while (ep < e->R.ep) {
			ee = & (min(*ee , nl2expr (*ep)));
			ep++;
		}
		return *ee;
	}
	case MAXLIST:  {
		expr **ep = e->L.ep;
		const ExprNode* ee=&(nl2expr (*ep));
		ep++;
		while (ep < e->R.ep) {
			ee = & (max(*ee , nl2expr (*ep)));
			ep++;
		}
		return *ee;
	}
	//case FLOOR:   notimpl ("floor");
	//case CEIL:    notimpl ("ceil");
	case ABS:     return (abs(nl2expr (e -> L.e)));
	case OPUMINUS:return (-(nl2expr (e -> L.e)));
	//case OPIFnl:  // TODO return (chi(nl2expr(????))) BoolInterval??
	// see ASL/solvers/rops.c, see f_OPIFnl and  expr_if

	case OP_tanh: return tanh(nl2expr (e->L.e));

	case OP_tan:   return tan(nl2expr (e->L.e));
	case OP_sqrt:  return sqrt(nl2expr (e -> L.e));
	case OP_sinh:  return sinh(nl2expr (e -> L.e));
	case OP_sin:   return sin(nl2expr (e -> L.e));
	case OP_log10: return ((ExprConstant::new_scalar(1.0/log(Interval(10.0)))) * log(nl2expr (e -> L.e)));
	case OP_log:   return log(nl2expr (e -> L.e));
	case OP_exp:   return exp(nl2expr (e -> L.e));
	case OP_cosh:  return cosh(nl2expr (e->L.e));
	case OP_cos:   return cos(nl2expr (e -> L.e));
	//case OP_atanh: notimpl ("atanh");
	case OP_atan2: return atan2(nl2expr (e -> L.e), nl2expr (e -> R.e));
	case OP_atan:  return tan(nl2expr (e -> L.e));
	//case OP_asinh: notimpl ("asinh");
	case OP_asin:  return asin(nl2expr (e -> L.e));
	//case OP_acosh: notimpl ("acosh");
	case OP_acos:  return acos(nl2expr (e -> L.e));

	case OPSUMLIST: {
		expr **ep = e->L.ep;
		const ExprNode* ee=&(nl2expr (*ep));
		ep++;
		while (ep < e->R.ep) {
			ee = & (*ee + nl2expr (*ep));
			ep++;
		}
		return *ee;
	}
	//case OPintDIV: notimpl ("intdiv");
	//case OPprecision: notimpl ("precision");
	//case OPround:  notimpl ("round");
	//case OPtrunc:  notimpl ("trunc");

	case OP1POW: return pow( (nl2expr (e -> L.e)), ExprConstant::new_scalar(((expr_n *)e->R.e)->v));
	case OP2POW: return sqr( nl2expr (e -> L.e));
	case OPCPOW: return pow(ExprConstant::new_scalar(((expr_n *)e->L.e)->v), nl2expr (e -> R.e));
	//case OPFUNCALL: notimpl ("function call");
	case OPNUM:     return (ExprConstant::new_scalar(((expr_n *)e)->v));
	//case OPPLTERM:  notimpl ("plterm");
	//case OPIFSYM:   notimpl ("ifsym");
	//case OPHOL:     notimpl ("hol");
	case OPVARVAL:  {
		int j = ((expr_v *) e) -> a;

		if (j<n_var) {
			return (*_x)[j];
		}
		// http://www.gerad.ca/~orban/drampl/def-vars.html
		//else if (j <= n_var + como + comc + comb + como1 + comc1) {
		// common expression | defined variable

		int k = (expr_v *)e - VAR_E;
		if( k >= n_var ) {
			// This is a common expression. Find pointer to its root.
			j = k - n_var;
			if( j < ncom0 ) {
				cexp *common = CEXPS + j;
				// init with the nonlinear part
				const ExprNode* body = &(nl2expr (common->e));

				int nlin = common->nlin; // Number of linear terms
				if( nlin > 0 ) {
					linpart * L = common->L;
					for(int i = 1; i < nlin; i++ ) {
						int coeff = L [i]. fac;
						int index = ((uintptr_t) (L [j].v.rp) - (uintptr_t) VAR_E) / sizeof (expr_v);
						body = &(*body +coeff * (*_x)[index]);
					}
				}
				return *body;
			}
			else {
				cexp1 *common = (CEXPS1 - ncom0) +j ;
				// init with the nonlinear part
				const ExprNode* body = &(nl2expr (common->e));

				int nlin = common->nlin; // Number of linear terms
				if( nlin > 0 ) {
					linpart * L = common->L;
					for(int i = 1; i < nlin; i++ ) {
						int coeff = L [i]. fac;
						int index = ((uintptr_t) (L [j].v.rp) - (uintptr_t) VAR_E) / sizeof (expr_v);
						body = &(*body +coeff * (*_x)[index]);
					}
				}
				return *body;
			}

		} else {
			ibex_error("Error: unknown variable x \n");
			throw -1;
		}

	}

	default: {
		ibex_error( "ERROR: unknown operator, aborting.\n");
		throw -2;
	}
	}

	return ExprConstant::new_scalar(0.);
}





}