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
      frames_2m_bits(),
      counts_1g(),
      counts_2m() {
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

void frame_pool::add_with_reserved(
    const multiboot_info &multiboot,
    const fixedsize_vector<pair<void *, void *>, 2> &reserved) {
  fixedsize_vector<multiboot_mmap_entry, 32> v;
  multiboot.get_memory_map(v);
  for (auto i = 0u; i < v.size(); ++i) {
    klog_debug("i: %x/%x", i, v.size() - 1);
    const multiboot_mmap_entry &e = v[i];
    char *start = reinterpret_cast<char *>(e.addr);
    char *end = reinterpret_cast<char *>(start + e.len);
    klog_debug("Original start: %x", start);
    klog_debug("Original len: %x", e.len);
    multiboot_mmap_entry chunk;
    for (auto j = 0u; j < reserved.size(); ++j) {
      const auto &p = reserved[j];
      char *res_start = static_cast<char *>(p.first);
      char *res_end = static_cast<char *>(p.second);
      klog_debug("res start: %x", res_start);
      klog_debug("res end: %x", res_end);
      if (start >= res_start && start < res_end) {
        if (end <= res_end) {
          continue;
        }
        chunk.addr = reinterpret_cast<multiboot_uint64_t>(res_end);
        chunk.len = end - res_end;
        add_memory_chunk(chunk);
        continue;
      }
      if (end <= res_end && end > res_start) {
        chunk.addr = reinterpret_cast<multiboot_uint64_t>(start);
        chunk.len = res_start - start;
        add_memory_chunk(chunk);
        continue;
      }
      if (start < res_start && end > res_end) {
        chunk.addr = reinterpret_cast<multiboot_uint64_t>(start);
        chunk.len = res_start - start;
        add_memory_chunk(chunk);
        chunk.addr = reinterpret_cast<multiboot_uint64_t>(res_end);
        chunk.len = end - res_end;
        add_memory_chunk(chunk);
        continue;
      }
      add_memory_chunk(e);
    }
  }
}

void frame_pool::add_memory_chunk(const multiboot_mmap_entry &chunk) {
  char *addr = reinterpret_cast<char *>(chunk.addr);
  char *max = addr + chunk.len;
  klog_debug("Chunk start: %x", addr);
  klog_debug("Chunk len: %x", chunk.len);
  int num_512g_frames =
      reinterpret_cast<uintptr_t>(vm::align_up(max, gib_512)) / gib_512;
  total_512g_frames = std::max(total_512g_frames, num_512g_frames);
  char *aligned_4k = vm::align_up_4k(addr);
  char *max_4k = vm::align_down_4k(max);
  maxmem = std::max(maxmem, static_cast<void *>(max_4k - 1));
  char *aligned_2m = vm::align_up_2m(addr);
  char *max_2m = vm::align_down_2m(max);
  char *aligned_1g = vm::align_up_1g(addr);
  char *max_1g = vm::align_down_1g(max);
  klog_debug("4k: %x to %x", aligned_4k, max_4k);
  klog_debug("2m: %x to %x", aligned_2m, max_2m);
  klog_debug("1g: %x to %x", aligned_1g, max_1g);
  for (; aligned_4k < max_4k && aligned_4k < aligned_2m;
       aligned_4k += vm::pgsz_4k) {
    add_frame_4k(static_cast<void *>(aligned_4k));
  }
  klog_debug("num_4k_frames = %x", num_4k_frames);
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
  klog_debug("num_4k_frames = %x", num_4k_frames);
  klog_debug("num_2m_frames = %x", num_2m_frames);
  klog_debug("num_1g_frames = %x", num_1g_frames);
}

void frame_pool::add_frame_4k(void *frame) { ++num_4k_frames; }

void frame_pool::add_frame_2m(void *frame) { ++num_2m_frames; }

void frame_pool::add_frame_1g(void *frame) { ++num_1g_frames; }
