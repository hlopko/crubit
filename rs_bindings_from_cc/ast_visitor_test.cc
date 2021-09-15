// Part of the Crubit project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "devtools/cymbal/common/clang_tool.h"
#include "rs_bindings_from_cc/frontend_action.h"
#include "rs_bindings_from_cc/ir.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "third_party/absl/container/flat_hash_map.h"
#include "third_party/absl/strings/string_view.h"
#include "third_party/absl/strings/substitute.h"
#include "third_party/absl/types/span.h"
#include "third_party/llvm/llvm-project/clang/include/clang/Frontend/FrontendAction.h"

namespace rs_bindings_from_cc {
namespace {

using ::testing::IsEmpty;
using ::testing::SizeIs;

constexpr absl::string_view kVirtualInputPath =
    "ast_visitor_test_virtual_input.cc";

IR ImportCode(absl::Span<const absl::string_view> header_files_contents,
              const std::vector<absl::string_view>& args) {
  std::vector<std::string> headers;
  absl::flat_hash_map<std::string, std::string> file_contents;
  std::string virtual_input_file_content;

  int counter = 0;
  for (const absl::string_view header_content : header_files_contents) {
    std::string filename(
        absl::Substitute("test/testing_header_$0.h", counter++));
    file_contents.insert({filename, std::string(header_content)});
    absl::SubstituteAndAppend(&virtual_input_file_content, "#include \"$0\"\n",
                              filename);
    headers.emplace_back(std::move(filename));
  }

  file_contents.insert(
      {std::string(kVirtualInputPath), virtual_input_file_content});

  std::vector<std::string> args_as_strings(args.begin(), args.end());
  args_as_strings.emplace_back(std::string("--syntax-only"));
  args_as_strings.emplace_back(std::string(kVirtualInputPath));

  IR ir;
  devtools::cymbal::RunToolWithClangFlagsOnCode(
      args_as_strings, file_contents,
      std::make_unique<rs_bindings_from_cc::FrontendAction>(
          std::vector<absl::string_view>(headers.begin(), headers.end()), ir));
  return ir;
}

TEST(AstVisitorTest, TestNoop) {
  IR ir = ImportCode({"// nothing interesting there."}, {});
  EXPECT_THAT(ir.Functions(), IsEmpty());
  EXPECT_THAT(ir.UsedHeaders(), SizeIs(1));
  EXPECT_EQ(ir.UsedHeaders()[0].IncludePath(), "test/testing_header_0.h");
}

TEST(AstVisitorTest, TestIREmptyOnInvalidInput) {
  IR ir = ImportCode({"int foo(); But this is not C++"}, {});
  EXPECT_THAT(ir.Functions(), IsEmpty());
}

TEST(AstVisitorTest, TestImportFuncWithVoidReturnType) {
  IR ir = ImportCode({"void Foo();"}, {});
  ASSERT_THAT(ir.Functions(), SizeIs(1));
  Func func = ir.Functions()[0];
  EXPECT_EQ(func.identifier.Ident(), "Foo");
  EXPECT_EQ(func.mangled_name, "_Z3Foov");
  EXPECT_TRUE(func.return_type.IsVoid());
  EXPECT_THAT(func.params, IsEmpty());
}

TEST(AstVisitorTest, TestImportTwoFuncs) {
  IR ir = ImportCode({"void Foo(); void Bar();"}, {});
  ASSERT_THAT(ir.Functions(), SizeIs(2));

  Func foo = ir.Functions()[0];
  EXPECT_EQ(foo.identifier.Ident(), "Foo");
  EXPECT_EQ(foo.mangled_name, "_Z3Foov");
  EXPECT_TRUE(foo.return_type.IsVoid());
  EXPECT_THAT(foo.params, IsEmpty());

  Func bar = ir.Functions()[1];
  EXPECT_EQ(bar.identifier.Ident(), "Bar");
  EXPECT_EQ(bar.mangled_name, "_Z3Barv");
  EXPECT_TRUE(bar.return_type.IsVoid());
  EXPECT_THAT(bar.params, IsEmpty());
}

TEST(AstVisitorTest, TestImportTwoFuncsFromTwoHeaders) {
  IR ir = ImportCode({"void Foo();", "void Bar();"}, {});
  ASSERT_THAT(ir.Functions(), SizeIs(2));
  Func foo = ir.Functions()[0];
  EXPECT_EQ(foo.identifier.Ident(), "Foo");
  Func bar = ir.Functions()[1];
  EXPECT_EQ(bar.identifier.Ident(), "Bar");
}

TEST(AstVisitorTest, TestImportNonInlineFunc) {
  IR ir = ImportCode({"void Foo() {}"}, {});
  ASSERT_THAT(ir.Functions(), SizeIs(1));
  Func func = ir.Functions()[0];
  EXPECT_EQ(func.identifier.Ident(), "Foo");
  EXPECT_FALSE(func.is_inline);
}

TEST(AstVisitorTest, TestImportInlineFunc) {
  IR ir = ImportCode({"inline void Foo() {}"}, {});
  ASSERT_THAT(ir.Functions(), SizeIs(1));
  Func func = ir.Functions()[0];
  EXPECT_EQ(func.identifier.Ident(), "Foo");
  EXPECT_TRUE(func.is_inline);
}

TEST(AstVisitorTest, TestImportFuncJustOnce) {
  IR ir = ImportCode({"void Foo(); void Foo();"}, {});
  ASSERT_THAT(ir.Functions(), SizeIs(1));
  Func func = ir.Functions()[0];
  EXPECT_EQ(func.identifier.Ident(), "Foo");
}

TEST(AstVisitorTest, TestImportFuncParams) {
  IR ir = ImportCode({"int Add(int a, int b);"}, {});
  EXPECT_THAT(ir.Functions(), SizeIs(1));

  Func func = ir.Functions()[0];
  EXPECT_EQ(func.identifier.Ident(), "Add");
  EXPECT_EQ(func.mangled_name, "_Z3Addii");
  EXPECT_EQ(func.return_type.RsName(), "i32");

  EXPECT_THAT(func.params, SizeIs(2));
  EXPECT_EQ(func.params[0].type.RsName(), "i32");
  EXPECT_EQ(func.params[0].identifier.Ident(), "a");
  EXPECT_EQ(func.params[1].type.RsName(), "i32");
  EXPECT_EQ(func.params[1].identifier.Ident(), "b");
}

}  // namespace
}  // namespace rs_bindings_from_cc
