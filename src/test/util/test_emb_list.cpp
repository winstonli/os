#include <test.h>

#include <util/emb_list.h>

struct test {
  emb_list_entry default_entry;
  int val;

  test(int i) : val(i) {}
};

UNIT(TestEmbList) { ADD_TEST(TestEmbList, Basic); }

TEST(TestEmbList, Basic) {
  emb_list<test> l;
  test t0(0);
  test t1(1);
  l.push_back(t1);
  l.push_back(t0);
  int i = 0;
  for (auto it = l.begin(); it != l.end(); ++it, ++i) {
    switch (i) {
      case 0:
        ASSERT_EQ(1, it->val);
        break;
      case 1:
        ASSERT_EQ(0, it->val);
        break;
      default:
        FAIL();
        break;
    }
  }
  test t2(2);
  l.push_front(t2);
  i = 0;
  for (auto it = l.begin(); it != l.end(); ++it, ++i) {
    switch (i) {
      case 0:
        ASSERT_EQ(2, it->val);
        break;
      case 1:
        ASSERT_EQ(1, it->val);
        break;
      case 2:
        ASSERT_EQ(0, it->val);
        break;
      default:
        FAIL();
        break;
    }
  }
  l.erase(t1);
  i = 0;
  for (auto it = l.begin(); it != l.end(); ++it, ++i) {
    switch (i) {
      case 0:
        ASSERT_EQ(2, it->val);
        break;
      case 1:
        ASSERT_EQ(0, it->val);
        break;
      default:
        FAIL();
        break;
    }
  }
}
