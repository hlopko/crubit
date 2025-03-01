// Part of the Crubit project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <cstddef>
#include <memory>

#include "rs_bindings_from_cc/support/cxx20_backports.h"
#include "rs_bindings_from_cc/support/offsetof.h"
#include "rs_bindings_from_cc/test/golden/templates_source_order.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wthread-safety-analysis"
extern "C" void
__rust_thunk___ZN10MyTemplateI8TopLevelE8processTES0___2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<TopLevel>* __this, struct TopLevel t) {
  __this->processT(t);
}
extern "C" void
__rust_thunk___ZN10MyTemplateIN23test_namespace_bindings5InnerEE8processTES1___2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<test_namespace_bindings::Inner>* __this,
    struct test_namespace_bindings::Inner t) {
  __this->processT(t);
}
extern "C" void
__rust_thunk___ZN10MyTemplateIS_I8TopLevelEE8processTES1___2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<MyTemplate<TopLevel>>* __this,
    class MyTemplate<TopLevel> t) {
  __this->processT(t);
}
extern "C" void
__rust_thunk___ZN10MyTemplateIS_IN23test_namespace_bindings5InnerEEE8processTES2___2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<MyTemplate<test_namespace_bindings::Inner>>* __this,
    class MyTemplate<test_namespace_bindings::Inner> t) {
  __this->processT(t);
}
extern "C" void
__rust_thunk___ZN10MyTemplateIbE8processTEb__2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<bool>* __this, bool t) {
  __this->processT(t);
}
extern "C" void
__rust_thunk___ZN10MyTemplateIcE8processTEc__2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<char>* __this, char t) {
  __this->processT(t);
}
extern "C" void
__rust_thunk___ZN10MyTemplateIdE8processTEd__2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<double>* __this, double t) {
  __this->processT(t);
}
extern "C" void
__rust_thunk___ZN10MyTemplateIfE8processTEf__2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<float>* __this, float t) {
  __this->processT(t);
}
extern "C" void
__rust_thunk___ZN10MyTemplateIiE8processTEi__2f_2fthird_5fparty_2fcrubit_2frs_5fbindings_5ffrom_5fcc_2ftest_2fgolden_3atemplates_5fsource_5forder_5fcc(
    class MyTemplate<int>* __this, int t) {
  __this->processT(t);
}

static_assert(sizeof(class MyTemplate<TopLevel>) == 1);
static_assert(alignof(class MyTemplate<TopLevel>) == 1);

static_assert(sizeof(class MyTemplate<test_namespace_bindings::Inner>) == 1);
static_assert(alignof(class MyTemplate<test_namespace_bindings::Inner>) == 1);

static_assert(sizeof(class MyTemplate<MyTemplate<TopLevel>>) == 1);
static_assert(alignof(class MyTemplate<MyTemplate<TopLevel>>) == 1);

static_assert(
    sizeof(class MyTemplate<MyTemplate<test_namespace_bindings::Inner>>) == 1);
static_assert(
    alignof(class MyTemplate<MyTemplate<test_namespace_bindings::Inner>>) == 1);

static_assert(sizeof(class MyTemplate<bool>) == 1);
static_assert(alignof(class MyTemplate<bool>) == 1);

static_assert(sizeof(class MyTemplate<char>) == 1);
static_assert(alignof(class MyTemplate<char>) == 1);

static_assert(sizeof(class MyTemplate<double>) == 8);
static_assert(alignof(class MyTemplate<double>) == 8);

static_assert(sizeof(class MyTemplate<float>) == 4);
static_assert(alignof(class MyTemplate<float>) == 4);

static_assert(sizeof(class MyTemplate<int>) == 4);
static_assert(alignof(class MyTemplate<int>) == 4);

static_assert(sizeof(struct TopLevel) == 1);
static_assert(alignof(struct TopLevel) == 1);

static_assert(sizeof(struct test_namespace_bindings::Inner) == 1);
static_assert(alignof(struct test_namespace_bindings::Inner) == 1);

#pragma clang diagnostic pop
