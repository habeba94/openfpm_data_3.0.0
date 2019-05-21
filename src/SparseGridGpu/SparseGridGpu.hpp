#ifndef SPARSE_GRID_GPU_HPP_
#define SPARSE_GRID_GPU_HPP_

#include "Vector/map_vector_sparse.hpp"
#include "SparseGridGpu_ker.cuh"
#include "SparseGridGpu_kernels.cuh"
#include "DataBlock.cuh"
#include <set>

template<typename BlockT, typename T>
struct AggregateAppend
{
};

template<typename BlockT, typename ... list>
struct AggregateAppend<BlockT, aggregate<list ...>>
{
    typedef aggregate<list..., BlockT> type;
};

//template<unsigned int i>
//struct plus_one
//{
//    typedef boost::mpl::int_<i+1> type;
//};
//
//template<typename T>
//struct prop_adder
//{
//};
//
//template<unsigned int ... props>
//struct vector_num
//{};
//
//template<unsigned int ... list>
//struct prop_adder<vector_num<list ...>>
//{
//    typedef vector_num< plus_one<list>::type::value ... > type;
//
//    template<typename T>
//    static void deviceToHost(T & obj)
//    {
//        obj.template deviceToHost<0, list ... >();
//    }
//};



template<typename AggregateT, unsigned int p>
using BlockTypeOf = typename std::remove_reference<typename boost::fusion::result_of::at_c<typename AggregateT::type, p>::type>::type;

template<typename AggregateT, unsigned int p>
using ScalarTypeOf = typename std::remove_reference<typename boost::fusion::result_of::at_c<typename AggregateT::type, p>::type>::type::scalarType;

template<typename AggregateBlockT, typename indexT=int, template<typename> class layout_base=memory_traits_inte>
class SparseGridGpu
{
private:
    typedef BlockTypeOf<AggregateBlockT, 0> BlockT0;
    typedef typename AggregateAppend<DataBlock<unsigned char, BlockT0::size>, AggregateBlockT>::type AggregateInternalT;
    static const unsigned int pMask = AggregateInternalT::max_prop_real - 1;
    openfpm::vector_sparse_gpu<AggregateInternalT> blockMap;

public:
    typedef AggregateBlockT AggregateType;

public:
    SparseGridGpu() = default;

    template<unsigned int p>
    auto get(unsigned int linId) const -> ScalarTypeOf<AggregateBlockT, p>;

    template<unsigned int p>
    auto insert(unsigned int linId) -> ScalarTypeOf<AggregateBlockT, p> &;

    SparseGridGpu_ker<AggregateInternalT, indexT, layout_base> toKernel()
    {
        SparseGridGpu_ker<AggregateInternalT, indexT, layout_base> toKer(blockMap.toKernel());
        return toKer;
    }

    template<unsigned int ... prp>
    void deviceToHost();

    void setGPUInsertBuffer(int nBlock, int nSlot);

    template<unsigned int p>
//    void initializeGPUInsertBuffer(unsigned int gridSize, unsigned int blockSize);
    void initializeGPUInsertBuffer();

    template<typename ... v_reduce>
    void flush(mgpu::ofp_context_t &context, flush_type opt = FLUSH_ON_HOST, int i = 0);

    template<unsigned int p>
    void setBackground(ScalarTypeOf<AggregateBlockT, p> backgroundValue);
};

template<typename AggregateBlockT, typename indexT, template<typename> class layout_base>
template<unsigned int p>
auto
SparseGridGpu<AggregateBlockT, indexT, layout_base>::get(unsigned int linId) const -> ScalarTypeOf<AggregateBlockT, p>
{
    typedef BlockTypeOf<AggregateBlockT, p> BlockT;
    unsigned int blockId = linId / BlockT::size;
    unsigned int offset = linId % BlockT::size;
    auto &block = blockMap.template get<p>(blockId);
    return block[offset];
}

template<typename AggregateBlockT, typename indexT, template<typename> class layout_base>
template<unsigned int p>
auto
SparseGridGpu<AggregateBlockT, indexT, layout_base>::insert(unsigned int linId) -> ScalarTypeOf<AggregateBlockT, p> &
{
    typedef BlockTypeOf<AggregateBlockT, p> BlockT;
    unsigned int blockId = linId / BlockT::size;
    unsigned int offset = linId % BlockT::size;
    auto &block = blockMap.template insert<p>(blockId);
    block.setElement(offset);
    return block[offset];
}

//template<typename AggregateBlockT, typename indexT, template<typename> class layout_base>
//SparseGridGpu_ker<AggregateBlockT, indexT, layout_base> SparseGridGpu<AggregateBlockT, indexT, layout_base>::toKernel()
//{
//    SparseGridGpu_ker<AggregateBlockT, indexT, layout_base> toKer(blockMap.toKernel());
//    return toKer;
//}

template<typename AggregateBlockT, typename indexT, template<typename> class layout_base>
template<unsigned int ... prp>
void SparseGridGpu<AggregateBlockT, indexT, layout_base>::deviceToHost()
{
    blockMap.template deviceToHost<prp..., pMask>();
//    typedef typename prop_adder<vector_num<prp ...>>::type plusOneT;
//    prop_adder<plusOneT>::deviceToHost(*this);
}

template<typename AggregateBlockT, typename indexT, template<typename> class layout_base>
void SparseGridGpu<AggregateBlockT, indexT, layout_base>::setGPUInsertBuffer(int nBlock, int nSlot)
{
    // Prealloc the insert buffer on the underlying sparse vector
    blockMap.setGPUInsertBuffer(nBlock, nSlot);
}

template<typename AggregateBlockT, typename indexT, template<typename> class layout_base>
template<unsigned int p>
//void SparseGridGpu<AggregateBlockT, indexT, layout_base>::initializeGPUInsertBuffer(unsigned int gridSize, unsigned int blockSize)
void SparseGridGpu<AggregateBlockT, indexT, layout_base>::initializeGPUInsertBuffer()
{
    // Initialize the blocks to background
    auto & insertBuffer = blockMap.getGPUInsertBuffer();
    typedef BlockTypeOf<AggregateBlockT, 0> BlockType; // Here assuming that all block types in the aggregate have the same size!
//    SparseGridGpuKernels::initializeInsertBuffer<p> <<< gridSize * BlockType::size, blockSize >>>(
    SparseGridGpuKernels::initializeInsertBuffer<p, pMask> <<< insertBuffer.size()/2, 2*BlockType::size >>>(
            insertBuffer.toKernel(),
            blockMap.template getBackground<p>()[0]
                    );
}

template<typename AggregateBlockT, typename indexT, template<typename> class layout_base>
template<typename ... v_reduce>
void SparseGridGpu<AggregateBlockT, indexT, layout_base>::flush(mgpu::ofp_context_t &context, flush_type opt, int i)
{
    blockMap.template flush<v_reduce ..., smax_<pMask>>(context, opt, i);
    //todo Implement the right bitwise or reduction for the mask
}

template<typename AggregateBlockT, typename indexT, template<typename> class layout_base>
template<unsigned int p>
void SparseGridGpu<AggregateBlockT, indexT, layout_base>::setBackground(
        ScalarTypeOf<AggregateBlockT, p> backgroundValue)
{
    // NOTE: Here we assume user only passes Blocks and not scalars in the templated aggregate type
    typedef BlockTypeOf<AggregateBlockT, p> BlockT;
    blockMap.template getBackground<pMask>() = 0;
    for (unsigned int i = 0; i < BlockT::size; ++i)
    {
        blockMap.template getBackground<p>()[i] = backgroundValue;
    }
}

#endif /* SPARSE_GRID_GPU_HPP_ */