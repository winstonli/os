#pragma once

class PACKED pdpe {
  bool nx : 1;
  uint64_t available : 11;
  uint64_t pdp_base_paddr : 40;
  uint64_t avl : 3;
  bool mbz : 1;
  bool ps : 1;
  bool ign : 1;
  bool a : 1;
  bool pcd : 1;
  bool pwt : 1;
  bool us : 1;
  bool rw : 1;
  bool p : 1;
};
