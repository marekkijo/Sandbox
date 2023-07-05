load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name = "wolf_common_lib",
    srcs = [
        "map_renderer.cpp",
        "map_renderer.hpp",
        "player_state.cpp",
        "player_state.hpp",
        "raw_map.cpp",
        "raw_map.hpp",
        "raw_map_from_ascii.cpp",
        "raw_map_from_ascii.hpp",
        "raw_map_from_wolf.cpp",
        "raw_map_from_wolf.hpp",
        "renderer.cpp",
        "renderer.hpp",
        "vector_map.cpp",
        "vector_map.hpp",
        "wolf_map_info.hpp",
        "wolf_renderer.cpp",
        "wolf_renderer.hpp",
    ],
    deps = [
        "//tools:math",
        "//tools:sdl",
        "@glm",
        "@sdl2",
    ],
)

cc_binary(
    name = "wolf_singlethread",
    srcs = [
        "singlethread/main.cpp",
        "singlethread/wolf_renderer_singlethread.cpp",
        "singlethread/wolf_renderer_singlethread.hpp",
    ],
    data = glob(["data/**"]),
    deps = [
        ":wolf_common_lib",
        "//tools:sdl",
        "@boost//:program_options",
        "@sdl2",
    ],
)

cc_binary(
    name = "wolf_multithread",
    srcs = [
        "multithread/main.cpp",
        "multithread/wolf_renderer_multithread.cpp",
        "multithread/wolf_renderer_multithread.hpp",
    ],
    data = glob(["data/**"]),
    deps = [
        ":wolf_common_lib",
        "//tools:sdl",
        "@boost//:program_options",
        "@sdl2",
    ],
)

cc_binary(
    name = "wolf_vulkan_compute",
    srcs = [
        "vulkan_compute/main.cpp",
        "vulkan_compute/wolf_renderer_vulkan_compute.cpp",
        "vulkan_compute/wolf_renderer_vulkan_compute.hpp",
    ],
    data = glob(["data/**"]),
    deps = [
        ":wolf_common_lib",
        "//tools:sdl",
        "@boost//:program_options",
        "@rules_vulkan//vulkan:vulkan_cc_library",
        "@sdl2",
    ],
)
