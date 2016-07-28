#include "frame_pool.h"

#include <algorithm>
#include <utility>

#include <assert.h>
#include <log.h>
#include <vm/vm.h>

frame_pool::frame_pool(const multiboot_info &multiboot, void *start_mod_start,
                       void *start_mod_end, void *kern_mod_start,
                       void *kern_mod_end)
    : maxmem(nullptr),
      total_512g_frames(0),
      num_1g_frames(0),
      num_2m_frames(0),
      num_4k_frames(0),
      frames_1g_bits(),
      counts_1g(),
      size_1g_bits(),
      frames_2m_bits(),
      counts_2m(),
      size_2m_bits(),
      freelist_4k() {
  fixedsize_vector<pair<void *, void *>, 2> reserved;
  assert(start_mod_end <= kern_mod_start);
  if (start_mod_end == kern_mod_start) {
    reserved.emplace_back(start_mod_start, kern_mod_end);
    add_with_reserved(multiboot, reserved);
    return;
  }
  reserved.emplace_back(start_mod_start, start_mod_end);
  reserved.emplace_back(kern_mod_start, kern_mod_end);
  add_with_reserved(multiboot, reserved);
}

void *frame_pool::get_maxmem() const { return maxmem; }

void frame_pool::set_index_2m(size_t index) {
  size_t arr_index = index / 64;
  frames_2m_bits[arr_index] |= 1 << (index % 64);
  ++counts_2m[arr_index];
  ++num_2m_frames;
}

void frame_pool::set_index_1g(size_t index) {
  size_t arr_index = index / 64;
  frames_1g_bits[arr_index] |= 1 << (index % 64);
  ++counts_1g[arr_index];
  ++num_1g_frames;
}

void frame_pool::add_with_reserved(
    const multiboot_info &multiboot,
    const fixedsize_vector<pair<void *, void *>, 2> &reserved) {
  fixedsize_vector<multiboot_mmap_entry, 32> v;
  multiboot.get_memory_map(v);
  for (auto i = 0u; i < v.size(); ++i) {
    multiboot_mmap_entry &e = v[i];
    char *start = reinterpret_cast<char *>(e.addr);
    char *end = reinterpret_cast<char *>(start + e.len);
    for (auto j = 0u; j < reserved.size(); ++j) {
      const auto &p = reserved[j];
      char *res_start = static_cast<char *>(p.first);
      char *res_end = static_cast<char *>(p.second);
      assert(res_start != res_end);
      if (start >= res_start && start < res_end) {
        if (end <= res_end) {
          continue;
        }
        e.addr = reinterpret_cast<multiboot_uint64_t>(res_end);
        e.len = end - res_end;
      } else if (end <= res_end && end > res_start) {
        e.addr = reinterpret_cast<multiboot_uint64_t>(start);
        e.len = res_start - start;
      } else if (start < res_start && end > res_end) {
        e.addr = reinterpret_cast<multiboot_uint64_t>(start);
        e.len = res_start - start;
        multiboot_mmap_entry second_split;
        second_split.addr = reinterpret_cast<multiboot_uint64_t>(res_end);
        second_split.len = end - res_end;
        update_maxmem(second_split);
        v.push_back(std::move(second_split));
      }
      update_maxmem(e);
    }
  }
  allocate_bitmaps(v, reserved);
  for (auto i = 0u; i < v.size(); ++i) {
    add_memory_chunk(v[i]);
  }
}

void frame_pool::update_maxmem(const multiboot_mmap_entry &chunk) {
  char *max = reinterpret_cast<char *>(chunk.addr) + chunk.len;
  int num_512g_frames =
      reinterpret_cast<uintptr_t>(vm::align_up(max, gib_512)) / gib_512;
  total_512g_frames = std::max(total_512g_frames, num_512g_frames);
  char *max_4k = vm::align_down_4k(max);
  maxmem = std::max(maxmem, static_cast<void *>(max_4k - 1));
}

void frame_pool::add_memory_chunk(const multiboot_mmap_entry &chunk) {
  char *addr = reinterpret_cast<char *>(chunk.addr);
  char *max = addr + chunk.len;
  char *aligned_4k = vm::align_up_4k(addr);
  char *max_4k = vm::align_down_4k(max);
  char *aligned_2m = vm::align_up_2m(addr);
  char *max_2m = vm::align_down_2m(max);
  char *aligned_1g = vm::align_up_1g(addr);
  char *max_1g = vm::align_down_1g(max);
  for (; aligned_4k < max_4k && aligned_4k < aligned_2m;
       aligned_4k += vm::pgsz_4k) {
    add_frame_4k(static_cast<void *>(aligned_4k));
  }
  for (; aligned_2m < max_2m && aligned_2m < aligned_1g;
       aligned_2m += vm::pgsz_2m) {
    add_frame_2m(static_cast<void *>(aligned_2m));
  }
  aligned_4k = std::max(aligned_2m, max_2m);
  for (; aligned_1g < max_1g; aligned_1g += vm::pgsz_1g) {
    add_frame_1g(static_cast<void *>(aligned_1g));
    aligned_2m = aligned_1g;
  }
  aligned_2m = std::max(aligned_1g, max_1g);
  for (; aligned_2m < max_2m; aligned_2m += vm::pgsz_2m) {
    add_frame_2m(static_cast<void *>(aligned_2m));
  }
  for (; aligned_4k < max_4k; aligned_4k += vm::pgsz_4k) {
    add_frame_4k(static_cast<void *>(aligned_4k));
  }
}

void frame_pool::add_frame_4k(void *frame) {
  freelist_4k.push_back(frame);
  ++num_4k_frames;
}

void frame_pool::add_frame_2m(void *frame) {
  set_index_2m(reinterpret_cast<size_t>(frame) / vm::pgsz_2m);
}

void frame_pool::add_frame_1g(void *frame) {
  set_index_1g(reinterpret_cast<size_t>(frame) / vm::pgsz_1g);
}

size_t frame_pool::calc_bitmap_size_aligned(
    std::array<size_t, 3> &offsets) const {
  assert(total_512g_frames != 0);
  auto counter = 0;
  counter += sizeof_2m_bitmap();
  offsets[0] = counter;
  counter += sizeof_2m_counts();
  offsets[1] = counter;
  counter += sizeof_1g_bitmap();
  offsets[2] = counter;
  return counter + sizeof_1g_counts();
}

size_t frame_pool::sizeof_2m_bitmap() const {
  return total_512g_frames * 512 * 512 / 8;
}

size_t frame_pool::sizeof_2m_counts() const {
  return total_512g_frames * 512 * 512 / 64;
}

size_t frame_pool::sizeof_1g_bitmap() const {
  return total_512g_frames * 512 / 8;
}

size_t frame_pool::sizeof_1g_counts() const {
  return total_512g_frames * 512 / 64;
}
