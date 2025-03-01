// Part of the Crubit project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Automatically @generated Rust bindings for C++ target
// //rs_bindings_from_cc/test/golden:user_of_imported_type_cc
#![rustfmt::skip]
#![feature(custom_inner_attributes)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![deny(warnings)]

// Part of the Crubit project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#[inline(always)]
pub fn UsesImportedType(t: trivial_type_cc::ns::Trivial) -> trivial_type_cc::ns::Trivial {
    unsafe { crate::detail::__rust_thunk___Z16UsesImportedTypeN2ns7TrivialE(t) }
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct UserOfImportedType {
    pub trivial: *mut trivial_type_cc::ns::Trivial,
}
forward_declare::unsafe_define!(
    forward_declare::symbol!("UserOfImportedType"),
    crate::UserOfImportedType
);

impl Default for UserOfImportedType {
    #[inline(always)]
    fn default() -> Self {
        let mut tmp = ::std::mem::MaybeUninit::<Self>::zeroed();
        unsafe {
            crate::detail::__rust_thunk___ZN18UserOfImportedTypeC1Ev(&mut tmp);
            tmp.assume_init()
        }
    }
}

impl<'b> From<::ctor::RvalueReference<'b, crate::UserOfImportedType>> for UserOfImportedType {
    #[inline(always)]
    fn from(__param_0: ::ctor::RvalueReference<'b, crate::UserOfImportedType>) -> Self {
        let mut tmp = ::std::mem::MaybeUninit::<Self>::zeroed();
        unsafe {
            crate::detail::__rust_thunk___ZN18UserOfImportedTypeC1EOS_(&mut tmp, __param_0);
            tmp.assume_init()
        }
    }
}

// rs_bindings_from_cc/test/golden/user_of_imported_type.h;l=14
// Error while generating bindings for item 'UserOfImportedType::operator=':
// operator= for Unpin types is not yet supported.

// rs_bindings_from_cc/test/golden/user_of_imported_type.h;l=14
// Error while generating bindings for item 'UserOfImportedType::operator=':
// operator= for Unpin types is not yet supported.

// CRUBIT_RS_BINDINGS_FROM_CC_TEST_GOLDEN_USER_OF_IMPORTED_TYPE_H_

mod detail {
    #[allow(unused_imports)]
    use super::*;
    extern "C" {
        #[link_name = "_Z16UsesImportedTypeN2ns7TrivialE"]
        pub(crate) fn __rust_thunk___Z16UsesImportedTypeN2ns7TrivialE(
            t: trivial_type_cc::ns::Trivial,
        ) -> trivial_type_cc::ns::Trivial;
        pub(crate) fn __rust_thunk___ZN18UserOfImportedTypeC1Ev<'a>(
            __this: &'a mut ::std::mem::MaybeUninit<crate::UserOfImportedType>,
        );
        pub(crate) fn __rust_thunk___ZN18UserOfImportedTypeC1EOS_<'a, 'b>(
            __this: &'a mut ::std::mem::MaybeUninit<crate::UserOfImportedType>,
            __param_0: ::ctor::RvalueReference<'b, crate::UserOfImportedType>,
        );
    }
}

const _: () = assert!(::std::mem::size_of::<Option<&i32>>() == ::std::mem::size_of::<&i32>());

const _: () = assert!(::std::mem::size_of::<crate::UserOfImportedType>() == 8);
const _: () = assert!(::std::mem::align_of::<crate::UserOfImportedType>() == 8);
const _: () = {
    static_assertions::assert_impl_all!(crate::UserOfImportedType: Clone);
};
const _: () = {
    static_assertions::assert_impl_all!(crate::UserOfImportedType: Copy);
};
const _: () = {
    static_assertions::assert_not_impl_any!(crate::UserOfImportedType: Drop);
};
const _: () = assert!(memoffset::offset_of!(crate::UserOfImportedType, trivial) == 0);
