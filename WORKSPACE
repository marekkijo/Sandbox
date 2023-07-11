load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "sdl2",
    build_file = "//externals:sdl2.BUILD",
    patch_args = ["-p1"],
    patches = ["//externals:SDL2-devel-2.26.5-VC.patch"],
    sha256 = "446cf6277ff0dd4211e6dc19c1b9015210a72758f53f5034c7e4d6b60e540ecf",
    strip_prefix = "SDL2-2.26.5",
    url = "https://github.com/libsdl-org/SDL/releases/download/release-2.26.5/SDL2-devel-2.26.5-VC.zip",
)

http_archive(
    name = "glm",
    build_file = "//externals:glm.BUILD",
    sha256 = "e7a1abc208278cc3f0dba59c5170d83863b3375f98136d588b8beb74825e503c",
    strip_prefix = "glm-efec5db081e3aad807d0731e172ac597f6a39447",
    url = "https://github.com/g-truc/glm/archive/efec5db081e3aad807d0731e172ac597f6a39447.zip",
)

http_archive(
    name = "com_github_nelhage_rules_boost",
    patches = ["//externals:rules_boost-086d0baee11189ddcbcff01714d0b243c142c7ca.patch"],
    sha256 = "6494cab0968efa2175b194bbdd44b884aca42a0dc9ac1b20b104b53fe64e3776",
    strip_prefix = "rules_boost-086d0baee11189ddcbcff01714d0b243c142c7ca",
    url = "https://github.com/nelhage/rules_boost/archive/086d0baee11189ddcbcff01714d0b243c142c7ca.tar.gz",
)

load("//tools/workspace:default.bzl", "add_default_repositories")

add_default_repositories()

load(
    "@com_github_mjbots_bazel_deps//tools/workspace:default.bzl",
    bazel_deps_add = "add_default_repositories",
)

bazel_deps_add()
