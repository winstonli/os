#pragma once

#include <cstdint>
#include <cstring>

#include <boot/multiboot_info.h>
#include <vm/vm.h>

class frame_pool {

  template<typename T0, typename T1>
  using pair = std::pair<T0, T1>;

  static constexpr uint64_t gib_512 = 0xff80'0000'0000;

  /* The highest available physical address in a frame. */
  void *maxmem;

  size_t avail_mem;
  /*
     The total number of 512g frames available on the system.

     Valid values: 1 to 512.

     Will be equal to 1.
   */
  int total_512g_frames;

  /* Number of available whole 1 GiB frames. */
  int num_1g_frames;

  /* Number of available 2 MiB frames (not part of a whole 1 GiB frame) */
  int num_2m_frames;

  /* Number of available 4 KiB frames (not part of a whole 2 MiB frame) */
  int num_4k_frames;

  /*
     The below members are allocated enough 2 MiB pages to fit their total size.

     Their order on the pages will be:

     frames_2m_bits
     counts_2m
     frames_1g_bits
     counts_1g
   */

  /*
     Pointer to a bitmap of available 1 GiB frames.

     Number of bits = total_512g_frames * 512

     Size will be between 64 B and 32 KiB
   */
  uint64_t *frames_1g_bits;

  /*
     Pointer to an array of counts, where each element i in the array
     corresponds to the number of available 1 GiB frames out of the 64 frames
     starting at index 64 * i.

     Number of bytes = total_512g_frames * 512 / 64

     Size will be between 8 B and 4 KiB
   */
  int8_t *counts_1g;

  size_t size_1g_bits;

  /*
     Pointer to a bitmap of available 2 MiB frames.

     Number of bits = total_512g_frames * 512 * 512

     Size will be between 32 KiB and 16 MiB
   */
  uint64_t *frames_2m_bits;

  /*
     As counts_1g, but for 2 MiB frames.

     Number of bytes = total_512g_frames * 512 * 512 / 64

     Size will be between 4 KiB to 2 MiB
   */
  int8_t *counts_2m;

  size_t size_2m_bits;

  fixedsize_vector<void *, 0x1000> freelist_4k;

public:

  frame_pool(
      const multiboot_info &multiboot,
      void *start_mod_start,
      void *start_mod_end,
      void *kern_mod_start,
      void *kern_mod_end
  );
  frame_pool(const frame_pool &) = delete;
  frame_pool &operator=(const frame_pool &) = delete;
  frame_pool(frame_pool &&) = delete;
  frame_pool &operator=(frame_pool &&) = delete;
  ~frame_pool() = default;

  /*
     Functions to permanently allocate frames.

     Frames allocated by these functions MUST NOT be returned.

     These will not trigger dynamic allocation and are safe to use from memory
     allocators.
   */
  void *falloc_perm_4k();
  void *falloc_perm_2m();
  void *falloc_perm_1g();

  /*
     Functions to allocate frames without triggering dynamic allocation, and are
     safe to use from memory allocators that may cause recursive frame
     allocation.

     The corresponding index() function MUST be called afterwards, which can
     trigger dynamic allocation. Generally, a memory allocator will call
     falloc_critical() when it needs more memory, and then falloc_index() when
     it has updated its free lists.

     Regular falloc/ffree calls MUST NOT be called before all matching index()
     functions are called.

     It is safe for falloc_index() to trigger another falloc_critical() call
     due to dynamic allocation, as long as the memory allocator is re-entrant.
   */
  void *falloc_critical_4k();
  void *falloc_critical_2m();
  void *falloc_critical_1g();

  void *falloc_index_4k();
  void *falloc_index_2m();
  void *falloc_index_1g();

  void *ffree_critical_4k();
  void *ffree_critical_2m();
  void *ffree_critical_1g();

  void *ffree_index_4k();
  void *ffree_index_2m();
  void *ffree_index_1g();

  /*
     General functions for allocating frames.
   */
  void *falloc_4k();
  void *falloc_2m();
  void *falloc_1g();

  void *ffree_4k();
  void *ffree_2m();
  void *ffree_1g();

  void *get_maxmem() const;
  size_t get_avail_mem() const;

private:

  /*
     Calculates the number of 2 MiB frames required for the bitmaps.

     Looks for enough 2 MiB frames in the initial memory map that are under
     1 GiB (all that we've direct-mapped at this point) and allocates them.
   */
  template<size_t N>
  void allocate_bitmaps(
      fixedsize_vector<multiboot_mmap_entry, N> &chunks,
      const fixedsize_vector<pair<void *, void*>, 2> &reserved
  ) {
    std::array<size_t, 3> offsets;
    auto size = calc_bitmap_size_aligned(offsets);
    for (auto i = 0u; i < chunks.size(); ++i) {
      auto &chunk = chunks[i];
      auto &start = chunk.addr;
      auto &len = chunk.len;
      char *const start_addr = reinterpret_cast<char *>(start);
      /* Not big enough. */
      if (len < size) {
        continue;
      }
      /* Find a big enough chunk without splitting a larger frame. */
      auto end = start_addr + len;
      auto start_2m = vm::align_up_2m(start_addr);
      if (start_2m == vm::align_up_1g(start_addr)) {
        /* We don't want to split a larger frame. */
        continue;
      }
      if (start_2m + size >= end) {
        /* This frame isn't big enough. */
        continue;
      }
      if (start_2m >= reinterpret_cast<char *>(vm::pgsz_1g)) {
        /* This frame is past the mapped region. */
        continue;
      }
      /* Found our frame. */
      auto end_bitmaps = start_2m + size;
      if (start_2m == start_addr) {
        start = reinterpret_cast<uintptr_t>(end_bitmaps);
        len = reinterpret_cast<uintptr_t>(end) - start;
      } else {
        /* We have 4k frames below us. We need to split them. */
        assert(start_2m > start_addr);
        len = start_2m - start_addr;
        if (end > end_bitmaps) {
          multiboot_mmap_entry entry;
          entry.addr = reinterpret_cast<uintptr_t>(end_bitmaps);
          entry.len = end - end_bitmaps;
          chunks.push_back(std::move(entry));
        }
      }
      char *bitmaps = static_cast<char *>(
          vm::paddr_to_vaddr(reinterpret_cast<char *>(start_2m))
      );
      memset(bitmaps, 0, size);
      frames_2m_bits = reinterpret_cast<uint64_t *>(bitmaps);
      counts_2m = reinterpret_cast<int8_t *>(bitmaps + offsets[0]);
      size_2m_bits = offsets[1] - offsets[0];
      frames_1g_bits = reinterpret_cast<uint64_t *>(bitmaps + offsets[1]);
      counts_1g = reinterpret_cast<int8_t *>(bitmaps + offsets[2]);
      size_1g_bits = size - offsets[2];
      return;
    }
    assertf(
        false,
        "Unable to allocate frame pool bitmaps." " "
        "No chunks of memory below 1 GiB with trailing 2 MiB frames."
    );
  }

  void set_index_2m(size_t index);
  void set_index_1g(size_t index);

  /*
     Grub does not include the memory used by our modules in the memory map.
     So we have to further partition the chunks from grub :(
   */
  void add_with_reserved(
      const multiboot_info &multiboot,
      /* Pairs of (start, end) addresses. */
      const fixedsize_vector<pair<void *, void*>, 2> &reserved
  );

  void update_maxmem(const multiboot_mmap_entry &chunk);

  void add_memory_chunk(const multiboot_mmap_entry &chunk);
  void add_frame_4k(void *frame);
  void add_frame_2m(void *frame);
  void add_frame_1g(void *frame);

  /*
     Places the required offsets of, respectively:

     counts_2m
     frames_1g_bits
     counts_1g

     into offsets, and returns the total count.

     The offset of frames_2m_bits is 0.
   */
  size_t calc_bitmap_size_aligned(std::array<size_t, 3> &offsets) const;

  size_t sizeof_2m_bitmap() const;

  size_t sizeof_2m_counts() const;

  size_t sizeof_1g_bitmap() const;

  size_t sizeof_1g_counts() const;

};
