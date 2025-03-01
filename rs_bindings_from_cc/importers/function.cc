// Part of the Crubit project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "rs_bindings_from_cc/importers/function.h"

#include "absl/strings/substitute.h"
#include "rs_bindings_from_cc/ast_util.h"
#include "clang/Sema/Sema.h"

namespace crubit {

std::optional<IR::Item> FunctionDeclImporter::Import(
    clang::FunctionDecl* function_decl) {
  if (!ictx_.IsFromCurrentTarget(function_decl)) return std::nullopt;
  if (function_decl->isDeleted()) return std::nullopt;
  if (function_decl->isInStdNamespace() &&
      function_decl->hasAttr<clang::VisibilityAttr>()) {
    auto visibility = function_decl->getAttr<clang::VisibilityAttr>();
    if (visibility->getVisibility() ==
        clang::VisibilityAttr::VisibilityType::Hidden) {
      return ictx_.ImportUnsupportedItem(
          function_decl,
          "Functions from the standard library with hidden visibility are not "
          "supported");
    }
  }

  // TODO(lukasza, mboehme): Consider changing the GetLifetimeAnnotations API to
  // distinguish 1) no lifetime annotations found vs 2) erroneous lifetime
  // annotations found.  This will allow avoiding the call to
  // `expectedToOptional` which can sometimes indicate design problems.
  clang::tidy::lifetimes::LifetimeSymbolTable lifetime_symbol_table;
  llvm::Optional<clang::tidy::lifetimes::FunctionLifetimes> lifetimes =
      expectedToOptional(clang::tidy::lifetimes::GetLifetimeAnnotations(
          function_decl, *ictx_.invocation_.lifetime_context_,
          &lifetime_symbol_table));

  std::vector<FuncParam> params;
  std::set<std::string> errors;
  auto add_error = [&errors](std::string msg) {
    auto result = errors.insert(std::move(msg));
    CHECK(result.second) << "Duplicated error message";
  };
  if (auto* method_decl =
          clang::dyn_cast<clang::CXXMethodDecl>(function_decl)) {
    if (!ictx_.HasBeenAlreadySuccessfullyImported(method_decl->getParent())) {
      return ictx_.ImportUnsupportedItem(function_decl,
                                         "Couldn't import the parent");
    }

    // non-static member functions receive an implicit `this` parameter.
    if (method_decl->isInstance()) {
      std::optional<clang::tidy::lifetimes::ValueLifetimes> this_lifetimes;
      if (lifetimes) {
        this_lifetimes = lifetimes->GetThisLifetimes();
      }
      auto param_type =
          ictx_.ConvertQualType(method_decl->getThisType(), this_lifetimes,
                                /*nullable=*/false);
      if (!param_type.ok()) {
        add_error(absl::StrCat("`this` parameter is not supported: ",
                               param_type.status().message()));
      } else {
        params.push_back({*std::move(param_type), Identifier("__this")});
      }
    }
  }

  if (lifetimes) {
    CHECK(lifetimes->IsValidForDecl(function_decl));
  }

  for (unsigned i = 0; i < function_decl->getNumParams(); ++i) {
    const clang::ParmVarDecl* param = function_decl->getParamDecl(i);
    std::optional<clang::tidy::lifetimes::ValueLifetimes> param_lifetimes;
    if (lifetimes) {
      param_lifetimes = lifetimes->GetParamLifetimes(i);
    }
    auto param_type = ictx_.ConvertQualType(param->getType(), param_lifetimes);
    if (!param_type.ok()) {
      add_error(absl::Substitute("Parameter #$0 is not supported: $1", i,
                                 param_type.status().message()));
      continue;
    }

    std::optional<Identifier> param_name = ictx_.GetTranslatedIdentifier(param);
    CHECK(param_name.has_value());  // No known failure cases.
    params.push_back({*param_type, *std::move(param_name)});
  }

  if (function_decl->getReturnType()->isUndeducedType()) {
    bool still_undeduced = ictx_.sema_.DeduceReturnType(
        function_decl, function_decl->getLocation());
    if (still_undeduced) {
      add_error("Couldn't deduce the return type");
    }
  }

  std::optional<clang::tidy::lifetimes::ValueLifetimes> return_lifetimes;
  if (lifetimes) {
    return_lifetimes = lifetimes->GetReturnLifetimes();
  }

  auto return_type =
      ictx_.ConvertQualType(function_decl->getReturnType(), return_lifetimes);
  if (!return_type.ok()) {
    add_error(absl::StrCat("Return type is not supported: ",
                           return_type.status().message()));
  }

  llvm::DenseSet<clang::tidy::lifetimes::Lifetime> all_free_lifetimes;
  if (lifetimes) {
    all_free_lifetimes = lifetimes->AllFreeLifetimes();
  }

  std::vector<LifetimeName> lifetime_params;
  for (clang::tidy::lifetimes::Lifetime lifetime : all_free_lifetimes) {
    std::optional<llvm::StringRef> name =
        lifetime_symbol_table.LookupLifetime(lifetime);
    CHECK(name.has_value());
    lifetime_params.push_back(
        {.name = name->str(), .id = LifetimeId(lifetime.Id())});
  }
  llvm::sort(lifetime_params,
             [](const LifetimeName& l1, const LifetimeName& l2) {
               return l1.name < l2.name;
             });

  llvm::Optional<MemberFuncMetadata> member_func_metadata;
  if (auto* method_decl =
          clang::dyn_cast<clang::CXXMethodDecl>(function_decl)) {
    switch (method_decl->getAccess()) {
      case clang::AS_public:
        break;
      case clang::AS_protected:
      case clang::AS_private:
      case clang::AS_none:
        // No need for IR to include Func representing private methods.
        // TODO(lukasza): Revisit this for protected methods.
        return std::nullopt;
    }
    llvm::Optional<MemberFuncMetadata::InstanceMethodMetadata>
        instance_metadata;
    if (method_decl->isInstance()) {
      MemberFuncMetadata::ReferenceQualification reference;
      switch (method_decl->getRefQualifier()) {
        case clang::RQ_LValue:
          reference = MemberFuncMetadata::kLValue;
          break;
        case clang::RQ_RValue:
          reference = MemberFuncMetadata::kRValue;
          break;
        case clang::RQ_None:
          reference = MemberFuncMetadata::kUnqualified;
          break;
      }
      instance_metadata = MemberFuncMetadata::InstanceMethodMetadata{
          .reference = reference,
          .is_const = method_decl->isConst(),
          .is_virtual = method_decl->isVirtual(),
          .is_explicit_ctor = false,
      };
      if (auto* ctor_decl =
              clang::dyn_cast<clang::CXXConstructorDecl>(function_decl)) {
        instance_metadata->is_explicit_ctor = ctor_decl->isExplicit();
      }
    }

    member_func_metadata = MemberFuncMetadata{
        .record_id = GenerateItemId(method_decl->getParent()),
        .instance_method_metadata = instance_metadata};
  }

  if (!errors.empty()) {
    return ictx_.ImportUnsupportedItem(function_decl, errors);
  }

  bool has_c_calling_convention =
      function_decl->getType()->getAs<clang::FunctionType>()->getCallConv() ==
      clang::CC_C;
  bool is_member_or_descendant_of_class_template =
      IsFullClassTemplateSpecializationOrChild(function_decl);
  std::optional<UnqualifiedIdentifier> translated_name =
      ictx_.GetTranslatedName(function_decl);

  llvm::Optional<std::string> doc_comment = ictx_.GetComment(function_decl);
  if (!doc_comment.has_value() && is_member_or_descendant_of_class_template) {
    // Despite `is_member_or_descendant_of_class_template` check above, we are
    // not guaranteed that a `func_pattern` exists below.  For example, it may
    // be missing when `function_decl` is an implicitly defined constructor of a
    // class template -- such decls are generated, not instantiated.
    if (clang::FunctionDecl* func_pattern =
            function_decl->getTemplateInstantiationPattern()) {
      doc_comment = ictx_.GetComment(func_pattern);
    }
  }

  std::string mangled_name = ictx_.GetMangledName(function_decl);
  if (is_member_or_descendant_of_class_template) {
    // `thunks_for_class_template_member_functions.md` explains in more detail
    // why the `mangled_name` has to include the target name when working with
    // members or descendants of a class template.
    mangled_name += '_';
    mangled_name += ConvertToCcIdentifier(ictx_.GetOwningTarget(function_decl));
  }

  // Silence ClangTidy, checked above: calling `add_error` if
  // `!return_type.ok()` and returning early if `!errors.empty()`.
  CHECK(return_type.ok());

  if (translated_name.has_value()) {
    return Func{
        .name = *translated_name,
        .owning_target = ictx_.GetOwningTarget(function_decl),
        .doc_comment = std::move(doc_comment),
        .mangled_name = std::move(mangled_name),
        .return_type = *return_type,
        .params = std::move(params),
        .lifetime_params = std::move(lifetime_params),
        .is_inline = function_decl->isInlined(),
        .member_func_metadata = std::move(member_func_metadata),
        .has_c_calling_convention = has_c_calling_convention,
        .is_member_or_descendant_of_class_template =
            is_member_or_descendant_of_class_template,
        .source_loc = ictx_.ConvertSourceLocation(function_decl->getBeginLoc()),
        .id = GenerateItemId(function_decl),
        .enclosing_namespace_id = GetEnclosingNamespaceId(function_decl),
    };
  }
  return std::nullopt;
}

}  // namespace crubit
