#include <malloc.h>
#include <cstdio>
#include <cstdint>

struct BlockHeader
{
    BlockHeader* next;
    uint32_t size;
    bool allocated;

    void linkNext(BlockHeader* block)
    {
        next = block;
    }
};

static void* heapBase = nullptr;
static BlockHeader* nextFree = nullptr;

template<typename T = uint8_t>
static T* getBlockAddress(BlockHeader* block)
{
    return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(block) + sizeof(*block));
}

static BlockHeader* getHeader(void* block)
{
    return reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(block) - sizeof(BlockHeader));
}

static BlockHeader* findNextFreeBlock(uint32_t size)
{
    auto block = reinterpret_cast<BlockHeader*>(heapBase);
    
    while (block) {
        if (!block->allocated && (block->size >= size || block->size == 0)) {
            return block;
        }

        block = block->next;
    }

    // idk what to do here ??? just crash
    return nullptr;
}

void* allocateBlock(uint32_t size)
{
    if (!nextFree) {
        nextFree = reinterpret_cast<BlockHeader*>(heapBase);
    }

    // make sure the size is a multiple of 16
    size = ((size - 1) & ~0xF) + 0x10;

    BlockHeader* block = nullptr;
    auto candidateBlock = findNextFreeBlock(size);

    if (nextFree->size >= size) {
        block = nextFree;
    } else if (candidateBlock) {
        block = candidateBlock;
    } else if (nextFree->size == 0) {
        block = nextFree;
        block->size = size;
    } 

    nextFree = reinterpret_cast<BlockHeader*>(getBlockAddress<uint8_t>(block) + size);
    nextFree->linkNext(nullptr);
    nextFree->size = 0;
    nextFree->allocated = false;

    block->linkNext(nextFree);
    block->allocated = true;

    return getBlockAddress(block);
}

void freeBlock(void* ptr)
{
    auto block = getHeader(ptr);
    block->allocated = false;
    nextFree = block;
}

int main(int argc, char* argv[])
{
    auto heapSize = 2048 * 1024;
    heapBase = malloc(heapSize);

    printf("heap size: %08x\n", heapSize);

    auto a = allocateBlock(10);
    printf("a(10) = %p\n", a);

    auto b = allocateBlock(15);
    printf("b(15) = %p\n", b);

    auto c = allocateBlock(15);
    printf("c(15) = %p\n", c);

    freeBlock(b);
    b = allocateBlock(40);
    printf("b(4) = %p\n", b);

    auto d = allocateBlock(10);
    printf("d(10) = %p\n", d);

    return 0;
}