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

nixflags = [
    "-O3",
    "-g",
    "-flto",
    "-ffunction-sections",
    "-fdata-sections",
    "-march=native",
    "-fsanitize=address,undefined",
]

gnuwflags = ["-Wall", "-Wextra", "-Wpedantic", "-Wconversion", "-Werror"]

if platform == "darwin":
    # SCons invokes 'gcc' normally on OS X.
    # Usually this is just clang but with options that we don't need.
    host_env["CC"] = "clang"
    flags = ["-Weverything", "-Werror"] + nixflags
    host_env.Append(CCFLAGS=flags, LINKFLAGS=flags + ["-dead_strip"])
elif platform == "posix":
    flags = gnuwflags + nixflags
    host_env.Append(CCFLAGS=flags, LINKFLAGS=flags + ["-Wl,--gc-sections"])
elif platform == "win32":
    host_env.Append(CCFLAGS=["/W4", "/WX", "/Ox"], CPPDEFINES=["_CRT_RAND_S"])
else:
    raise Exception("unsupported platform")

build_with_env("dist", host_env)

arm_env = env.Clone()
arm_env["CC"] = "arm-none-eabi-gcc"
arm_env["LINK"] = "arm-none-eabi-gcc"
arm_env["AR"] = "arm-none-eabi-gcc-ar"
arm_env["RANLIB"] = "arm-none-eabi-gcc-ranlib"
flags = gnuwflags + [
    "-specs=nosys.specs",
    "-specs=nano.specs",
    "-mcpu=cortex-m4",
    "-Os",
    "-flto",
    "-ffat-lto-objects",
    "-g",
    "-ffunction-sections",
    "-fdata-sections",
    "-fstack-usage",
    "-Wl,--gc-sections",
]
arm_env.Append(CCFLAGS=flags, LINKFLAGS=flags)

build_with_env("dist/arm", arm_env, test=False)
