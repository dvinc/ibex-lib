/* ============================================================================
 * I B E X - Parser Tests
 * ============================================================================
 * Copyright   : Ecole des Mines de Nantes (FRANCE)
 * License     : This program can be distributed under the terms of the GNU LGPL.
 *               See the file COPYING.LESSER.
 *
 * Author(s)   : Gilles Chabert
 * Created     : Apr 02, 2012
 * ---------------------------------------------------------------------------- */

#ifndef __TEST_PARSER_H__
#define __TEST_PARSER_H__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

namespace ibex {

class TestParser : public CppUnit::TestFixture {

public:

	CPPUNIT_TEST_SUITE(TestParser);
	
		CPPUNIT_TEST(var01);

		CPPUNIT_TEST(const01);
		CPPUNIT_TEST(const02);
		CPPUNIT_TEST(const03);
		CPPUNIT_TEST(const04);
		CPPUNIT_TEST(const05);
		CPPUNIT_TEST(const06);
		CPPUNIT_TEST(const07);
		CPPUNIT_TEST(const08);

		CPPUNIT_TEST(ponts);
		CPPUNIT_TEST(choco01);
		CPPUNIT_TEST(func01);
		CPPUNIT_TEST(func02);
		CPPUNIT_TEST(func03);
		CPPUNIT_TEST(loop01);

		CPPUNIT_TEST(nary_max);
		//		CPPUNIT_TEST(error01);
	CPPUNIT_TEST_SUITE_END();

	void var01();

	void const01();
	void const02();
	void const03();
	void const04();
	void const05();
	void const06();
	void const07();
	// test hexadecimal constant
	void const08();

	void func01();
	void func02();
	void func03();
	void ponts();
	void choco01();
	void error01();
	void loop01();

	void nary_max();

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestParser);


} // end namespace

#endif // __TEST_PARSER_H__
