// Part of the Crubit project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef CRUBIT_RS_BINDINGS_FROM_CC_DECL_IMPORTER_H_
#define CRUBIT_RS_BINDINGS_FROM_CC_DECL_IMPORTER_H_

#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "absl/status/statusor.h"
#include "lifetime_annotations/lifetime_annotations.h"
#include "rs_bindings_from_cc/bazel_types.h"
#include "rs_bindings_from_cc/ir.h"

namespace crubit {

// Top-level parameters as well as return value of an importer invocation.
class Invocation {
 public:
  Invocation(BazelLabel target, absl::Span<const HeaderName> entry_headers,
             const absl::flat_hash_map<const HeaderName, const BazelLabel>&
                 header_targets)
      : target_(target),
        entry_headers_(entry_headers),
        lifetime_context_(std::make_shared<
                          clang::tidy::lifetimes::LifetimeAnnotationContext>()),
        header_targets_(header_targets) {
    // Caller should verify that the inputs are non-empty.
    CHECK(!entry_headers_.empty());
    CHECK(!header_targets_.empty());

    ir_.used_headers.insert(ir_.used_headers.end(), entry_headers_.begin(),
                            entry_headers.end());
    ir_.current_target = target_;
  }

  // Returns the target of a header, if any.
  std::optional<BazelLabel> header_target(const HeaderName header) const {
    auto it = header_targets_.find(header);
    return (it != header_targets_.end()) ? std::optional(it->second)
                                         : std::nullopt;
  }

  // The main target from which we are importing.
  const BazelLabel target_;

  // The headers from which the import starts (a collection of
  // paths in the format suitable for a google3-relative quote include).
  const absl::Span<const HeaderName> entry_headers_;

  const std::shared_ptr<clang::tidy::lifetimes::LifetimeAnnotationContext>
      lifetime_context_;

  // The main output of the import process
  IR ir_;

 private:
  const absl::flat_hash_map<const HeaderName, const BazelLabel>&
      header_targets_;
};

// Converts primitive types like `std::usize` or `int64_t` into their Rust
// equivalents.
std::optional<absl::string_view> MapKnownCcTypeToRsType(
    absl::string_view cc_type);

// Explicitly defined interface that defines how `DeclImporter`s are allowed to
// interface with the global state of the importer.
class ImportContext {
 public:
  ImportContext(Invocation& invocation, clang::ASTContext& ctx,
                clang::Sema& sema)
      : invocation_(invocation), ctx_(ctx), sema_(sema) {}
  virtual ~ImportContext() {}

  // Imports all decls contained in a `DeclContext`.
  virtual void ImportDeclsFromDeclContext(
      const clang::DeclContext* decl_context) = 0;

  // Imports an unsupported item with a single error message.
  virtual IR::Item ImportUnsupportedItem(const clang::Decl* decl,
                                         std::string error) = 0;

  // Imports an unsupported item with multiple error messages.
  virtual IR::Item ImportUnsupportedItem(const clang::Decl* decl,
                                         std::set<std::string> errors) = 0;

  // Imports a decl and creates an IR item (or error messages). This allows
  // importers to recursively delegate to other importers.
  // Does not use or update the cache.
  virtual std::optional<IR::Item> ImportDecl(clang::Decl* decl) = 0;

  virtual std::optional<IR::Item> GetImportedItem(const clang::Decl* decl) = 0;

  // Imports children of `decl`.
  //
  // Returns item ids of the children that belong to the current target.  This
  // includes ids of comments within `decl`.  The returned ids are ordered by
  // their source order.
  virtual std::vector<ItemId> GetItemIdsInSourceOrder(clang::Decl* decl) = 0;

  // Mangles the name of a named decl.
  virtual std::string GetMangledName(
      const clang::NamedDecl* named_decl) const = 0;

  // Returs the label of the target that contains a decl.
  virtual BazelLabel GetOwningTarget(const clang::Decl* decl) const = 0;

  // Checks if the given decl belongs to the current target. Does not look into
  // other redeclarations of the decl.
  virtual bool IsFromCurrentTarget(const clang::Decl* decl) const = 0;

  // Gets an IR UnqualifiedIdentifier for the named decl.
  //
  // If the decl's name is an identifier, this returns that identifier as-is.
  //
  // If the decl is a special member function or operator overload, this returns
  // a SpecialName.
  //
  // If the translated name is not yet implemented, this returns null.
  virtual std::optional<UnqualifiedIdentifier> GetTranslatedName(
      const clang::NamedDecl* named_decl) const = 0;

  // GetTranslatedName, but only for identifier names. This is the common case.
  virtual std::optional<Identifier> GetTranslatedIdentifier(
      const clang::NamedDecl* named_decl) const = 0;

  // Gets the doc comment of the declaration.
  virtual llvm::Optional<std::string> GetComment(
      const clang::Decl* decl) const = 0;

  // Converts a Clang source location to IR.
  virtual SourceLoc ConvertSourceLocation(clang::SourceLocation loc) const = 0;

  // Converts the Clang type `qual_type` into an equivalent `MappedType`.
  // Lifetimes for the type can optionally be specified using `lifetimes`.
  // If `qual_type` is a pointer type, `nullable` specifies whether the
  // pointer can be null.
  // TODO(b/209390498): Currently, we're able to specify nullability only for
  // top-level pointers. Extend this so that we can specify nullability for
  // all pointers contained in `qual_type`, in the same way that `lifetimes`
  // specifies lifetimes for all these pointers. Once this is done, make sure
  // that all callers pass in the appropriate information, derived from
  // nullability annotations.
  virtual absl::StatusOr<MappedType> ConvertQualType(
      clang::QualType qual_type,
      std::optional<clang::tidy::lifetimes::ValueLifetimes>& lifetimes,
      bool nullable = true) = 0;

  // Marks `decl` as successfully imported.  Other pieces of code can check
  // HasBeenAlreadySuccessfullyImported to avoid introducing dangling ItemIds
  // that refer to an unimportable `decl`.
  virtual void MarkAsSuccessfullyImported(const clang::TypeDecl* decl) = 0;

  // Returns whether the `decl` has been already successfully imported (maybe
  // partially - e.g. CXXRecordDeclImporter::Import marks the import as success
  // before importing the fields, because the latter cannot fail).  See also
  // MarkAsSuccessfullyImported.
  virtual bool HasBeenAlreadySuccessfullyImported(
      const clang::TypeDecl* decl) const = 0;

  // Adds an asssociation from anonymous structs/unionts to their typedef names.
  virtual void AddAnonDeclTypedefName(clang::Decl* record,
                                      absl::string_view name) = 0;

  Invocation& invocation_;
  clang::ASTContext& ctx_;
  clang::Sema& sema_;
};

// Interface for components that can import decls of a certain category.
class DeclImporter {
 public:
  DeclImporter(ImportContext& ictx) : ictx_(ictx) {}
  virtual ~DeclImporter() {}

  // Determines whether this importer is autoritative for a decl. This does not
  // imply that the import will be succesful.
  virtual bool CanImport(clang::Decl*) = 0;

  // Returns an IR item for a decl, or `std::nullopt` if importing failed.
  // This member function may only be called after `CanImport` returned `true`.
  virtual std::optional<IR::Item> ImportDecl(clang::Decl*) = 0;

 protected:
  ImportContext& ictx_;
};

// Common implementation for defining `DeclImporter`s that determine their
// applicability by the dynamic type of the decl.
template <typename D>
class DeclImporterBase : public DeclImporter {
 public:
  DeclImporterBase(ImportContext& context) : DeclImporter(context) {}

 protected:
  bool CanImport(clang::Decl* decl) { return clang::isa<D>(decl); }
  std::optional<IR::Item> ImportDecl(clang::Decl* decl) {
    return Import(clang::cast<D>(decl));
  }
  virtual std::optional<IR::Item> Import(D*) = 0;
};

}  // namespace crubit

#endif  // CRUBIT_RS_BINDINGS_FROM_CC_DECL_IMPORTER_H_
