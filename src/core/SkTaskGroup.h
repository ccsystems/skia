/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup_DEFINED
#define SkTaskGroup_DEFINED

#include <functional>

#include "SkTypes.h"
#include "SkAtomics.h"
#include "SkTemplates.h"

struct SkRunnable;

class SkTaskGroup : SkNoncopyable {
public:
    // Create one of these in main() to enable SkTaskGroups globally.
    struct Enabler : SkNoncopyable {
        explicit Enabler(int threads = -1);  // Default is system-reported core count.
        ~Enabler();
    };

    SkTaskGroup();
    ~SkTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.  It will likely run on another thread.
    // Neither add() method takes owership of any of its parameters.
    void add(SkRunnable*);

    void add(std::function<void(void)> fn);

    // Add a batch of N tasks, all calling fn with different arguments.
    void batch(std::function<void(int)> fn, int N);

    // Block until all Tasks previously add()ed to this SkTaskGroup have run.
    // You may safely reuse this SkTaskGroup after wait() returns.
    void wait();

private:
    SkAtomic<int32_t> fPending;
};

// Returns best estimate of number of CPU cores available to use.
int sk_num_cores();

int sk_parallel_for_thread_count();

// Call f(i) for i in [0, end).
template <typename Func>
void sk_parallel_for(int end, const Func& f) {
    if (end <= 0) { return; }

    struct Chunk {
        const Func* f;
        int start, end;
    };

    // TODO(mtklein): this chunking strategy could probably use some tuning.
    int max_chunks  = sk_num_cores() * 2,
        stride      = (end + max_chunks - 1 ) / max_chunks,
        nchunks     = (end + stride - 1 ) / stride;
    SkASSERT(nchunks <= max_chunks);

#if defined(GOOGLE3)
    // Stack frame size is limited in GOOGLE3.
    SkAutoSTMalloc<512, Chunk> chunks(nchunks);
#else
    // With the chunking strategy above this won't malloc until we have a machine with >512 cores.
    SkAutoSTMalloc<1024, Chunk> chunks(nchunks);
#endif

    for (int i = 0; i < nchunks; i++) {
        Chunk& c = chunks[i];
        c.f      = &f;
        c.start  = i * stride;
        c.end    = SkTMin(c.start + stride, end);
        SkASSERT(c.start < c.end);  // Nothing will break if start >= end, but it's a wasted chunk.
    }

    Chunk* chunkBase = chunks.get();
    auto run_chunk = [chunkBase](int i) {
        Chunk& c = chunkBase[i];
        for (int i = c.start; i < c.end; i++) {
            (*c.f)(i);
        }
    };
    SkTaskGroup().batch(run_chunk, nchunks);
}

#endif//SkTaskGroup_DEFINED
