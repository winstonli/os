#pragma once

#include <cstdint>

#include <boot/multiboot_info.h>

class frame_pool {

  template<typename T0, typename T1>
  using pair = std::pair<T0, T1>;

  static constexpr uint64_t gib_512 = 0xff80'0000'0000;

  /* The highest available physical address in a frame. */
  void *maxmem;
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
     Pointer to a bitmap of available 2 MiB frames.

     Number of bits = total_512g_frames * 512 * 512

     Size will be between 32 KiB and 16 MiB
   */
  uint64_t *frames_2m_bits;

  /*
     Pointer to an array of counts, where each element i in the array
     corresponds to the number of available 1 GiB frames out of the 64 frames
     starting at index 64 * i.

     Number of bytes = total_512g_frames * 512 / 64

     Size will be between 8 B and 4 KiB
   */
  int8_t *counts_1g;

  /*
     As counts_1g, but for 2 MiB frames.

     Number of bytes = total_512g_frames * 512 * 512 / 64

     Size will be between 4 KiB to 2 MiB
   */
  int8_t *counts_2m;

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

  void *get_maxmem() const;

private:

  /*
     Grub does not include the memory used by our modules in the memory map.
     So we have to further partition the chunks from grub :(
   */
  void add_with_reserved(
      const multiboot_info &multiboot,
      /* Pairs of (start, end) addresses. */
      const fixedsize_vector<pair<void *, void*>, 2> &reserved
  );

  void add_memory_chunk(const multiboot_mmap_entry &chunk);
  void add_frame_4k(void *frame);
  void add_frame_2m(void *frame);
  void add_frame_1g(void *frame);

};
