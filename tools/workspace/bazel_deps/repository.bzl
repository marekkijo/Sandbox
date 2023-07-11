load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def bazel_deps_repository():
    commit = "83636439e0da8c284f9677c1777f992d4190573f"
    http_archive(
        name = "com_github_mjbots_bazel_deps",
        url = "https://github.com/mjbots/bazel_deps/archive/{}.zip".format(commit),
        # Try the following empty sha256 hash first, then replace with whatever
        # bazel says it is looking for once it complains.
        sha256 = "6dbeea3275920c7d8b8f4e8ef44de546d611899691614d3891df43063a847012",
        strip_prefix = "bazel_deps-{}".format(commit),
    )
