//============================================================================
//                                  I B E X                                   
// File        : ibex_ExprLabel.cpp
// Author      : Gilles Chabert
// Copyright   : Ecole des Mines de Nantes (France)
// License     : See the LICENSE file
// Created     : May 22, 2012
// Last Update : May 22, 2012
//============================================================================


#include "ibex_ExprLabel.h"

namespace ibex {

ostream& operator<<(ostream& os, const ExprLabel& l) {
	if (l.d) os << "d=" << *l.d << " ";
	if (l.g) os << "g=" << *l.g << " ";
	return os;
}

} // end namespace ibex



