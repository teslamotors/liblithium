# SConstruct

import os


def build_with_env(path, env, test=True):
    liblithium = SConscript(
        dirs="src",
        variant_dir=os.path.join(path, "lib"),
        exports=["env"],
        duplicate=False,
    )

    lith_env = env.Clone()
    lith_env.Append(LIBS=[liblithium])

    if test:
        SConscript(
            dirs="test",
            variant_dir=os.path.join(path, "test"),
            exports={"env": lith_env},
            duplicate=False,
        )

    SConscript(
        dirs="examples", variant_dir=path, exports={"env": lith_env}, duplicate=False
    )


env = Environment(CPPPATH=[Dir("include")])
# for color terminal output when available
if "TERM" in os.environ:
    env["ENV"]["TERM"] = os.environ["TERM"]

platform = env["PLATFORM"]

host_env = env.Clone()

if platform == "darwin":
    # SCons invokes 'gcc' normally on OS X.
    # Usually this is just clang but with options that we don't need.
    host_env["CC"] = "clang"
    host_env.Append(
        CCFLAGS=[
            "-Weverything",
            "-Werror",
            "-O3",
            "-g",
            "-flto",
            "-ffunction-sections",
            "-fdata-sections",
            "-march=native",
        ],
        LINKFLAGS=["-O3", "-g", "-flto", "-dead_strip"],
    )
elif platform == "posix":
    host_env.Append(
        CCFLAGS=[
            "-Wall",
            "-Wextra",
            "-Wpedantic",
            "-Wconversion",
            "-Werror",
            "-O3",
            "-g",
            "-flto",
            "-ffunction-sections",
            "-fdata-sections",
            "-fsanitize=address,undefined",
            "-march=native",
        ],
        LINKFLAGS=["-O3", "-g", "-flto", "-Wl,--gc-sections"],
        LIBS=["asan", "ubsan"],
    )
elif platform == "win32":
    host_env.Append(CCFLAGS=["/W4", "/WX", "/Ox"], CPPDEFINES=["_CRT_RAND_S"])
else:
    raise Exception("unsupported platform")

build_with_env("dist", host_env)

arm_env = env.Clone()

GCCFLAGS = [
    "--specs=nosys.specs",
    "--specs=nano.specs",
    "-mcpu=cortex-m4",
    "-Os",
    "-flto",
    "-g",
]

arm_env["CC"] = "arm-none-eabi-gcc"
arm_env["LINK"] = "arm-none-eabi-gcc"
arm_env["AR"] = "arm-none-eabi-gcc-ar"
arm_env["RANLIB"] = "arm-none-eabi-gcc-ranlib"
arm_env.Append(
    CCFLAGS=GCCFLAGS + ["-ffunction-sections", "-fdata-sections"],
    LINKFLAGS=GCCFLAGS + ["-Wl,--gc-sections"],
)

build_with_env("dist/arm", arm_env, test=False)
