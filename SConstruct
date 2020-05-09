# SConstruct

import os


def build_with_env(path, env, test=True):
    lith_env = env.Clone()
    lith_env.Append(CPPPATH=[Dir("include")])
    liblith_env = lith_env.Clone()
    liblith_env.Append(CFLAGS=["-ansi"])
    liblithium = SConscript(
        dirs="src",
        variant_dir=os.path.join(path, "lib"),
        exports={"env": liblith_env},
        duplicate=False,
    )
    lith_env.Append(LIBS=[liblithium])

    SConscript(
        dirs="examples", variant_dir=path, exports={"env": lith_env}, duplicate=False
    )

    if test:
        SConscript(
            dirs="test",
            variant_dir=os.path.join(path, "test"),
            exports={"env": lith_env},
            duplicate=False,
        )

    hydro_env = lith_env.Clone()
    hydro_env.Append(CPPPATH=[Dir("hydro")])
    libhydrogen = SConscript(
        dirs="hydro",
        variant_dir=os.path.join(path, "hydro", "lib"),
        exports={"env": hydro_env},
        duplicate=False,
    )
    hydro_env.Prepend(LIBS=[libhydrogen])

    SConscript(
        dirs="hydro/examples",
        variant_dir=os.path.join(path, "hydro"),
        exports={"env": hydro_env},
        duplicate=False,
    )


env = Environment()
# for color terminal output when available
if "TERM" in os.environ:
    env["ENV"]["TERM"] = os.environ["TERM"]

platform = env["PLATFORM"]

host_env = env.Clone()

llvm_flags = [
    "-Weverything",
    "-Werror",
    "-O3",
    "-g",
    "-flto",
    "-ffunction-sections",
    "-fdata-sections",
    "-march=native",
]

AddOption(
    "--no-sanitize",
    dest="sanitize",
    default="true",
    action="store_false",
    help="disable sanitizers",
)
if GetOption("sanitize"):
    llvm_flags.append("-fsanitize=address,undefined")

host_env["CC"] = "clang"
host_env["CXX"] = "clang++"

if platform == "darwin":
    link_flags = ["-dead_strip"]
elif platform == "posix":
    link_flags = ["-Wl,--gc-sections"]
    # need llvm-ar and llvm-ranlib for LLVM LTO to work on Linux
    host_env["AR"] = "llvm-ar"
    host_env["RANLIB"] = "llvm-ranlib"
else:
    raise Exception("unsupported platform")

host_env.Append(CCFLAGS=llvm_flags, LINKFLAGS=llvm_flags + link_flags)

build_with_env("dist", host_env)

half_env = host_env.Clone()
half_env.Append(CPPDEFINES={"LITH_X25519_WBITS": 16})
build_with_env("dist/half", half_env)

arm_env = env.Clone()
arm_env["CC"] = "arm-none-eabi-gcc"
arm_env["LINK"] = "arm-none-eabi-gcc"
arm_env["AR"] = "arm-none-eabi-gcc-ar"
arm_env["RANLIB"] = "arm-none-eabi-gcc-ranlib"
flags = [
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Werror",
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
