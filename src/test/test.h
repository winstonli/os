#pragma once

#include <assert.h>
#include <log.h>

#include <util/fixedsize_vector.h>

class test_runner {

  struct unit {
    const char *_name;
    size_t num_tests;
    unit(const char *name) : _name(name), num_tests(0) {}

  public:
    void incr() { ++num_tests; }
  };

  struct test {
    const char *_name;
    void (*_test_func)();
    test(const char *name, void (*test_func)()) : _name(name), _test_func(test_func) {}
  };

  fixedsize_vector<unit, 0x100> units;
  fixedsize_vector<test, 0x1000> tests;

public:

  void add_unit(const char *name) {
    units.push_back(unit(name));
  }

  void add_test(void (*test_func)(), const char *name) {
    tests.push_back(test(name, test_func));
    units[units.size() - 1].incr();
  }

  void run_tests() {
    auto total_units = units.size();
    auto total_tests = tests.size();
    klog("Running %d unit%s containing %d test%s.", total_units, plur(total_units), total_tests, plur(total_tests));
    size_t t = 0;
    for (size_t i = 0; i < total_units; ++i) {
      const unit &u = units[i];
      const auto &num_tests = u.num_tests;
      const auto &unit_name = u._name;
      const char *s = "";
      if (num_tests == 1) {
        s = "s";
      }
      klog("");
      klog("== %s ==", unit_name);
      for (size_t j = 0; j < num_tests; ++j, ++t) {
        const test &test = tests[t];
        klog("[%d/%d] %s.%s", j + 1, num_tests, unit_name, test._name);
        test._test_func();
      }
      klog("");
    }
    klog("Finished running %d test%s.", total_tests, plur(total_tests));
  }

  static test_runner *inst;

private:

  const char *plur(size_t num) {
    return num == 1 ? "" : "s";
  }

};

#define ASSERT_TRUE(cond) assert(cond)

#define ASSERTF_TRUE(cond, msg, ...) assertf(cond, msg, ##__VA_ARGS__)

#define ASSERT_EQ(expected, actual) assert((expected) == (actual))

#define FAIL() assert(false)

#define TESTS() test_runner r; test_runner::inst = &r

#define RUN_ALL_TESTS() do { test_runner::inst->run_tests(); } while (false)

#define UNIT(unitname) void unitname ()

#define TEST(unitname, testname) void unitname##__##testname ()

#define ADD_UNIT(unitname) do { void unitname (); test_runner::inst->add_unit(#unitname); unitname (); } while (false)

#define ADD_TEST(unitname, testname) void unitname##__##testname (); do { test_runner::inst->add_test(&unitname##__##testname, #testname); } while (false)

