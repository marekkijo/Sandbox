load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "vulkan_minimal_compute",
    srcs = [
        "lodepng.cpp",
        "lodepng.hpp",
        "main.cpp",
    ],
    data = [
        "shaders/comp.spv",
        "shaders/shader.comp",
    ],
    deps = [
        "@rules_vulkan//vulkan:vulkan_cc_library",
    ],
)
