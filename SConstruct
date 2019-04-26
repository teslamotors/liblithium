# SConstruct

import os

env = Environment(CPPPATH=[Dir("include")])

platform = env["PLATFORM"]

# Clang
if platform == "darwin":
    # SCons invokes 'gcc' normally on OS X.
    # Usually this is just clang but with options that we don't need.
    env["CC"] = "clang"
    env.Append(CCFLAGS=["-Weverything", "-Werror", "-Os", "-g"])

# GCC
if platform == "posix":
    env.Append(
        CCFLAGS=[
            "-Wall",
            "-Wextra",
            "-Wpedantic",
            "-Werror",
            "-Os",
            "-g",
            "-fsanitize=address,undefined",
        ],
        LIBS=["asan", "ubsan", "m", "bsd"],
    )

# MSVC
if platform == "win32":
    env.Append(CCFLAGS=["/W4", "/WX", "/Ox"], CPPDEFINES=["_CRT_RAND_S"])

# for color terminal output when available
if "TERM" in os.environ:
    env["ENV"]["TERM"] = os.environ["TERM"]

liblithium = SConscript(
    dirs="src", variant_dir="dist", exports=["env"], duplicate=False
)

test_env = env.Clone()
test_env.Append(LIBS=[liblithium])
SConscript(
    dirs="test", variant_dir="dist/test", exports={"env": test_env}, duplicate=False
)
