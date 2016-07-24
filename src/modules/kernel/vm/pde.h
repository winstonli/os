#pragma once

#include <common/common.h>

#include <vm/pte.h>

class PACKED pde {

  bool p : 1;
  bool rw : 1;
  bool us : 1;
  bool pwt : 1;
  bool pcd : 1;
  bool a : 1;
  bool ign0 : 1;
  bool ps : 1;
  bool ign1 : 1;
  uint64_t avl : 3;
  uint64_t pt_base_paddr : 40;
  uint64_t available : 11;
  bool nx : 1;

public:
  bool has_pt() const;
  pte *get_pt() const;
};
