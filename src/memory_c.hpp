/*
 * memory_c.hpp
 *
 *  Created on: Aug 17, 2014
 *      Author: Pietro Incardona
 */

#include <boost/multi_array.hpp>
#include <array>
#include "ct_array.hpp"

#ifndef MEMORY_C_HPP_
#define MEMORY_C_HPP_

/*!
 * \brief This class is a container for the memory interface
 *
 * This class is a container for the memory interface. It give the possibility
 * to have two specialization, one when the memory interface is full known
 * at compile time, and one when is not-known at compile time.
 * It internally store two objects
 *
 * mem is object used to allocate device/host/... memory
 * mem_r is a representation of this memory as an array of objects T
 *
 */

template<typename T, typename D = memory>
class memory_c
{
	public:

	//! define T
	typedef memory_c<T> type;

	//! define a reference to T
	typedef T& reference;

	//! compile time specialization object that allocate memory
	D * mem;

	//! object that represent the memory as an array of objects T
	memory_array<T> * mem_r;

	/*! \brief This function set the object that allocate memory
	 *
	 * This object set the object that allocate memory
	 *
	 * \param the memory object
	 *
	 */

	void setMemory(memory & mem)
	{
		this->mem = &mem;
	}

	/*! \brief This function allocate memory and associate the representation to mem_r
	 *
	 * This function allocate memory and associate the representation of that chunk of
	 * memory to mem_r
	 *
	 */
	bool allocate(const size_t sz)
	{
		memory * mem = this->mem;

		//! We create a chunk of memory
	    mem->resize( sz*sizeof(T) );

	    //! we create the representation for this buffer
	    mem_r = new memory_array<T>(mem->getPointer());

	    return true;
	}

	//! constructor
	memory_c(){}
};

/*! \brief This class is a trick to indicate the compiler a specific
 *  specialization pattern
 *
 * This class is a trick to indicate the compiler a specific specialization
 * pattern, in particular it say that T is a multidimensional array and
 * need a special treatment, T is suppose to be a boost::mpl::vector of
 * unsigned int indicating each dimension. T has to be a type of known size at
 * compile time
 *
 */

template<typename T>
class multi_array
{
	typedef T type;
};

/*! \brief This class is a trick to indicate the compiler a specific
 *  specialization pattern
 *
 * This class is a trick to indicate the compiler a specific specialization
 * pattern, in particular it say that T is a key and
 * need a special treatment. T is suppose to be a boost::mpl::vector of
 * any type, (Actually does not have an application but is a generalization
 * of multi-array). T has to be a type of known size at compile time
 *
 */

template<typename T>
class key
{
	typedef T type;
};


/*! \brief this class multiply all the unsigned element in a boost::mpl::vector
 *
 * this class multiply all the unsigned element in a boost::mpl::vector excluding
 * the first element
 *
 * \param T expecting a boost::mpl::vector
 *
 */
template<typename T, unsigned int N>
struct mult
{
	enum { value = mult<T,N-1>::value * boost::mpl::at<T,boost::mpl::int_<N>>::type::value };
};

template <typename T>
struct mult<T,1>
{
	enum { value = boost::mpl::at<T,boost::mpl::int_<1>>::type::value };
};

/*! \brief Specialization of memory_c for multi_array
 *
 * Specialization of memory_c for multi_array
 *
 * \param T is suppose to be a boost::mpl::vector specifing at position 0 the type and at
 * position 1 to N the dimensions size of the multi_array
 *
 */

template<typename T>
class memory_c<multi_array<T>, memory>
{
	//! define T
//	typedef T type;

	//! define boost::mpl::int_ without boost::mpl
	template<int S> using int_ = boost::mpl::int_<S>;

	//! define the template vector size it give a number at compile time
	typedef typename boost::mpl::size<T> size_p;

	//! Define "at" meta function without boost::mpl
	template< typename S, unsigned int n> using at = boost::mpl::at<S,boost::mpl::int_<n> >;

	typedef typename at<T,0>::type base;

	//! define size_type
	typedef typename boost::multi_array_ref<base,size_p::value>::size_type size_type;


	/*! \brief In combination with generate_array is used to produce array at compile-time
	 *
	 * In combination with generate_array is used to produce at compile-time
	 * arrays like {0,N-1,.........0} used in boost::multi_array for index ordering
	 *
	 */

	template<size_t index,size_t N> struct ordering
	{
	    enum { value = (N-index) % N };
	};

	public:

	/*! \brief This function set the object that allocate memory
	 *
	 * This object set the object that allocate memory
	 *
	 * \param the memory object
	 *
	 */

	void setMemory(memory & mem)
	{
		this->mem = &mem;
	}

	/*! \brief This function allocate memory and associate the representation to mem_r
	 *
	 * This function allocate memory and associate the representation of that chunk of
	 * memory to mem_r
	 *
	 */
	bool allocate(const size_t sz)
	{
		memory * mem = this->mem;

		//! We create a chunk of memory
	    mem->resize( sz*mult<T,size_p::value-1>::value*sizeof(base) );

	    // We create an array dims from the boost::mpl::vector
	    typedef typename generate_array_dims<T>::result dims;

	    //! we define the dimension of the full multi_array buffer
	    boost::array<size_type ,size_p::value> dimensions = {sz,{dims::data}};

	    std::cout << "Dimensions \n";

	    for (int i = 0; i < size_p::value ; i++)
	    {
	    	std::cout << dimensions[i] << "    ";
	    }

	    //! we generate the ordering buffer ord::data = {0,N-1 ...... 1 }
	    typedef typename generate_array<size_p::value, ordering>::result ord;

	    //! we define the index ordering
	    const typename boost::multi_array<T,size_p::value>::size_type ordering[size_p::value] = {*ord::data};

	    std::cout << "Ordering \n";

	    for (int i = 0; i < size_p::value ; i++)
	    {
	    	std::cout << dimensions[i] << "    ";
	    }

	    //! we define the ascending order
	    bool ascending[] = {true,true,true,true,true,true};

	    //! we create the representation for this buffer
	    mem_r = new boost::multi_array_ref<base,size_p::value>(static_cast<base *>(mem->getPointer()),dimensions,boost::general_storage_order<size_p::value>(ordering,ascending));

	    return true;
	}

	//! define the type of the multi_array vector minus one of the original size
	//! basically we remove the index 0 of the multi_array
	typedef boost::multi_array<base,size_p::value> type;

	//! object that represent the memory as an multi-dimensional array of objects T
	boost::multi_array_ref<base,boost::mpl::size<T>::value> * mem_r;

	//! Reference to an object to allocate memory
	memory * mem;

	//! constructor
	memory_c()
	{}

	//! set the device memory interface, the object that allocate memory
	void set_mem(memory & mem)
	{
		mem = new boost::shared_ptr<memory>(&mem);
	}
};


// Compile based specialization ( Should be automated at some point )

#include "memory_config.h"

#endif

