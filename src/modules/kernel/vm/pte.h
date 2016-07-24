#pragma once

#include <common/common.h>

class PACKED pte {

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
  uint64_t base_paddr : 40;
  uint64_t available : 11;
  bool nx : 1;

public:
  void *get_page_paddr() const;
};
