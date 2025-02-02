/*
 * SparseGrid_chunk_copy_unit_tests.cpp
 *
 *  Created on: Mar 18, 2020
 *      Author: i-bird
 */


#define DISABLE_MPI_WRITTERS

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "SparseGrid/SparseGrid.hpp"
#include "NN/CellList/CellDecomposer.hpp"
#include <math.h>
//#include "util/debug.hpp"
#include "SparseGrid_chunk_copy.hpp"

BOOST_AUTO_TEST_SUITE( sparse_grid_chunk_copy_test )

struct test_chunking3
{
	typedef boost::mpl::vector<boost::mpl::int_<64>,
			                   boost::mpl::int_<8>,
							   boost::mpl::int_<4>> type;

	typedef boost::mpl::vector<boost::mpl::int_<6>,
			                   boost::mpl::int_<3>,
							   boost::mpl::int_<2>> shift;

	typedef boost::mpl::vector<boost::mpl::int_<6>,
			                   boost::mpl::int_<9>,
							   boost::mpl::int_<11>> shift_c;

	typedef boost::mpl::int_<2048> size;
};

BOOST_AUTO_TEST_CASE( sparse_grid_chunk_test )
{
	typedef aggregate<double> T;

	typedef typename v_transform_two<Ft_chunk,boost::mpl::int_<test_chunking3::size::value>,typename T::type>::type chunk_def;

	double chunk_with_border[66*10*6];
	unsigned char mask[66*10*6];

	memset(chunk_with_border,0,sizeof(double)*66*10*6);

	//! vector of chunks
	openfpm::vector<aggregate_bfv<chunk_def>> chunks;
	chunks.resize(1);

	mheader<4096> h;

	memset(&h.mask[0],0xFF,4096);

	for (int i = 0 ; i < test_chunking3::size::value ; i++)
	{chunks.template get<0>(0)[i] = i;}

	// Ok now copy XY

	copy_xy_3<0,0,1,test_chunking3,true>::copy<3,0,66*10*6>(chunk_with_border,mask,h,chunks.get(0));
	copy_xy_3<0,0,1,test_chunking3,true>::copy<0,5,66*10*6>(chunk_with_border,mask,h,chunks.get(0));

	// check
	for (int k = 1 ; k < 9 ; k++)
	{
		for (int j = 1 ; j < 65 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[k*66+j],chunks.template get<0>(0)[3*64*8 + (k-1)*64 + (j-1)]);
		}
	}

	for (int k = 1 ; k < 9 ; k++)
	{
		for (int j = 1 ; j < 65 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[5*66*10+k*66+j],chunks.template get<0>(0)[(k-1)*64 + (j-1)]);
		}
	}


	memset(chunk_with_border,0,sizeof(double)*66*10*6);
	// OK now copy XZ

	copy_xz_3<0,0,1,test_chunking3,true>::copy<7,0,66*10*6>(chunk_with_border,mask,h,chunks.get(0));
	copy_xz_3<0,0,1,test_chunking3,true>::copy<0,9,66*10*6>(chunk_with_border,mask,h,chunks.get(0));

	// check
	for (int i = 1 ; i < 5 ; i++)
	{
		for (int j = 1 ; j < 65 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[i*66*10+j],chunks.template get<0>(0)[(i-1)*64*8 + 7*64 + (j-1)]);
		}
	}

	for (int i = 1 ; i < 5 ; i++)
	{
		for (int j = 1 ; j < 65 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[i*66*10+9*66+j],chunks.template get<0>(0)[(i-1)*64*8 + 0*64 + (j-1)]);
		}
	}

	memset(chunk_with_border,0,sizeof(double)*66*10*6);
	// OK now copy YZ

	copy_yz_3<0,0,1,test_chunking3,true>::copy<63,0,66*10*6>(chunk_with_border,mask,h,chunks.get(0));
	copy_yz_3<0,0,1,test_chunking3,true>::copy<0,65,66*10*6>(chunk_with_border,mask,h,chunks.get(0));

	// check
	for (int i = 1 ; i < 5 ; i++)
	{
		for (int j = 1 ; j < 9 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[i*66*10+j*66],chunks.template get<0>(0)[(i-1)*64*8 + (j-1)*64 + 63]);
		}
	}

	for (int i = 1 ; i < 5 ; i++)
	{
		for (int j = 1 ; j < 9 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[i*66*10+j*66+65],chunks.template get<0>(0)[(i-1)*64*8 + (j-1)*64]);
		}
	}

	// OK now copy x

	memset(chunk_with_border,0,sizeof(double)*66*10*6);

	copy_x_3<0,0,1,test_chunking3,false>::copy<3,0,7,0,66*10*6>(chunk_with_border,chunks.get(0));
	copy_x_3<0,0,1,test_chunking3,false>::copy<0,5,7,0,66*10*6>(chunk_with_border,chunks.get(0));
	copy_x_3<0,0,1,test_chunking3,false>::copy<3,0,0,9,66*10*6>(chunk_with_border,chunks.get(0));
	copy_x_3<0,0,1,test_chunking3,false>::copy<0,5,0,9,66*10*6>(chunk_with_border,chunks.get(0));

	// check
	for (int k = 1 ; k < 65 ; k++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*66*10+0*66+k],chunks.template get<0>(0)[3*64*8 + 7*64 + (k-1)]);
	}

	for (int k = 1 ; k < 65 ; k++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[5*66*10+0*66+k],chunks.template get<0>(0)[0*64*8 + 7*64+(k-1)]);
	}

	// check
	for (int k = 1 ; k < 65 ; k++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*66*10+9*66+k],chunks.template get<0>(0)[3*64*8 + 0*64 + (k-1)]);
	}

	for (int k = 1 ; k < 65 ; k++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[5*66*10+9*66+k],chunks.template get<0>(0)[0*64*8 + 0*64 + (k-1)]);
	}
}


BOOST_AUTO_TEST_CASE( sparse_grid_chunk_test_2 )
{
	typedef aggregate<double> T;

	typedef typename v_transform_two<Ft_chunk,boost::mpl::int_<test_chunking3::size::value>,typename T::type>::type chunk_def;

	double chunk_with_border[68*12*8];
	unsigned char mask[68*12*8];

	memset(chunk_with_border,0,sizeof(double)*68*12*8);

	//! vector of chunks
	openfpm::vector<aggregate_bfv<chunk_def>> chunks;

	chunks.resize(1);

	mheader<4096> h;
	memset(&h.mask[0],0xFF,4096);

	for (int i = 0 ; i < test_chunking3::size::value ; i++)
	{chunks.template get<0>(0)[i] = i;}

	// Ok now copy XY

	copy_xy_3<0,0,2,test_chunking3,true>::copy<2,0,68*12*8>(chunk_with_border,mask,h,chunks.get(0));
	copy_xy_3<0,0,2,test_chunking3,true>::copy<0,6,68*12*8>(chunk_with_border,mask,h,chunks.get(0));

	// check
	for (int s = 0 ; s < 2 ; s++)
	{
	for (int k = 2 ; k < 10 ; k++)
	{
		for (int j = 2 ; j < 66 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[s*68*12 + k*68+j],chunks.template get<0>(0)[s*64*8 + 2*64*8 + (k-2)*64 + (j-2)]);
		}
	}
	}

	for (int s = 0 ; s < 2 ; s++)
	{
	for (int k = 2 ; k < 10 ; k++)
	{
		for (int j = 2 ; j < 66 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[(6+s)*68*12 + k*68+j],chunks.template get<0>(0)[s*64*8 + 0*64*8 + (k-2)*64 + (j-2)]);
		}
	}
	}

	memset(chunk_with_border,0,sizeof(double)*68*12*8);
	// OK now copy XZ

	copy_xz_3<0,0,2,test_chunking3,true>::copy<6,0,66*10*6>(chunk_with_border,mask,h,chunks.get(0));
	copy_xz_3<0,0,2,test_chunking3,true>::copy<0,10,66*10*6>(chunk_with_border,mask,h,chunks.get(0));

	// check
	for (int s = 0 ; s < 2 ; s++)
	{
	for (int i = 2 ; i < 6 ; i++)
	{
		for (int j = 2 ; j < 66 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[i*68*12 + (0+s)*68 +j],chunks.template get<0>(0)[(i-2)*64*8 + (6+s)*64 + (j-2)]);
		}
	}
	}

	// check
	for (int s = 0 ; s < 2 ; s++)
	{
	for (int i = 2 ; i < 6 ; i++)
	{
		for (int j = 2 ; j < 66 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[i*68*12 + (10+s)*68 +j],chunks.template get<0>(0)[(i-2)*64*8 + (0+s)*64 + (j-2)]);
		}
	}
	}

	memset(chunk_with_border,0,sizeof(double)*68*12*8);
	// OK now copy YZ

	copy_yz_3<0,0,2,test_chunking3,true>::copy<62,0,68*12*8>(chunk_with_border,mask,h,chunks.get(0));
	copy_yz_3<0,0,2,test_chunking3,true>::copy<0,66,68*12*8>(chunk_with_border,mask,h,chunks.get(0));

	// check
	for (int s = 0 ; s < 2 ; s++)
	{
	for (int i = 2 ; i < 6 ; i++)
	{
		for (int j = 2 ; j < 10 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[i*68*12+j*68+s],chunks.template get<0>(0)[(i-2)*64*8 + (j-2)*64 + 62+s]);
		}
	}
	}

	for (int s = 0 ; s < 2 ; s++)
	{
	for (int i = 2 ; i < 6 ; i++)
	{
		for (int j = 2 ; j < 10 ; j++)
		{
			BOOST_REQUIRE_EQUAL(chunk_with_border[i*68*12+j*68+s+66],chunks.template get<0>(0)[(i-2)*64*8 + (j-2)*64 + 0+s]);
		}
	}
	}

	// OK now copy x

	memset(chunk_with_border,0,sizeof(double)*68*12*8);

	copy_x_3<0,0,2,test_chunking3,false>::copy<2,0,6,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_x_3<0,0,2,test_chunking3,false>::copy<0,6,6,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_x_3<0,0,2,test_chunking3,false>::copy<2,0,0,10,68*12*8>(chunk_with_border,chunks.get(0));
	copy_x_3<0,0,2,test_chunking3,false>::copy<0,6,0,10,68*12*8>(chunk_with_border,chunks.get(0));

	// check
	for (int k = 2 ; k < 66 ; k++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+0*68+k],chunks.template get<0>(0)[2*64*8 + 6*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+0*68+k],chunks.template get<0>(0)[3*64*8 + 6*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+1*68+k],chunks.template get<0>(0)[2*64*8 + 7*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+1*68+k],chunks.template get<0>(0)[3*64*8 + 7*64 + (k-2)]);
	}

	for (int k = 2 ; k < 66 ; k++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+0*68+k],chunks.template get<0>(0)[0*64*8 + 6*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+0*68+k],chunks.template get<0>(0)[1*64*8 + 6*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+1*68+k],chunks.template get<0>(0)[0*64*8 + 7*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+1*68+k],chunks.template get<0>(0)[1*64*8 + 7*64 + (k-2)]);
	}

	// check
	for (int k = 2 ; k < 66 ; k++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+10*68+k],chunks.template get<0>(0)[2*64*8 + 0*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+10*68+k],chunks.template get<0>(0)[3*64*8 + 0*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+11*68+k],chunks.template get<0>(0)[2*64*8 + 1*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+11*68+k],chunks.template get<0>(0)[3*64*8 + 1*64 + (k-2)]);
	}

	for (int k = 2 ; k < 66 ; k++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+10*68+k],chunks.template get<0>(0)[0*64*8 + 0*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+10*68+k],chunks.template get<0>(0)[1*64*8 + 0*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+11*68+k],chunks.template get<0>(0)[0*64*8 + 1*64 + (k-2)]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+11*68+k],chunks.template get<0>(0)[1*64*8 + 1*64 + (k-2)]);
	}

	// OK now copy y

	memset(chunk_with_border,0,sizeof(double)*68*12*8);

	copy_y_3<0,0,2,test_chunking3,false>::copy<2,0,62,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_y_3<0,0,2,test_chunking3,false>::copy<0,6,62,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_y_3<0,0,2,test_chunking3,false>::copy<2,0,0,66,68*12*8>(chunk_with_border,chunks.get(0));
	copy_y_3<0,0,2,test_chunking3,false>::copy<0,6,0,66,68*12*8>(chunk_with_border,chunks.get(0));

	// check
	for (int j = 2 ; j < 10 ; j++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+j*68+0],chunks.template get<0>(0)[2*64*8 + (j-2)*64 + 62]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+j*68+0],chunks.template get<0>(0)[3*64*8 + (j-2)*64 + 62]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+j*68+1],chunks.template get<0>(0)[2*64*8 + (j-2)*64 + 63]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+j*68+1],chunks.template get<0>(0)[3*64*8 + (j-2)*64 + 63]);
	}

	for (int j = 2 ; j < 10 ; j++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+j*68+0],chunks.template get<0>(0)[2*64*8 + (j-2)*64 + 62]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+j*68+0],chunks.template get<0>(0)[3*64*8 + (j-2)*64 + 62]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+j*68+1],chunks.template get<0>(0)[2*64*8 + (j-2)*64 + 63]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+j*68+1],chunks.template get<0>(0)[3*64*8 + (j-2)*64 + 63]);
	}

	// check
	for (int j = 2 ; j < 10 ; j++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+j*68+66],chunks.template get<0>(0)[2*64*8 + (j-2)*64 +  0]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+j*68+66],chunks.template get<0>(0)[3*64*8 + (j-2)*64 +  0]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+j*68+67],chunks.template get<0>(0)[2*64*8 + (j-2)*64 +  1]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+j*68+67],chunks.template get<0>(0)[3*64*8 + (j-2)*64 +  1]);
	}

	for (int j = 2 ; j < 10 ; j++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+j*68+66],chunks.template get<0>(0)[0*64*8 + (j-2)*64 + 0]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+j*68+66],chunks.template get<0>(0)[1*64*8 + (j-2)*64 + 0]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+j*68+67],chunks.template get<0>(0)[0*64*8 + (j-2)*64 + 1]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+j*68+67],chunks.template get<0>(0)[1*64*8 + (j-2)*64 + 1]);
	}

	// OK now copy Z

	memset(chunk_with_border,0,sizeof(double)*68*12*8);

	copy_z_3<0,0,2,test_chunking3,false>::copy<6,0,62,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_z_3<0,0,2,test_chunking3,false>::copy<0,10,62,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_z_3<0,0,2,test_chunking3,false>::copy<6,0,0,66,68*12*8>(chunk_with_border,chunks.get(0));
	copy_z_3<0,0,2,test_chunking3,false>::copy<0,10,0,66,68*12*8>(chunk_with_border,chunks.get(0));

	// check
/*	for (int j = 2 ; j < 6 ; j++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+0*68+0],chunks.template get<0>(0)[(j-2)*64*8 + 6*64 + 62]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+1*68+0],chunks.template get<0>(0)[(j-2)*64*8 + 7*64 + 62]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+0*68+1],chunks.template get<0>(0)[(j-2)*64*8 + 6*64 + 63]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+1*68+1],chunks.template get<0>(0)[(j-2)*64*8 + 7*64 + 63]);
	}*/

/*	for (int j = 2 ; j < 6 ; j++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+10*68+0],chunks.template get<0>(0)[(j-2)*64*8 + 0*64 + 62]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+11*68+0],chunks.template get<0>(0)[(j-2)*64*8 + 1*64 + 62]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+10*68+1],chunks.template get<0>(0)[(j-2)*64*8 + 0*64 + 63]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+11*68+1],chunks.template get<0>(0)[(j-2)*64*8 + 1*64 + 63]);
	}

	// check
	for (int j = 2 ; j < 6 ; j++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+0*68+66],chunks.template get<0>(0)[(j-2)*64*8 + 6*64 +  0]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+1*68+66],chunks.template get<0>(0)[(j-2)*64*8 + 7*64 +  0]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+0*68+67],chunks.template get<0>(0)[(j-2)*64*8 + 6*64 +  1]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+1*68+67],chunks.template get<0>(0)[(j-2)*64*8 + 7*64 +  1]);
	}

	for (int j = 2 ; j < 6 ; j++)
	{
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+10*68+66],chunks.template get<0>(0)[(j-2)*64*8 + 0*64 + 0]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+11*68+66],chunks.template get<0>(0)[(j-2)*64*8 + 1*64 + 0]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+10*68+67],chunks.template get<0>(0)[(j-2)*64*8 + 0*64 + 1]);
		BOOST_REQUIRE_EQUAL(chunk_with_border[j*68*12+11*68+67],chunks.template get<0>(0)[(j-2)*64*8 + 1*64 + 1]);
	}*/

	// Copy corner

/*	memset(chunk_with_border,0,sizeof(double)*68*12*8);

	copy_corner_3<0,0,2,test_chunking3,false>::copy<2,0,6,0,62,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_corner_3<0,0,2,test_chunking3,false>::copy<2,0,6,0,0,66,68*12*8>(chunk_with_border,chunks.get(0));
	copy_corner_3<0,0,2,test_chunking3,false>::copy<2,0,0,10,62,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_corner_3<0,0,2,test_chunking3,false>::copy<2,0,0,10,0,66,68*12*8>(chunk_with_border,chunks.get(0));

	copy_corner_3<0,0,2,test_chunking3,false>::copy<0,6,6,0,62,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_corner_3<0,0,2,test_chunking3,false>::copy<0,6,6,0,0,66,68*12*8>(chunk_with_border,chunks.get(0));
	copy_corner_3<0,0,2,test_chunking3,false>::copy<0,6,0,10,62,0,68*12*8>(chunk_with_border,chunks.get(0));
	copy_corner_3<0,0,2,test_chunking3,false>::copy<0,6,0,10,0,66,68*12*8>(chunk_with_border,chunks.get(0));


	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+0*68+0],chunks.template get<0>(0)[2*64*8 + 6*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+0*68+1],chunks.template get<0>(0)[2*64*8 + 6*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+1*68+0],chunks.template get<0>(0)[2*64*8 + 7*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+1*68+1],chunks.template get<0>(0)[2*64*8 + 7*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+0*68+0],chunks.template get<0>(0)[3*64*8 + 6*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+0*68+1],chunks.template get<0>(0)[3*64*8 + 6*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+1*68+0],chunks.template get<0>(0)[3*64*8 + 7*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+1*68+1],chunks.template get<0>(0)[3*64*8 + 7*64 + 63]);

	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+0*68+66],chunks.template get<0>(0)[2*64*8 + 6*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+0*68+67],chunks.template get<0>(0)[2*64*8 + 6*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+1*68+66],chunks.template get<0>(0)[2*64*8 + 7*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+1*68+67],chunks.template get<0>(0)[2*64*8 + 7*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+0*68+66],chunks.template get<0>(0)[3*64*8 + 6*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+0*68+67],chunks.template get<0>(0)[3*64*8 + 6*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+1*68+66],chunks.template get<0>(0)[3*64*8 + 7*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+1*68+67],chunks.template get<0>(0)[3*64*8 + 7*64 + 1]);

	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+10*68+0],chunks.template get<0>(0)[2*64*8 + 0*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+10*68+1],chunks.template get<0>(0)[2*64*8 + 0*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+11*68+0],chunks.template get<0>(0)[2*64*8 + 1*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+11*68+1],chunks.template get<0>(0)[2*64*8 + 1*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+10*68+0],chunks.template get<0>(0)[3*64*8 + 0*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+10*68+1],chunks.template get<0>(0)[3*64*8 + 0*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+11*68+0],chunks.template get<0>(0)[3*64*8 + 1*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+11*68+1],chunks.template get<0>(0)[3*64*8 + 1*64 + 63]);

	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+10*68+66],chunks.template get<0>(0)[2*64*8 + 0*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+10*68+67],chunks.template get<0>(0)[2*64*8 + 0*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+11*68+66],chunks.template get<0>(0)[2*64*8 + 1*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[0*68*12+11*68+67],chunks.template get<0>(0)[2*64*8 + 1*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+10*68+66],chunks.template get<0>(0)[3*64*8 + 0*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+10*68+67],chunks.template get<0>(0)[3*64*8 + 0*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+11*68+66],chunks.template get<0>(0)[3*64*8 + 1*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[1*68*12+11*68+67],chunks.template get<0>(0)[3*64*8 + 1*64 + 1]);

    ///////////////////////////////////////////////////

	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+0*68+0],chunks.template get<0>(0)[0*64*8 + 6*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+0*68+1],chunks.template get<0>(0)[0*64*8 + 6*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+1*68+0],chunks.template get<0>(0)[0*64*8 + 7*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+1*68+1],chunks.template get<0>(0)[0*64*8 + 7*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+0*68+0],chunks.template get<0>(0)[1*64*8 + 6*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+0*68+1],chunks.template get<0>(0)[1*64*8 + 6*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+1*68+0],chunks.template get<0>(0)[1*64*8 + 7*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+1*68+1],chunks.template get<0>(0)[1*64*8 + 7*64 + 63]);

	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+0*68+66],chunks.template get<0>(0)[0*64*8 + 6*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+0*68+67],chunks.template get<0>(0)[0*64*8 + 6*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+1*68+66],chunks.template get<0>(0)[0*64*8 + 7*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+1*68+67],chunks.template get<0>(0)[0*64*8 + 7*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+0*68+66],chunks.template get<0>(0)[1*64*8 + 6*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+0*68+67],chunks.template get<0>(0)[1*64*8 + 6*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+1*68+66],chunks.template get<0>(0)[1*64*8 + 7*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+1*68+67],chunks.template get<0>(0)[1*64*8 + 7*64 + 1]);

	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+10*68+0],chunks.template get<0>(0)[0*64*8 + 0*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+10*68+1],chunks.template get<0>(0)[0*64*8 + 0*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+11*68+0],chunks.template get<0>(0)[0*64*8 + 1*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+11*68+1],chunks.template get<0>(0)[0*64*8 + 1*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+10*68+0],chunks.template get<0>(0)[1*64*8 + 0*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+10*68+1],chunks.template get<0>(0)[1*64*8 + 0*64 + 63]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+11*68+0],chunks.template get<0>(0)[1*64*8 + 1*64 + 62]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+11*68+1],chunks.template get<0>(0)[1*64*8 + 1*64 + 63]);

	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+10*68+66],chunks.template get<0>(0)[0*64*8 + 0*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+10*68+67],chunks.template get<0>(0)[0*64*8 + 0*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+11*68+66],chunks.template get<0>(0)[0*64*8 + 1*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[6*68*12+11*68+67],chunks.template get<0>(0)[0*64*8 + 1*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+10*68+66],chunks.template get<0>(0)[1*64*8 + 0*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+10*68+67],chunks.template get<0>(0)[1*64*8 + 0*64 + 1]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+11*68+66],chunks.template get<0>(0)[1*64*8 + 1*64 + 0]);
	BOOST_REQUIRE_EQUAL(chunk_with_border[7*68*12+11*68+67],chunks.template get<0>(0)[1*64*8 + 1*64 + 1]);*/
}


BOOST_AUTO_TEST_CASE( sparse_grid_chunk_missalign_copy_test )
{
	unsigned char mp[16*16*16];
	short int mp_off[16*16*16];

	// Aligned
	Box<3,long int> box_src({17,19,3},{18,100,100});
	Box<3,long int> box_dst({33,19,3},{34,100,100});

	openfpm::vector<key_int<3>> vk;

	construct_chunk_missalign_map<3,16*16*16,default_chunking<3>>(mp,mp_off,box_src,box_dst,vk);

	size_t sz[3] = {16,16,16};
	grid_sm<3,void> g_sm(sz);

	for (int i = 0 ; i < 16 ; i++)
	{
		for (int j = 0 ; j < 16 ; j++)
		{
			for (int k = 0 ; k < 16 ; k++)
			{
				size_t id = g_sm.Lin(k,j,i);

				BOOST_REQUIRE_EQUAL(mp[id],13);
				BOOST_REQUIRE_EQUAL(mp_off[id],id);
			}
		}
	}

	BOOST_REQUIRE_EQUAL(vk.size(),1);

	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(0),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(1),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(2),0);

	BOOST_REQUIRE_EQUAL(vk.get(0).i,0);

	// Miss-Aligned
	Box<3,long int> box_src2({17,19,3},{18,100,100});
	Box<3,long int> box_dst2({34,19,3},{35,100,100});

	vk.clear();

	construct_chunk_missalign_map<3,16*16*16,default_chunking<3>>(mp,mp_off,box_src2,box_dst2,vk);

	for (int i = 0 ; i < 16 ; i++)
	{
		for (int j = 0 ; j < 16 ; j++)
		{
			for (int k = 0 ; k < 16 ; k++)
			{
				size_t id = g_sm.Lin(i,j,k);

				if (k < 15)
				{
					BOOST_REQUIRE_EQUAL(mp[id],13);
					BOOST_REQUIRE_EQUAL(mp_off[id],id+1);
				}

				if (k == 15)
				{
					BOOST_REQUIRE_EQUAL(mp[id],14);
					BOOST_REQUIRE_EQUAL(mp_off[id],j*16+i*16*16);
				}
			}
		}
	}

	BOOST_REQUIRE_EQUAL(vk.size(),1);

	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(0),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(1),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(2),0);


	BOOST_REQUIRE_EQUAL(vk.get(0).i,0);


	// Miss-Aligned with 2 entry

	// Miss-Aligned
	Box<3,long int> box_src3({17,19,3},{30,100,100});
	Box<3,long int> box_dst3({34,19,3},{47,100,100});

	vk.clear();

	construct_chunk_missalign_map<3,16*16*16,default_chunking<3>>(mp,mp_off,box_src3,box_dst3,vk);

	for (int i = 0 ; i < 16 ; i++)
	{
		for (int j = 0 ; j < 16 ; j++)
		{
			for (int k = 0 ; k < 16 ; k++)
			{
				size_t id = g_sm.Lin(i,j,k);

				if (k < 15)
				{
					BOOST_REQUIRE_EQUAL(mp[id],13);
					BOOST_REQUIRE_EQUAL(mp_off[id],id+1);
				}

				if (k == 15)
				{
					BOOST_REQUIRE_EQUAL(mp[id],14);
					BOOST_REQUIRE_EQUAL(mp_off[id],j*16+i*16*16);
				}
			}
		}
	}

	BOOST_REQUIRE_EQUAL(vk.size(),2);

	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(0),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(1),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(2),0);

	BOOST_REQUIRE_EQUAL(vk.get(1).k.get(0),1);
	BOOST_REQUIRE_EQUAL(vk.get(1).k.get(1),0);
	BOOST_REQUIRE_EQUAL(vk.get(1).k.get(2),0);

	BOOST_REQUIRE_EQUAL(vk.get(0).i,0);

	// Miss-Aligned xy with 4 entry

	// Miss-Aligned
	Box<3,long int> box_src4({17,19,3},{30,100,100});
	Box<3,long int> box_dst4({34,20,3},{47,101,100});

	vk.clear();

	construct_chunk_missalign_map<3,16*16*16,default_chunking<3>>(mp,mp_off,box_src4,box_dst4,vk);

	for (int i = 0 ; i < 16 ; i++)
	{
		for (int j = 0 ; j < 16 ; j++)
		{
			for (int k = 0 ; k < 16 ; k++)
			{
				size_t id = g_sm.Lin(i,j,k);

				if (k < 15)
				{
					if (j < 15)
					{
						BOOST_REQUIRE_EQUAL(mp[id],13);
						BOOST_REQUIRE_EQUAL(mp_off[id],k+1+(j+1)*16+i*16*16);
					}
					else
					{
						BOOST_REQUIRE_EQUAL(mp[id],16);
						BOOST_REQUIRE_EQUAL(mp_off[id],k+1+i*16*16);
					}
				}

				if (k == 15)
				{
					if (j < 15)
					{
						BOOST_REQUIRE_EQUAL(mp[id],14);
						BOOST_REQUIRE_EQUAL(mp_off[id],(j+1)*16+i*16*16);
					}
					else
					{
						BOOST_REQUIRE_EQUAL(mp[id],17);
						BOOST_REQUIRE_EQUAL(mp_off[id],i*16*16);
					}
				}
			}
		}
	}

	BOOST_REQUIRE_EQUAL(vk.size(),4);

	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(0),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(1),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(2),0);

	BOOST_REQUIRE_EQUAL(vk.get(1).k.get(0),1);
	BOOST_REQUIRE_EQUAL(vk.get(1).k.get(1),0);
	BOOST_REQUIRE_EQUAL(vk.get(1).k.get(2),0);

	BOOST_REQUIRE_EQUAL(vk.get(2).k.get(0),0);
	BOOST_REQUIRE_EQUAL(vk.get(2).k.get(1),1);
	BOOST_REQUIRE_EQUAL(vk.get(2).k.get(2),0);

	BOOST_REQUIRE_EQUAL(vk.get(3).k.get(0),1);
	BOOST_REQUIRE_EQUAL(vk.get(3).k.get(1),1);
	BOOST_REQUIRE_EQUAL(vk.get(3).k.get(2),0);

	// Now check the opposite

	// Miss-Aligned
	Box<3,long int> box_src5({17,19,3},{18,100,100});
	Box<3,long int> box_dst5({32,19,3},{33,100,100});

	vk.clear();

	construct_chunk_missalign_map<3,16*16*16,default_chunking<3>>(mp,mp_off,box_src5,box_dst5,vk);

	for (int i = 0 ; i < 16 ; i++)
	{
		for (int j = 0 ; j < 16 ; j++)
		{
			for (int k = 0 ; k < 16 ; k++)
			{
				size_t id = g_sm.Lin(i,j,k);

				if (k == 0)
				{
					BOOST_REQUIRE_EQUAL(mp[id],12);
					BOOST_REQUIRE_EQUAL(mp_off[id],15+j*16+i*16*16);
				}

				if (k > 0)
				{
					BOOST_REQUIRE_EQUAL(mp[id],13);
					BOOST_REQUIRE_EQUAL(mp_off[id],(k-1)+j*16+i*16*16);
				}
			}
		}
	}

	BOOST_REQUIRE_EQUAL(vk.size(),1);

	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(0),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(1),0);
	BOOST_REQUIRE_EQUAL(vk.get(0).k.get(2),0);


	BOOST_REQUIRE_EQUAL(vk.get(0).i,0);
}

BOOST_AUTO_TEST_SUITE_END()
