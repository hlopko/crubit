"""End-to-end example of using type aliases to fully-instantiated templates."""

load("@rules_rust//rust:defs.bzl", "rust_test")

licenses(["notice"])

cc_library(
    name = "non_type_template_params",
    hdrs = ["non_type_template_params.h"],
)

rust_test(
    name = "main",
    srcs = ["test.rs"],
    cc_deps = [":non_type_template_params"],
)
