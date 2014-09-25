/* ============================================================================
 * I B E X - CellHeap Tests
 * ============================================================================
 * Copyright   : Ecole des Mines de Nantes (FRANCE)
 * License     : This program can be distributed under the terms of the GNU LGPL.
 *               See the file COPYING.LESSER.
 *
 * Author(s)   : Jordan Ninin
 * Created     : Mar 2, 2014
 * ---------------------------------------------------------------------------- */

#include "TestCellHeap.h"
#include "ibex_CellHeapOptim.h"
#include "ibex_CellHeap_2.h"
#include "ibex_CellDoubleHeap.h"

using namespace std;

namespace ibex {


void TestCellHeap::test00() {

	int nb= 10;
	CellHeapVarLB h1(1);

	double _box[][2] = {{5,15},  {15,25}};
	IntervalVector box(2,_box);
	OptimCell *cell;
	for (int i=0; i<nb ;i++) {
		cell = new OptimCell((pow(-1,i)*i)*box);
		h1.push(cell);
	}
	for (int i=0; i<nb ;i++) {
		delete h1.pop();
	}

	TEST_ASSERT(h1.size()==0);
}


void TestCellHeap::test01() {

	int nb= 10;
	CellHeapOptim h1(1,CellHeapOptim::LB);
	CellHeapVarLB h2(1);

	double _box[][2] = {{5,15},  {15,25}};
	IntervalVector box(2,_box);
	OptimCell *cell;
	for (int i=0; i<nb ;i++) {
		cell = new OptimCell((pow(-1,i)*i)*box);
		h1.push(cell);
		h2.push(cell);
	}


	check(h1.top()->box,h2.top()->box);
	check(h1.size(),h2.size());
}

void TestCellHeap::test02() {

	int nb= 10;
	CellHeapOptim h1(1,CellHeapOptim::UB);
	CellHeapVarUB h2(1);

	double _box[][2] = {{5,15},  {15,25},  {25,35},  {35,45}};
	IntervalVector box(4,_box);
	OptimCell *cell;
	for (int i=0; i<nb ;i++) {
		cell = new OptimCell((pow(-1,i)*i)*box);
		h1.push(cell);
		h2.push(cell);
	}
	h1.pop(); h1.pop();
	h2.pop(); h2.pop();

	check(h1.top()->box,h2.top()->box);
	check(h1.size(),h2.size());
}


void TestCellHeap::test03() {

	int nb= 10;
	CellHeapOptim h1(1,CellHeapOptim::C3);
	CellHeapCost h2(CellHeap_2::C3);
	h2.setLoup(100);

	double _box[][2] = {{5,15},  {15,25},  {25,35},  {35,45}};
	IntervalVector box(4,_box);
	OptimCell *cell1 ,*cell2;
	for (int i=0; i<nb ;i++) {
		cell1 = new OptimCell((pow(-1,i)*i)*box);
		cell1->pu=0.2;
		cell1->pf = box[0]*box[1];
		cell1->loup = 100;
		cell2 = new OptimCell((pow(-1,i)*i)*box);
		cell2->pu=cell1->pu;
		cell2->pf = cell1->pf;
		cell2->loup = cell1->loup;
		h1.push(cell1);
		h2.push(cell2);
	}
	delete h1.pop(); delete h1.pop();
	delete h2.pop(); delete h2.pop();

	check(h2.cost(*h1.top()),h2.cost(*h2.top()));
	check(h1.size(),h2.size());
}


void TestCellHeap::test04() {

	int nb= 10;
	CellHeapOptim h1(1,CellHeapOptim::LB);
	CellHeapVarLB h2(1);

	double _box[][2] = {{5,15},  {15,25},  {25,35},  {35,45}};
	IntervalVector box(4,_box);
	OptimCell *cell1 ,*cell2;
	for (int i=0; i<nb ;i++) {
		cell1 = new OptimCell((pow(-1,i)*i)*box);
		cell1->pu=rand();
		cell1->pf = box[0]*box[1];
		cell2 = new OptimCell((pow(-1,i)*i)*box);
		cell2->pu=cell1->pu;
		cell2->pf = box[0]*box[1];
		h1.push(cell1);
		h2.push(cell2);
	}
	delete h1.pop(); delete h1.pop();
	delete h2.pop(); delete h2.pop();
	h1.contract_heap(50);
	h2.contract_heap(50);
	delete h1.pop(); delete h1.pop();
	delete h2.pop(); delete h2.pop();

	check(h1.top()->box,h2.top()->box);
	check(h1.size(),h2.size());
}


void TestCellHeap::test05() {

	int nb= 10;

	CellHeapCost h1(CellHeap_2::C5);
	CellHeapCost h2(CellHeap_2::C5);

	double _box[][2] = {{5,15},  {15,25},  {25,35},  {35,45}};
	IntervalVector box(4,_box);
	OptimCell *cell1 ,*cell2;
	for (int i=0; i<nb ;i++) {
		cell1 = new OptimCell((pow(-1,i)*i)*box);
		cell1->pu=rand();
		cell1->pf = box[0]*box[1];
		cell2 = new OptimCell((pow(-1,i)*i)*box);
		cell2->pu=cell1->pu;
		cell2->pf = box[0]*box[1];
		h1.push(cell1);
		h2.push(cell2);
	}
	delete h1.pop(); delete h1.pop();
	delete h2.pop(); delete h2.pop();
	h1.sort();
	h2.sort();
	delete h1.pop(); delete h1.pop();
	delete h2.pop(); delete h2.pop();

	check(h1.top()->box,h2.top()->box);
	check(h1.size(),h2.size());

	h1.flush();
	TEST_ASSERT(h1.size()==0);
}







void TestCellHeap::test_D00() {

	int nb= 10;
	CellDoubleHeap h1(0);

	double _box[][2] = {{5,15},  {15,25}};
	IntervalVector box(2,_box);
	OptimCell *cell;
	for (int i=0; i<nb ;i++) {
		cell = new OptimCell((pow(-1,i)*i)*box);
		h1.push(cell);
	}
	for (int i=0; i<nb ;i++) {
		delete h1.pop();
	}

	TEST_ASSERT(h1.size()==0);
}


void TestCellHeap::test_D01() {

	int nb= 10;
	CellDoubleHeap h1(1,50,CellHeap_2::C5);
	h1.setLoup(100);


	double _box[][2] = {{5,15},  {15,25}};
	IntervalVector box(2,_box);
	OptimCell *cell;
	for (int i=0; i<nb ;i++) {
		cell = new OptimCell((pow(-1,i)*i)*box);
		cell->pu = 0.5;
		cell->pf = box[0];
		h1.push(cell);
	}
	for (int i=0; i<nb ;i++) {
		delete h1.pop();
	}


	TEST_ASSERT(h1.size()==0);
}


void TestCellHeap::test_D03() {

	int nb= 10;
	CellDoubleHeap h1(1,100,CellHeap_2::C5);
	h1.setLoup(100);

	double _box[][2] = {{5,15},  {15,25},  {25,35},  {35,45}};
	IntervalVector box(4,_box);
	OptimCell *cell1 ;
	for (int i=0; i<nb ;i++) {
		cell1 = new OptimCell((pow(-1,i)*i)*box);
		cell1->pu=0.2;
		cell1->pf = box[0]*box[1];
		cell1->loup = 100;
		h1.push(cell1);
	}
	delete h1.pop(); delete h1.pop();

	TEST_ASSERT(h1.size()==nb-2);
}


void TestCellHeap::test_D04() {


	int nb= 10;
	CellDoubleHeap h1(1,100,CellHeap_2::C5);
	h1.setLoup(10);

	double _box[][2] = {{5,15},  {15,25},  {25,35},  {35,45}};
	IntervalVector box(4,_box);
	OptimCell *cell1 ;
	for (int i=0; i<nb ;i++) {
		cell1 = new OptimCell((pow(-1,i)*i)*box);
		cell1->pu=0.2;
		cell1->pf = box[0]*box[1];
		cell1->loup = 10;
		h1.push(cell1);
	}
	delete h1.pop(); delete h1.pop();
	h1.contract_heap(100);
	delete h1.pop(); delete h1.pop();
	cout<< h1<<endl;
	h1.flush();
}


void TestCellHeap::test_D05() {

	int nb= 10;

	CellDoubleHeap h1(CellHeap_2::C5);
	CellDoubleHeap h2(CellHeap_2::C5);
	h1.setLoup(100);
	h2.setLoup(100);

	double _box[][2] = {{5,15},  {15,25},  {25,35},  {35,45}};
	IntervalVector box(4,_box);
	OptimCell *cell1 ,*cell2;
	for (int i=0; i<nb ;i++) {
		cell1 = new OptimCell((pow(-1,i)*i)*box);
		cell1->pu=rand();
		cell1->pf = box[0]*box[1];
		cell2 = new OptimCell((pow(-1,i)*i)*box);
		cell2->pu=cell1->pu;
		cell2->pf = box[0]*box[1];
		h1.push(cell1);
		h2.push(cell2);
	}
	delete h1.pop(); delete h1.pop();
	delete h2.pop(); delete h2.pop();
	h1.sort();
	h2.sort();
	delete h1.pop(); delete h1.pop();
	delete h2.pop(); delete h2.pop();

	check(h1.top()->box,h2.top()->box);
	check(h1.size(),h2.size());

	h1.flush();
	TEST_ASSERT(h1.size()==0);
}



} // end namespace
