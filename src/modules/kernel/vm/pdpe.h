#pragma once

#include <common/common.h>

#include <vm/pde.h>

class PACKED pdpe {
  bool p : 1;
  bool rw : 1;
  bool us : 1;
  bool pwt : 1;
  bool pcd : 1;
  bool a : 1;
  bool ign : 1;
  bool ps : 1;
  bool mbz : 1;
  uint64_t avl : 3;
  uint64_t pdp_base_paddr : 40;
  uint64_t available : 11;
  bool nx : 1;

public:
  pde *get_pd() const;
};
