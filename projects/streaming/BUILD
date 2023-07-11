load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "opengl_streamer",
    srcs = [
        "opengl_streamer/main.cpp",
    ],
    linkopts = [
        "-DEFAULTLIB:opengl32.lib",
        #"-lglfw",
        #"-lGLEW",
        #"-lGLU",
        #"-lGL",
        #"-lglut",
        #"-lsfml-graphics",
        #"-lsfml-window",
        #"-lsfml-system",
    ],
    deps = [
        "//tools:sdl",
        "@com_github_mjbots_bazel_deps//:everything",
        "@sdl2",
    ],
)
