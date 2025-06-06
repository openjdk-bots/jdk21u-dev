/*
 * Copyright (c) 2015, 2023, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_GC_G1_G1ALLOCATOR_INLINE_HPP
#define SHARE_GC_G1_G1ALLOCATOR_INLINE_HPP

#include "gc/g1/g1Allocator.hpp"

#include "gc/g1/g1AllocRegion.inline.hpp"
#include "gc/shared/plab.inline.hpp"
#include "memory/universe.hpp"

inline uint G1Allocator::current_node_index() const {
  return _numa->index_of_current_thread();
}

inline MutatorAllocRegion* G1Allocator::mutator_alloc_region(uint node_index) {
  assert(node_index < _num_alloc_regions, "Invalid index: %u", node_index);
  return &_mutator_alloc_regions[node_index];
}

inline SurvivorGCAllocRegion* G1Allocator::survivor_gc_alloc_region(uint node_index) {
  assert(node_index < _num_alloc_regions, "Invalid index: %u", node_index);
  return &_survivor_gc_alloc_regions[node_index];
}

inline OldGCAllocRegion* G1Allocator::old_gc_alloc_region() {
  return &_old_gc_alloc_region;
}

inline HeapWord* G1Allocator::attempt_allocation(uint node_index,
                                                 size_t min_word_size,
                                                 size_t desired_word_size,
                                                 size_t* actual_word_size) {
  HeapWord* result = mutator_alloc_region(node_index)->attempt_retained_allocation(min_word_size, desired_word_size, actual_word_size);
  if (result != nullptr) {
    return result;
  }

  return mutator_alloc_region(node_index)->attempt_allocation(min_word_size, desired_word_size, actual_word_size);
}

inline HeapWord* G1Allocator::attempt_allocation_locked(uint node_index, size_t word_size) {
  HeapWord* result = mutator_alloc_region(node_index)->attempt_allocation_locked(word_size);

  assert(result != nullptr || mutator_alloc_region(node_index)->get() == nullptr,
         "Must not have a mutator alloc region if there is no memory, but is " PTR_FORMAT, p2i(mutator_alloc_region(node_index)->get()));
  return result;
}

inline HeapWord* G1Allocator::attempt_allocation_force(uint node_index, size_t word_size) {
  return mutator_alloc_region(node_index)->attempt_allocation_force(word_size);
}

inline PLAB* G1PLABAllocator::alloc_buffer(G1HeapRegionAttr dest, uint node_index) const {
  assert(dest.is_valid(),
         "Allocation buffer index out of bounds: %s", dest.get_type_str());
  assert(_dest_data[dest.type()]._alloc_buffer != nullptr,
         "Allocation buffer is null: %s", dest.get_type_str());
  return alloc_buffer(dest.type(), node_index);
}

inline PLAB* G1PLABAllocator::alloc_buffer(region_type_t dest, uint node_index) const {
  assert(dest < G1HeapRegionAttr::Num,
         "Allocation buffer index out of bounds: %u", dest);

  if (dest == G1HeapRegionAttr::Young) {
    assert(node_index < alloc_buffers_length(dest),
           "Allocation buffer index out of bounds: %u, %u", dest, node_index);
    return _dest_data[dest]._alloc_buffer[node_index];
  } else {
    return _dest_data[dest]._alloc_buffer[0];
  }
}

inline uint G1PLABAllocator::alloc_buffers_length(region_type_t dest) const {
  if (dest == G1HeapRegionAttr::Young) {
    return _allocator->num_nodes();
  } else {
    return 1;
  }
}

inline HeapWord* G1PLABAllocator::plab_allocate(G1HeapRegionAttr dest,
                                                size_t word_sz,
                                                uint node_index) {
  PLAB* buffer = alloc_buffer(dest, node_index);
  return buffer->allocate(word_sz);
}

inline HeapWord* G1PLABAllocator::allocate(G1HeapRegionAttr dest,
                                           size_t word_sz,
                                           bool* refill_failed,
                                           uint node_index) {
  HeapWord* const obj = plab_allocate(dest, word_sz, node_index);
  if (obj != nullptr) {
    return obj;
  }
  return allocate_direct_or_new_plab(dest, word_sz, refill_failed, node_index);
}

#endif // SHARE_GC_G1_G1ALLOCATOR_INLINE_HPP
