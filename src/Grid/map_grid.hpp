#ifndef MAP_HPP_
#define MAP_HPP_

#include "config.h"

//! Warning: apparently you cannot used nested boost::mpl with boost::fusion
//! can create template circularity, this include avoid the problem
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container/vector/vector_fwd.hpp>
#include <boost/fusion/include/vector_fwd.hpp>
#include <boost/type_traits.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include "memory_ly/memory_conf.hpp"
#include "util/meta_copy.hpp"
#include "Memleak_check.hpp"
#include "util/for_each_ref.hpp"
#include "util.hpp"
#include <utility>
#ifdef CUDA_GPU
#include "memory/CudaMemory.cuh"
#endif
#include "grid_sm.hpp"
#include "Encap.hpp"
#include "memory_ly/memory_array.hpp"
#include "memory_ly/memory_c.hpp"
#include <vector>
#include "se_grid.hpp"



// Debugging macro

/*! \brief this class is a functor for "for_each" algorithm
 *
 * This class is a functor for "for_each" algorithm. For each
 * element of the boost::vector the operator() is called.
 * Is mainly used to copy one object into one target
 * grid  element in a generic way for a
 * generic object T with variable number of property
 *
 * \tparam dim Dimensionality
 * \tparam S type of grid
 * \tparam Memory type of memory needed for encap
 *
 */

template<unsigned int dim, typename S, typename Memory>
struct copy_cpu_encap
{
	//! size to allocate
	grid_key_dx<dim> & key;

	//! grid where we have to store the data
	S & grid_dst;

	//! type of the object we have to set
	typedef typename S::value_type obj_type;

	//! type of the object boost::sequence
	typedef typename S::value_type::type ov_seq;

	//! object we have to store
	const encapc<1,obj_type,Memory> & obj;

	/*! \brief constructor
	 *
	 * It define the copy parameters.
	 *
	 * \param key which element we are modifying
	 * \param grid_dst grid we are updating
	 * \param obj object we have to set in grid_dst (encapsulated)
	 *
	 */
	copy_cpu_encap(grid_key_dx<dim> & key, S & grid_dst, const encapc<1,obj_type,Memory> & obj)
	:key(key),grid_dst(grid_dst),obj(obj){};


#ifdef SE_CLASS1
	/*! \brief Constructor
	 *
	 * Calling this constructor produce an error. This class store the reference of the object,
	 * this mean that the object passed must not be a temporal object
	 *
	 */
	copy_cpu_encap(grid_key_dx<dim> & key, S & grid_dst, const encapc<1,obj_type,Memory> && obj)
	:key(key),grid_dst(grid_dst),obj(obj)
	{std::cerr << "Error: " <<__FILE__ << ":" << __LINE__ << " Passing a temporal object";};
#endif

	//! It call the copy function for each property
	template<typename T>
	void operator()(T& t) const
	{
		// This is the type of the object we have to copy
		typedef typename boost::fusion::result_of::at_c<ov_seq,T::value>::type copy_type;

		// Remove the reference from the type to copy
		typedef typename boost::remove_reference<copy_type>::type copy_rtype;

		meta_copy<copy_rtype> cp(obj.template get<T::value>(),grid_dst.template get<T::value>(key));
	}
};

/*! \brief this class is a functor for "for_each" algorithm
 *
 * This class is a functor for "for_each" algorithm. For each
 * element of the boost::vector the operator() is called.
 * Is mainly used to copy one object into one target
 * grid  element in a generic way for a
 * generic object T with variable number of property
 *
 * \param dim Dimensionality
 * \param S type of grid
 *
 */

template<unsigned int dim, typename S>
struct copy_cpu
{
	//! size to allocate
	grid_key_dx<dim> & key;

	//! grid where we have to store the data
	S & grid_dst;

	//! type of the object we have to set
	typedef typename S::value_type obj_type;

	//! type of the object boost::sequence
	typedef typename S::value_type::type ov_seq;

	//! object we have to store
	const obj_type & obj;

	/*! \brief constructor
	 *
	 * It define the copy parameters.
	 *
	 * \param key which element we are modifying
	 * \param grid_dst grid we are updating
	 * \param obj object we have to set in grid_dst
	 *
	 */
	copy_cpu(grid_key_dx<dim> & key, S & grid_dst, const obj_type & obj)
	:key(key),grid_dst(grid_dst),obj(obj){};

#ifdef SE_CLASS1
	/*! \brief Constructor
	 *
	 * Calling this constructor produce an error. This class store the reference of the object,
	 * this mean that the object passed must not be a temporal object
	 *
	 */
	copy_cpu(grid_key_dx<dim> & key, S & grid_dst, const obj_type && obj)
	:key(key),grid_dst(grid_dst),obj(obj)
	{std::cerr << "Error: " <<__FILE__ << ":" << __LINE__ << " Passing a temporal object\n";};
#endif

	//! It call the copy function for each property
	template<typename T>
	void operator()(T& t) const
	{
		// This is the type of the object we have to copy
		typedef typename boost::fusion::result_of::at_c<ov_seq,T::value>::type copy_type;

		// Remove the reference from the type to copy
		typedef typename boost::remove_reference<copy_type>::type copy_rtype;

		meta_copy<copy_rtype> cp(boost::fusion::at_c<T::value>(obj.data),grid_dst.template get<T::value>(key));
	}
};

/*! \brief this class is a functor for "for_each" algorithm
 *
 * This class is a functor for "for_each" algorithm. For each
 * element of the boost::vector the operator() is called.
 * Is mainly used to copy one source grid element into one target
 * grid element in a generic way for an object T with variable
 * number of property
 *
 * \param dim Dimensionality
 * \param S grid type
 *
 */

template<unsigned int dim, typename S>
struct copy_cpu_sd
{
	//! size to allocate
	grid_key_dx<dim> & key;

	//! Source grid
	const S & grid_src;

	//! Destination grid
	S & grid_dst;

	//! type of the object boost::sequence
	typedef typename S::value_type::type ov_seq;

	//! constructor
	copy_cpu_sd(grid_key_dx<dim> & key, const S & grid_src, S & grid_dst)
	:key(key),grid_src(grid_src),grid_dst(grid_dst){};

	//! It call the copy function for each member
	template<typename T>
	void operator()(T& t) const
	{
		// This is the type of the object we have to copy
		typedef typename boost::fusion::result_of::at_c<ov_seq,T::value>::type copy_type;

		// Remove the reference from the type to copy
		typedef typename boost::remove_reference<copy_type>::type copy_rtype;

		meta_copy<copy_rtype> cp(grid_src.template get<T::value>(key),grid_dst.template get<T::value>(key));
	}
};


/*! \brief this class is a functor for "for_each" algorithm
 *
 * This class is a functor for "for_each" algorithm. For each
 * element of the boost::vector the operator() is called.
 * Is mainly used to copy one source grid element into one target
 * grid element in a generic way for an object T with variable
 * number of property
 *
 * \param dim Dimensionality
 * \param S grid type
 *
 */

template<unsigned int dim, typename S>
struct copy_cpu_sd_k
{
	//! source key
	grid_key_dx<dim> & key_s;

	//! destination key
	grid_key_dx<dim> & key_d;

	//! Source grid
	const S & grid_src;

	//! Destination grid
	S & grid_dst;

	//! type of the object boost::sequence
	typedef typename S::value_type::type ov_seq;

	//! constructor
	copy_cpu_sd_k(grid_key_dx<dim> & key_s, grid_key_dx<dim> & key_d, const S & grid_src, S & grid_dst)
	:key_s(key_s),key_d(key_d),grid_src(grid_src),grid_dst(grid_dst){};

	//! It call the copy function for each member
	template<typename T>
	void operator()(T& t) const
	{
		// This is the type of the object we have to copy
		typedef typename boost::fusion::result_of::at_c<ov_seq,T::value>::type copy_type;

		// Remove the reference from the type to copy
		typedef typename boost::remove_reference<copy_type>::type copy_rtype;

		meta_copy<copy_rtype> cp(grid_src.template get<T::value>(key_s),grid_dst.template get<T::value>(key_d));
	}
};

/*! \brief Metafunction take T and return a reference
 *
 * Metafunction take T and return a reference
 *
 * \param T type
 *
 */

template<typename T>
struct mem_reference
{
	typedef T& type;
};


/*!
 *
 * \brief This is an N-dimensional grid or an N-dimensional array with memory_traits_lin layout
 *
 * it is basically an N-dimensional Cartesian grid
 *
 *	\tparam dim Dimensionality of the grid
 *	\tparam T type of object the grid store
 *	\tparam Mem memory layout
 *
 * ### Defining the grid size on each dimension
 *
 * \code{.cpp}
 *  size_t sz[3] = {16,16,16};
 * \endcode
 *
 * ### Definition and allocation of a 3D grid on CPU memory
 * \snippet grid_unit_tests.hpp Definition and allocation of a 3D grid on CPU memory
 * ### Access a grid c3 of size sz on each direction
 * \snippet grid_unit_tests.hpp Access a grid c3 of size sz on each direction
 * ### Access an N-dimensional grid with an iterator
 * \snippet grid_unit_tests.hpp Access to an N-dimensional grid with an iterator
 * ### Iterate only on a sub-set of the grid
 * \snippet grid_unit_tests.hpp Sub-grid iterator test usage
 * ### Get the full-object in an N-dimensional grid
 * \snippet grid_unit_tests.hpp Get the object in an N-dimensional grid with an iterator
 *
 */
template<unsigned int dim, typename T, typename Mem = typename memory_traits_lin< typename T::type >::type >
class grid_cpu
{
public:
	// expose the dimansionality as a static const
	static constexpr unsigned int dims = dim;

	//! Access key
	typedef grid_key_dx<dim> access_key;

	//! boost::vector that describe the data type
	typedef typename T::type T_type;

	typedef Mem memory_conf;

private:

	//! Error code
	size_t err_code;


	//! Is the memory initialized
	bool is_mem_init = false;

	//! This is an header that store all information related to the grid
	grid_sm<dim,T> g1;

	//! Memory layout specification + memory chunk pointer
	Mem data_;

	//! The memory allocator is not internally created
	bool isExternal;

	/*! \brief Get 1D vector with the
	 *
	 * Get std::vector with element 0 to dim set to 0
	 *
	 */

	std::vector<size_t> getV()
				{
		std::vector<size_t> tmp;

		for (unsigned int i = 0 ; i < dim ; i++)
		{
			tmp.push_back(0);
		}

		return tmp;
				}

public:

	//! it define that it is a grid
	typedef int yes_i_am_grid;

	//! Definition of the layout
	typedef typename memory_traits_lin<typename T::type>::type memory_lin;

	//! Memory traits
	typedef Mem memory_t;

	//! Object container for T, it is the return type of get_o it return a object type trough
	// you can access all the properties of T
	typedef encapc<dim,T,Mem> container;

	// The object type the grid is storing
	typedef T value_type;

	//! Default constructor
	grid_cpu()
	:g1(getV()),isExternal(false)
	{
	}

	/*! \brief create a grid from another grid
	 *
	 * \tparam S memory type for allocation
	 *
	 * \param g the grid to copy
	 * \param mem memory object (only used for template deduction)
	 *
	 */
	template<typename S> grid_cpu(const grid_cpu & g, S & mem)
	:isExternal(false)
	{
		swap(g.duplicate<S>());
	}

	//! Constructor allocate memory and give them a representation
	grid_cpu(std::vector<size_t> & sz)
	:g1(sz),isExternal(false)
	{
	}

	//! Constructor allocate memory and give them a representation
	grid_cpu(std::vector<size_t> && sz)
	:g1(sz),isExternal(false)
	{
	}

	//! Constructor allocate memory and give them a representation
	grid_cpu(const size_t (& sz)[dim])
	:g1(sz),isExternal(false)
	{
	}

	/*! \brief create a duplicated version of the grid
	 *
	 */

	template<typename S> grid_cpu<dim,T,Mem> duplicate() const
	{
		//! Create a completely new grid with sz

		grid_cpu<dim,T,Mem> grid_new(g1.getSize());

		//! Set the allocator and allocate the memory
		grid_new.template setMemory<S>();

		// We know that, if it is 1D we can safely copy the memory
		if (dim == 1)
		{
			//! 1-D copy (This case is simple we use raw memory copy because is the fastest option)
			grid_new.data_.mem->copy(*data_.mem);
		}
		else
		{
			//! N-D copy

			//! create a source grid iterator
			grid_key_dx_iterator<dim> it(g1);

			while(it.isNext())
			{
				// get the grid key
				grid_key_dx<dim> key = it.get();

				// create a copy element

				copy_cpu_sd<dim,grid_cpu<dim,T,Mem>> cp(key,*this,grid_new);

				// copy each property for each point of the grid

				boost::mpl::for_each_ref< boost::mpl::range_c<int,0,T::max_prop> >(cp);

				++it;
			}
		}

		// copy grid_new to the base

		return grid_new;
	}

	/*! \brief Return the internal grid information
	 *
	 * Return the internal grid information
	 *
	 * \return the internal grid
	 *
	 */

	const grid_sm<dim,T> & getGrid() const
	{
		return g1;
	}

	/*! \brief Create the object that provide memory
	 *
	 * Create the object that provide memory
	 *
	 * \tparam S memory type to allocate
	 *
	 */

	template<typename S> void setMemory()
	{
		S * mem = new S();

		//! Create and set the memory allocator
		data_.setMemory(*mem);

		//! Allocate the memory and create the representation
		if (g1.size() != 0) data_.allocate(g1.size());

		is_mem_init = true;
	}

	/*! \brief Get the object that provide memory
	 *
	 * An external allocator is useful with allocator like PreAllocHeapMem
	 * to have contiguous in memory vectors.
	 *
	 * \tparam S memory type
	 *
	 * \param m external memory allocator
	 *
	 */

	template<typename S> void setMemory(S & m)
	{
		//! Is external
		isExternal = true;

		//! Create and set the memory allocator
		data_.setMemory(m);

		//! Allocate the memory and create the reppresentation
		if (g1.size() != 0) data_.allocate(g1.size());

		is_mem_init = true;
	}

	/*! \brief Return a plain pointer to the internal data
	 *
	 * Return a plain pointer to the internal data
	 *
	 * \return plain data pointer
	 *
	 */

	void * getPointer()
	{
		if (data_.mem_r == NULL)
			return NULL;

		return data_.mem_r->get_pointer();
	}

	/*! \brief Get the reference of the selected element
	 *
	 * \param v1 grid_key that identify the element in the grid
	 *
	 * \return the reference to the element
	 *
	 */
	template <unsigned int p>inline typename type_cpu_prop<p,memory_lin>::type & get(grid_key_d<dim,p> & v1)
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(v1)
#endif
#ifdef SE_CLASS2
		check_valid(&boost::fusion::at_c<p>(data_.mem_r->operator[](g1.LinId(v1))));
#endif
		return boost::fusion::at_c<p>(data_.mem_r->operator[](g1.LinId(v1)));
	}

	/*! \brief Get the const reference of the selected element
	 *
	 * \param v1 grid_key that identify the element in the grid
	 *
	 * \return the const reference to the element
	 *
	 */
	template <unsigned int p>inline const typename type_cpu_prop<p,memory_lin>::type & get(grid_key_d<dim,p> & v1) const
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(v1)
#endif
#ifdef SE_CLASS2
		check_valid(&boost::fusion::at_c<p>(data_.mem_r->operator[](g1.LinId(v1))));
#endif
		return boost::fusion::at_c<p>(data_.mem_r->operator[](g1.LinId(v1)));
	}

	/*! \brief Get the reference of the selected element
	 *
	 * \param v1 grid_key that identify the element in the grid
	 *
	 * \return the reference of the element
	 *
	 */
	template <unsigned int p>inline typename type_cpu_prop<p,memory_lin>::type & get(const grid_key_dx<dim> & v1)
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(v1)
#endif
#ifdef SE_CLASS2
		check_valid(&boost::fusion::at_c<p>(data_.mem_r->operator[](g1.LinId(v1))),sizeof(typename type_cpu_prop<p,memory_lin>::type));
#endif
		return boost::fusion::at_c<p>(data_.mem_r->operator[](g1.LinId(v1)));
	}

	/*! \brief Get the const reference of the selected element
	 *
	 * \param v1 grid_key that identify the element in the grid
	 *
	 * \return the const reference of the element
	 *
	 */
	template <unsigned int p>inline const typename type_cpu_prop<p,memory_lin>::type & get(const grid_key_dx<dim> & v1) const
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(v1)
#endif
#ifdef SE_CLASS2
		check_valid(&boost::fusion::at_c<p>(data_.mem_r->operator[](g1.LinId(v1))),sizeof(typename type_cpu_prop<p,memory_lin>::type));
#endif
		return boost::fusion::at_c<p>(data_.mem_r->operator[](g1.LinId(v1)));
	}

	/*! \brief Get the of the selected element as a boost::fusion::vector
	 *
	 * Get the selected element as a boost::fusion::vector
	 *
	 * \param v1 grid_key that identify the element in the grid
	 *
	 */
	inline encapc<dim,T,Mem> get_o(const grid_key_dx<dim> & v1)
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(v1)
#endif
#ifdef SE_CLASS2
		check_valid(&data_.mem_r->operator[](g1.LinId(v1)),sizeof(T));
#endif
		return encapc<dim,T,Mem>(data_.mem_r->operator[](g1.LinId(v1)));
	}

	/*! \brief Get the of the selected element as a boost::fusion::vector
	 *
	 * Get the selected element as a boost::fusion::vector
	 *
	 * \param v1 grid_key that identify the element in the grid
	 *
	 */
	inline const encapc<dim,T,Mem> get_o(const grid_key_dx<dim> & v1) const
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(v1)
#endif
#ifdef SE_CLASS2
		check_valid(&data_.mem_r->operator[](g1.LinId(v1)),sizeof(T));
#endif
		return encapc<dim,T,Mem>(data_.mem_r->operator[](g1.LinId(v1)));
	}

	/*! \brief Fill the memory with the selected byte
	 *
	 * \param fl byte pattern to fill
	 *
	 */
	void fill(unsigned char fl)
	{
		memset(getPointer(),fl,size() * sizeof(T));
	}

	/*! \brief Resize the space
	 *
	 * Resize the space to a new grid, the element are retained on the new grid,
	 * if the new grid is bigger the new element are now initialized, if is smaller
	 * the data are cropped
	 *
	 * \param sz reference to an array of dimension dim
	 *
	 */
	template<typename S> void resize(const size_t (& sz)[dim])
	{
		//! Create a completely new grid with sz

		grid_cpu<dim,T,Mem> grid_new(sz);

		//! Set the allocator and allocate the memory
		if (isExternal == true)
		{
			grid_new.template setMemory<S>(static_cast<S&>(data_.getMemory()));

			// Create an empty memory allocator for the actual structure

			setMemory<S>();
		}
		else
			grid_new.template setMemory<S>();


		// We know that, if it is 1D we can safely copy the memory
		if (dim == 1)
		{
			//! 1-D copy (This case is simple we use raw memory copy because is the fastest option)
			grid_new.data_.mem->copy(*data_.mem);
		}
		else
		{
			//! N-D copy

			//! create a source grid iterator
			grid_key_dx_iterator<dim> it(g1);

			while(it.isNext())
			{
				// get the grid key
				grid_key_dx<dim> key = it.get();

				// create a copy element

				copy_cpu_sd<dim,grid_cpu<dim,T,Mem>> cp(key,*this,grid_new);

				// copy each property for each point of the grid

				boost::mpl::for_each_ref< boost::mpl::range_c<int,0,T::max_prop> >(cp);

				++it;
			}
		}

		// copy grid_new to the base

		this->swap(grid_new);
	}

	/*! \brief Remove one element valid only on 1D
	 *
	 *
	 */
	void remove(size_t key)
	{
		if (dim != 1)
		{
#ifdef SE_CLASS1
			std::cerr << "Error: " << __FILE__ << " " << __LINE__ << " remove work only on dimension == 1 " << "\n";
#endif
			return;
		}

		// It is safe to do a memory copy

		data_.move(&this->template get<0>());
	}


	/*! \brief It move the allocated object from one grid to another
	 *
	 * It move the allocated object from one grid to another, after this
	 * call the argument grid is no longer valid
	 *
	 * \param grid to move/copy
	 *
	 */

	void swap(grid_cpu<dim,T,Mem> & grid)
	{
		// move the data
		data_.swap(grid.data_);

		// move the grid info
		g1 = grid.g1;

		// exchange the init status
		bool exg = is_mem_init;
		is_mem_init = grid.is_mem_init;
		grid.is_mem_init = exg;

		// exchange the is external status
		exg = isExternal;
		isExternal = grid.isExternal;
		grid.isExternal = exg;
	}

	/*! \brief It move the allocated object from one grid to another
	 *
	 * It move the allocated object from one grid to another, after this
	 * call the argument grid is no longer valid
	 *
	 * \param grid to move/copy
	 *
	 */

	void swap(grid_cpu<dim,T,Mem> && grid)
	{
		swap(grid);
	}

	/*! \brief set an element of the grid
	 *
	 * set an element of the grid
	 *
	 * \param dx is the grid key or the position to set
	 * \param obj value to set
	 *
	 */

	template<typename Memory> inline void set(grid_key_dx<dim> dx, const encapc<1,T,Memory> & obj)
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(dx)
#endif

		// create the object to copy the properties
		copy_cpu_encap<dim,grid_cpu<dim,T,Mem>,Mem> cp(dx,*this,obj);

		// copy each property
		boost::mpl::for_each_ref< boost::mpl::range_c<int,0,T::max_prop> >(cp);
	}

	/*! \brief set an element of the grid
	 *
	 * set an element of the grid
	 *
	 * \param dx is the grid key or the position to set
	 * \param obj value to set
	 *
	 */

	inline void set(grid_key_dx<dim> dx, const T & obj)
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(dx)
#endif
		// create the object to copy the properties
		copy_cpu<dim,grid_cpu<dim,T,Mem>> cp(dx,*this,obj);

		// copy each property
		boost::mpl::for_each_ref< boost::mpl::range_c<int,0,T::max_prop> >(cp);
	}

	/*! \brief Set an element of the grid from another element of another grid
	 *
	 * \param key1 element of the grid to set
	 * \param g source grid
	 * \param key2 element of the source grid to copy
	 *
	 */

	inline void set(grid_key_dx<dim> key1,const grid_cpu<dim,T,Mem> & g, grid_key_dx<dim> key2)
	{
#ifdef SE_CLASS1
		CHECK_INIT()
		GRID_OVERFLOW(key1)
		GRID_OVERFLOW_EXT(g,key2)
#endif

		//create the object to copy the properties
		copy_cpu_sd_k<dim,grid_cpu<dim,T,Mem>> cp(key2,key1,g,*this);

		// copy each property for each point of the grid

		boost::mpl::for_each_ref< boost::mpl::range_c<int,0,T::max_prop> >(cp);

	}

	/*! \brief return the size of the grid
	 *
	 * Return the size of the grid
	 *
	 */

	inline size_t size() const
	{
		return g1.size();
	}

	/*! \brief Return a sub-grid iterator
	 *
	 * Return a sub-grid iterator, to iterate through the grid
	 *
	 * \param start start point
	 * \param stop stop point
	 *
	 */
	inline grid_key_dx_iterator_sub<dim> getSubIterator(grid_key_dx<dim> & start, grid_key_dx<dim> & stop) const
	{
		return g1.getSubIterator(start,stop);
	}

	/*! \brief Return a sub-grid iterator
	 *
	 * Return a sub-grid iterator, to iterate through the grid
	 *
	 * \param m Margin
	 *
	 */

	inline grid_key_dx_iterator_sub<dim> getSubIterator(size_t m)
	{
		return grid_key_dx_iterator_sub<dim>(g1,m);
	}

	/*! \brief Return a grid iterator
	 *
	 * Return a grid iterator, to iterate through the grid
	 *
	 */

	inline grid_key_dx_iterator<dim> getIterator() const
	{
		return grid_key_dx_iterator<dim>(g1);
	}

	/*! \brief Return a grid iterator over all the point with the exception
	 *   of the ghost part
	 *
	 * Return a grid iterator over all the point with the exception of the
	 * ghost part
	 *
	 */

	inline grid_key_dx_iterator_sub<dim> getIterator(const grid_key_dx<dim> & start, const grid_key_dx<dim> & stop) const
	{
		// get the starting point and the end point of the real domain

		return grid_key_dx_iterator_sub<dim>(g1,start,stop);
	}

	/*! \brief Return the last error
	 *
	 */
	size_t getLastError()
	{
		return err_code;
	}

	/*! \brief Return the size of the message needed to pack this object
	 *
	 * TODO They just return 0 for now
	 *
	 * \return The size of the object to pack this object
	 *
	 *
	 */

	size_t packObjectSize()
	{
		return 0;
	}

	/*! \brief It fill the message packet
	 *
	 * TODO They just return 0 doing nothing
	 *
	 * \return The packet size
	 *
	 *
	 */

	size_t packObject(void * mem)
	{
		return 0;
	}
};

/*! \brief this class is a functor for "for_each" algorithm
 *
 * This class is a functor for "for_each" algorithm. For each
 * element of the boost::vector the operator() is called
 *
 * \param T Type of memory allocator
 *
 */

template<typename S>
struct allocate
{
	//! size to allocate
	size_t sz;

	//! constructor it fix the size
	allocate(size_t sz)
	:sz(sz){};

	//! It call the allocate function for each member
	template<typename T>
	void operator()(T& t) const
	{
		//! Create and set the memory allocator
		t.setMemory(*new S());

		//! Allocate the memory and create the reppresentation
		t.allocate(sz);
	}
};

/*! \brief This is an N-dimensional grid or an N-dimensional array with memory_traits_inte layout
 *
 * it is basically an N-dimensional Cartesian grid
 *
 *	\tparam dim Dimensionality of the grid
 *	\tparam T type of object the grid store
 *	\tparam Mem memory layout
 *
 * ### Definition and allocation of a 3D grid on GPU memory
 * \snippet grid_unit_tests.hpp Definition and allocation of a 3D grid on GPU memory
 * ### Access a grid c3 of size sz on each direction
 * \snippet grid_unit_tests.hpp Access a grid c3 of size sz on each direction
 * ### Access to an N-dimensional grid with an iterator
 * \snippet grid_unit_tests.hpp Access to an N-dimensional grid with an iterator
 *
 */
template<unsigned int dim, typename T, typename Mem = typename memory_traits_inte< typename T::type >::type >
class grid_gpu
{
	// Indicate if set memory has been called
	bool is_mem_init = false;

	//! Access the key
	typedef grid_key_dx<dim> access_key;

	//! It store all the information regarding the grid
	grid_sm<dim,void> g1;

	//! This is the interface to allocate,resize ... memory
	//! and give also a representation to the allocated memory
	Mem data_;

public:

	//! it define that it is a grid
	typedef int yes_i_am_grid;

	//! Definition of the layout
	typedef typename memory_traits_inte<typename T::type>::type memory_int;

	//! Memory traits
	typedef Mem memory_conf;

	//! Object container for T, it is the return type of get_o it return a object type trough
	// you can access all the properties of T
	typedef encapg<dim,T,Mem> container;

	// The object type the grid is storing
	typedef T type;

	//! Default constructor
	grid_gpu()
	{
	}

	//! Set the grid dimensions
	void setDimensions(std::vector<size_t> & sz)
	{
		g1.setDimension(sz);
	}

	//! Constructor it initialize the memory and give representation
	grid_gpu(std::vector<size_t> & sz)
	:g1(sz)
	{
	}

	/*! \brief Return the internal grid information
	 *
	 * \return the internal grid information
	 *
	 */

	const grid_sm<dim,void> & getGrid() const
	{
		return g1;
	}

	/*! \brief Create the object that provide memory
	 *
	 * \tparam S memory object type
	 *
	 */

	template<typename S> void setMemory()
	{
		//! Create an allocate object
		allocate<S> all(g1.size());

		//! for each element in the vector allocate the buffer
		boost::fusion::for_each(data_,all);

		is_mem_init = true;
	}

	template <unsigned int p>inline typename type_gpu_prop<p,memory_int>::type::reference get(grid_key_d<dim,p> & v1)
	{
		return boost::fusion::at_c<p>(data_).mem_r->operator[](g1.LinId(v1));
	}

	template <unsigned int p>inline typename type_gpu_prop<p,memory_int>::type::reference get(grid_key_dx<dim> & v1)
	{
		return boost::fusion::at_c<p>(data_).mem_r->operator[](g1.LinId(v1));
	}

	/*! \brief Get the of the selected element as a boost::fusion::vector
	 *
	 * Get the selected element as a boost::fusion::vector
	 *
	 * \param v1 grid_key that identify the element in the grid
	 *
	 */
	inline encapg<dim,T,Mem> get_o(grid_key_dx<dim> & v1)
	{
		return encapg<dim,T,Mem>(data_,g1.LinId(v1));
	}

	/*! \brief Get the of the selected element as a boost::fusion::vector
	 *
	 * Get the selected element as a boost::fusion::vector
	 *
	 * \param v1 grid_key that identify the element in the grid
	 *
	 */
	inline const encapg<dim,T,Mem> get_o(grid_key_dx<dim> & v1) const
	{
		return encapg<dim,T,Mem>(data_,g1.LinId(v1));
	}

	inline size_t size()
	{
		return g1.size();
	}

	//! this function set the memory interface if required
	//! this operation is required when we define a void memory
	//! allocator
	void set_memory(memory & mem)
	{
		data_.mem.set_memory(mem);
	}

	/*! \brief Return a grid iterator
	 *
	 * Return a grid iterator, to iterate through the grid
	 *
	 */

	inline grid_key_dx_iterator<dim> getIterator()
	{
		return grid_key_dx_iterator<dim>(g1);
	}


	/*! \brief Return a sub-grid iterator
	 *
	 * Return a sub-grid iterator, to iterate through the grid
	 *
	 */

	inline grid_key_dx_iterator_sub<dim> getSubIterator(grid_key_dx<dim> & start, grid_key_dx<dim> & stop)
	{
		return grid_key_dx_iterator_sub<dim>(g1,start,stop);
	}

	/*! \brief Swap the memory of another grid
	 *
	 * Swap the memory of another grid
	 *
	 * \param obj Memory to swap with
	 *
	 */
	void swap(grid_gpu<dim,T,Mem> & obj)
	{
		g1.swap(obj.g1);
		data_.swap(obj.data_);
	}
};

/*! device selector struct
 *
 * device selector struct, it return the correct data type for each device
 *
 */

template<unsigned int dim, typename T>
struct device_g
{
	//! cpu
	typedef grid_cpu<dim,T> cpu;
	//! gpu
	typedef grid_gpu<dim,T> gpu;
};

#endif


