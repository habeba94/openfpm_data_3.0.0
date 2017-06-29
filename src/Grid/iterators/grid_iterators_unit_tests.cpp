/*
 * grid_iterators_unit_tests.hpp
 *
 *  Created on: Jun 27, 2016
 *      Author: i-bird
 */

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "Grid/iterators/grid_skin_iterator.hpp"
#include "Grid/map_grid.hpp"
#include "data_type/aggregate.hpp"
#include "Grid/iterators/grid_key_dx_iterator_stencil.hpp"

BOOST_AUTO_TEST_SUITE( grid_iterators_tests )

template <unsigned int dim> void test_skin_iterator(Box<3,size_t> & bx1, Box<3,size_t> & bx2, grid_sm<3,void> & g_sm,size_t (& bc)[dim] , size_t vol_test)
{
	grid_cpu<3,aggregate<size_t>> gtest(g_sm.getSize());
	gtest.setMemory();
	auto it = gtest.getSubIterator(0);

	while (it.isNext())
	{
		auto key = it.get();

		gtest.get<0>(key) = 0;

		++it;
	}

	size_t count = 0;
	grid_skin_iterator_bc<3> gsi(g_sm,bx1,bx2,bc);

	while (gsi.isNext() == true)
	{
		auto key = gsi.get();

		gtest.get<0>(key) += 1;

		count++;

		++gsi;
	}

	BOOST_REQUIRE_EQUAL(count,(size_t)vol_test);

	bool ret = true;
	auto it2 = gtest.getSubIterator(0);

	while (it2.isNext())
	{
		auto key = it2.get();

		ret &= gtest.get<0>(key) <= 1;

		++it2;
	}

	BOOST_REQUIRE_EQUAL(ret,true);
}

BOOST_AUTO_TEST_CASE( grid_skin_iterator_test )
{
	// Test box inside box

	size_t sz[] = {112,112,112};
	Box<3,size_t> bx1({20,25,30},{90,93,98});
	Box<3,size_t> bx2({18,23,28},{92,95,100});
	Box<3,size_t> bx3({20,25,30},{90,93,30});
	Box<3,size_t> bx4({20,25,30},{90,93,50});
	Box<3,size_t> bx5({19,24,29},{90,93,50});

	grid_sm<3,void> g_sm(sz);
	size_t bc[] = {PERIODIC,PERIODIC,PERIODIC};

	// Volume calculation

	size_t tot = 1;

	tot *= bx2.getHigh(0) - bx2.getLow(0) + 1;
	tot *= bx2.getHigh(1) - bx2.getLow(1) + 1;
	tot *= bx2.getHigh(2) - bx2.getLow(2) + 1;

	size_t tot_ = 1;

	tot_ *= bx1.getHigh(0) - bx1.getLow(0) + 1 - 2;
	tot_ *= bx1.getHigh(1) - bx1.getLow(1) + 1 - 2;
	tot_ *= bx1.getHigh(2) - bx1.getLow(2) + 1 - 2;

	tot -= tot_;

	test_skin_iterator<3>(bx1,bx2,g_sm,bc,tot);
	test_skin_iterator<3>(bx2,bx1,g_sm,bc,0);
	test_skin_iterator<3>(bx3,bx3,g_sm,bc,4899);

	Point<3,size_t> p({60,60,60});

	bx2 += p;
	bx1 += p;

	test_skin_iterator<3>(bx1,bx2,g_sm,bc,tot);
	test_skin_iterator<3>(bx2,bx1,g_sm,bc,0);

	test_skin_iterator<3>(bx4,bx4,g_sm,bc,15042);
	test_skin_iterator<3>(bx5,bx4,g_sm,bc,7679);
}

void test_stencil_iterator(grid_sm<3,void> & g_sm)
{
	grid_cpu<3,aggregate<long int>> gtest(g_sm.getSize());
	gtest.setMemory();
	auto it = gtest.getSubIterator(0);

	while (it.isNext())
	{
		auto key = it.get();

		gtest.get<0>(key) = key.get(0) + key.get(1) + key.get(2);

		++it;
	}

	grid_key_dx<3> stencil[1];
	stencil[0].set_d(0,0);
	stencil[0].set_d(1,0);
	stencil[0].set_d(2,0);

	bool ret = true;
	grid_key_dx_iterator<3,stencil_offset_compute<3,1>> gsi(g_sm,stencil);

	while (gsi.isNext() == true)
	{
		auto key = gsi.get();
		auto lin = gsi.getStencil<0>();

		ret &= (gtest.get<0>(lin) == key.get(0) + key.get(1) + key.get(2));

		++gsi;
	}


	BOOST_REQUIRE_EQUAL(ret,true);
}

BOOST_AUTO_TEST_CASE( grid_iterator_stencil_test )
{
	size_t sz[] = {52,52,52};
	grid_sm<3,void> g_sm(sz);
	test_stencil_iterator(g_sm);
}

static grid_key_dx<3> star_stencil_3D[7] = {{0,0,0},
                                         {0,0,-1},
										 {0,0,1},
										 {0,-1,0},
										 {0,1,0},
										 {-1,0,0},
										 {1,0,0}};

void test_stencil_sub_iterator(grid_sm<3,void> & g_sm)
{
	grid_cpu<3,aggregate<long int>> gtest(g_sm.getSize());
	gtest.setMemory();
	auto it = gtest.getSubIterator(0);

	while (it.isNext())
	{
		auto key = it.get();

		gtest.get<0>(key) = key.get(0) + key.get(1) + key.get(2);

		++it;
	}

	grid_key_dx<3> start({1,1,1});
	grid_key_dx<3> stop({(long int)gtest.getGrid().size(0)-2,(long int)gtest.getGrid().size(1)-2,(long int)gtest.getGrid().size(2)-2});

	bool ret = true;
	grid_key_dx_iterator_sub<3,stencil_offset_compute<3,7>> gsi(g_sm,start,stop,star_stencil_3D);

	while (gsi.isNext() == true)
	{
		size_t lin1 = gsi.getStencil<0>();
		size_t lin2 = gsi.getStencil<1>();
		size_t lin3 = gsi.getStencil<2>();
		size_t lin4 = gsi.getStencil<3>();
		size_t lin5 = gsi.getStencil<4>();
		size_t lin6 = gsi.getStencil<5>();
		size_t lin7 = gsi.getStencil<6>();


		size_t sum = 6*gtest.get<0>(lin1) -
				     gtest.get<0>(lin2) -
					 gtest.get<0>(lin3) -
					 gtest.get<0>(lin4) -
					 gtest.get<0>(lin5) -
					 gtest.get<0>(lin6) -
					 gtest.get<0>(lin7);

		ret &= (sum == 0);

		++gsi;
	}


	BOOST_REQUIRE_EQUAL(ret,true);
}

BOOST_AUTO_TEST_CASE( grid_iterator_sub_stencil_test )
{
	size_t sz[] = {52,52,52};
	grid_sm<3,void> g_sm(sz);
	test_stencil_sub_iterator(g_sm);
}

BOOST_AUTO_TEST_SUITE_END()

