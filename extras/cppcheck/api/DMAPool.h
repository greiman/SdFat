/*
  This file is part of the Arduino_AdvancedAnalog library.
  Copyright (c) 2023-2024 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __DMA_POOL_H__
#define __DMA_POOL_H__

#include <atomic>
#include <memory>

namespace arduino {

#if defined(__DCACHE_PRESENT)
#define __CACHE_LINE_SIZE__ __SCB_DCACHE_LINE_SIZE
#elif defined(__cpp_lib_hardware_interference_size)
#define __CACHE_LINE_SIZE__ std::hardware_constructive_interference_size
#else // No cache.
#define __CACHE_LINE_SIZE__ alignof(int)
#endif

// Single-producer, single-consumer, lock-free bounded Queue.
template <class T> class SPSCQueue {
    private:
        size_t capacity;
        std::atomic<size_t> head;
        std::atomic<size_t> tail;
        std::unique_ptr<T[]> buff;

    public:
        SPSCQueue(size_t size=0):
         capacity(0), tail(0), head(0), buff(nullptr) {
            if (size) {
                T *mem = new T[size + 1];
                if (mem) {
                    buff.reset(mem);
                    capacity = size + 1;
                }
            }
        }

        void reset() {
            tail = head = 0;
        }

        size_t empty() {
            return tail == head;
        }

        operator bool() const {
            return buff.get() != nullptr;
        }

        bool push(T data) {
            size_t curr = head.load(std::memory_order_relaxed);
            size_t next = (curr + 1) % capacity;
            if (!buff || (next == tail.load(std::memory_order_acquire))) {
                return false;
            }
            buff[curr] = data;
            head.store(next, std::memory_order_release);
            return true;
        }

        T pop(bool peek=false) {
            size_t curr = tail.load(std::memory_order_relaxed);
            if (!buff || (curr == head.load(std::memory_order_acquire))) {
                return nullptr;
            }
            T data = buff[curr];
            if (!peek) {
                size_t next = (curr + 1) % capacity;
                tail.store(next, std::memory_order_release);
            }
            return data;
        }
};

enum {
    DMA_BUFFER_READ     = (1 << 0),
    DMA_BUFFER_WRITE    = (1 << 1),
    DMA_BUFFER_DISCONT  = (1 << 2),
    DMA_BUFFER_INTRLVD  = (1 << 3),
} DMABufferFlags;

// Forward declaration of DMAPool class.
template <class, size_t> class DMAPool;

template <class T, size_t A=__CACHE_LINE_SIZE__> class DMABuffer {
    private:
        DMAPool<T, A> *pool;
        size_t n_samples;
        size_t n_channels;
        T *ptr;
        uint32_t ts;
        uint32_t flags;

    public:
        DMABuffer(DMAPool<T, A> *pool=nullptr, size_t samples=0, size_t channels=0, T *mem=nullptr):
            pool(pool), n_samples(samples), n_channels(channels), ptr(mem), ts(0), flags(0) {
        }

        T *data() {
            return ptr;
        }

        size_t size() {
            return n_samples * n_channels;
        }

        size_t bytes() {
            return n_samples * n_channels * sizeof(T);
        }

        void flush() {
            #if __DCACHE_PRESENT
            if (ptr) {
                SCB_CleanDCache_by_Addr(data(), bytes());
            }
            #endif
        }

        void invalidate() {
            #if __DCACHE_PRESENT
            if (ptr) {
                SCB_InvalidateDCache_by_Addr(data(), bytes());
            }
            #endif
        }

        uint32_t timestamp() {
            return ts;
        }

        void timestamp(uint32_t ts) {
            this->ts = ts;
        }

        uint32_t channels() {
            return n_channels;
        }

        void release() {
            if (pool && ptr) {
                pool->free(this, flags);
            }
        }

        void set_flags(uint32_t f) {
            flags |= f;
        }

        bool get_flags(uint32_t f=0xFFFFFFFFU) {
            return flags & f;
        }

        void clr_flags(uint32_t f=0xFFFFFFFFU) {
            flags &= (~f);
        }

        T& operator[](size_t i) {
            assert(ptr && i < size());
            return ptr[i];
        }

        const T& operator[](size_t i) const {
            assert(ptr && i < size());
            return ptr[i];
        }

        operator bool() const {
            return (ptr != nullptr);
        }
};

template <class T, size_t A=__CACHE_LINE_SIZE__> class DMAPool {
    private:
        uint8_t *mem;
        bool managed;
        SPSCQueue<DMABuffer<T>*> wqueue;
        SPSCQueue<DMABuffer<T>*> rqueue;

        // Allocates dynamic aligned memory.
        // Note this memory must be free'd with aligned_free.
        static void *aligned_malloc(size_t size) {
            void **ptr, *stashed;
            size_t offset = A - 1 + sizeof(void *);
            if ((A % 2) || !((stashed = ::malloc(size + offset)))) {
                return nullptr;
            }
            ptr = (void **) (((uintptr_t) stashed + offset) & ~(A - 1));
            ptr[-1] = stashed;
            return ptr;
        }

        // Frees dynamic aligned memory allocated with aligned_malloc.
        static void aligned_free(void *ptr) {
            if (ptr != nullptr) {
                ::free(((void **) ptr)[-1]);
            }
        }

    public:
        DMAPool(size_t n_samples, size_t n_channels, size_t n_buffers, void *mem_in=nullptr):
            mem((uint8_t *) mem_in), managed(mem_in==nullptr), wqueue(n_buffers), rqueue(n_buffers) {
            // Round up to the next multiple of the alignment.
            size_t bufsize = (((n_samples * n_channels * sizeof(T)) + (A-1)) & ~(A-1));
            if (bufsize && rqueue && wqueue) {
                if (mem == nullptr) {
                    // Allocate an aligned memory block for the DMA buffers' memory.
                    mem = (uint8_t *) aligned_malloc(n_buffers * bufsize);
                    if (!mem) {
                        // Failed to allocate memory.
                        return;
                    }
                }
                // Allocate the DMA buffers, initialize them using aligned
                // pointers from the pool, and add them to the write queue.
                for (size_t i=0; i<n_buffers; i++) {
                    DMABuffer<T> *buf =  new DMABuffer<T>(
                        this, n_samples, n_channels, (T *) &mem[i * bufsize]
                    );
                    if (buf == nullptr) {
                        break;
                    }
                    wqueue.push(buf);
                }
            }
        }

        ~DMAPool() {
            while (readable()) {
                delete alloc(DMA_BUFFER_READ);
            }

            while (writable()) {
                delete alloc(DMA_BUFFER_WRITE);
            }

            if (mem && managed) {
                aligned_free(mem);
            }
        }

        bool writable() {
            return !(wqueue.empty());
        }

        bool readable() {
            return !(rqueue.empty());
        }

        void flush() {
            while (readable()) {
                DMABuffer<T> *buf = alloc(DMA_BUFFER_READ);
                if (buf) {
                    buf->release();
                }
            }
        }

        DMABuffer<T> *alloc(uint32_t flags) {
            DMABuffer<T> *buf = nullptr;
            if (flags & DMA_BUFFER_READ) {
                // Get a DMA buffer from the read/ready queue.
                buf = rqueue.pop();
            } else {
                // Get a DMA buffer from the write/free queue.
                buf = wqueue.pop();
            }
            if (buf) {
                buf->clr_flags(DMA_BUFFER_READ | DMA_BUFFER_WRITE);
                buf->set_flags(flags);
            }
            return buf;
        }

        void free(DMABuffer<T> *buf, uint32_t flags=0) {
            if (buf == nullptr) {
                return;
            }
            if (flags == 0) {
                flags = buf->get_flags();
            }
            if (flags & DMA_BUFFER_READ) {
                // Return the DMA buffer to the write/free queue.
                buf->clr_flags();
                wqueue.push(buf);
            } else {
                // Return the DMA buffer to the read/ready queue.
                rqueue.push(buf);
            }
        }

};

} // namespace arduino

using arduino::DMAPool;
using arduino::DMABuffer;
using arduino::SPSCQueue;
#endif //__DMA_POOL_H__
