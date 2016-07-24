#pragma once

#include <common/common.h>

#include <vm/pdpe.h>

class PACKED pml4e {

  bool nx : 1;
  uint64_t available : 11;
  uint64_t pdpe_base_paddr : 40;
  uint64_t avl : 3;
  uint64_t mbz : 2;
  bool ign : 1;
  bool a : 1;
  bool pcd : 1;
  bool pwt : 1;
  bool us : 1;
  bool rw : 1;
  bool p : 1;

  void *get_pdpe_base_paddr() const;
  void *get_pdpe_base_kvaddr() const;

public:
  pdpe *get_pdp() const;
};
