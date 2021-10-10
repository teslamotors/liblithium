# SConstruct

import os
import platform


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
    liblith_random = SConscript(
        "src/SConscript-random",
        variant_dir=os.path.join(path, "lib", "random"),
        exports={"env": liblith_env},
        duplicate=False,
    )
    # Prepend so platforms that need extra libraries will have them last and
    # unresolved symbols from liblith_random will be resolved.
    lith_env.Prepend(LIBS=[liblithium, liblith_random])

    SConscript(
        dirs="examples", variant_dir=path, exports={"env": lith_env}, duplicate=False
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

    if test:
        test_env = lith_env.Clone()
        test_env.Append(CPPPATH=Dir("src"))
        SConscript(
            dirs="test",
            variant_dir=os.path.join(path, "test"),
            exports={"env": test_env},
            duplicate=False,
        )
        SConscript(
            dirs="hydro/test",
            variant_dir=os.path.join(path, "hydro", "test"),
            exports={"env": hydro_env},
            duplicate=False,
        )


uname = platform.uname()

if uname.system == "Windows":
    env = Environment(tools=["cc", "c++", "link", "ar"])
    env["ENV"]["PATH"] = os.environ["PATH"]
else:
    env = Environment()

# for color terminal output when available
if "TERM" in os.environ:
    env["ENV"]["TERM"] = os.environ["TERM"]

llvm_env = env.Clone()

llvm_flags = [
    "-Weverything",
    "-Wno-unknown-warning-option",
    "-Wno-poison-system-directories",
    "-Wno-c99-extensions",
    "-Wno-long-long",
    "-Wno-variadic-macros",
    "-Wno-format-non-iso",
    "-Werror",
    "-O3",
    "-g",
    "-flto",
    "-ffunction-sections",
    "-fdata-sections",
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

llvm_env["CC"] = "clang"

llvm_env.Append(CCFLAGS=llvm_flags, LINKFLAGS=llvm_flags)

if uname.system == "Darwin":
    llvm_env.Append(LINKFLAGS=["-dead_strip"])
elif uname.system == "Linux":
    llvm_env.Append(LINKFLAGS=["-Wl,--gc-sections"])
    # need llvm-ar and llvm-ranlib for LLVM LTO to work on Linux
    llvm_env["AR"] = "llvm-ar"
    llvm_env["RANLIB"] = "llvm-ranlib"

mingw_flags = [
    "-Wall",
    "-Wextra",
    "-Werror",
    "-O3",
    "-flto",
    "-ffunction-sections",
    "-fdata-sections",
    "-Wl,--gc-sections",
]
mingw_env = env.Clone(
    CC="x86_64-w64-mingw32-gcc",
    AS="x86_64-w64-mingw32-as",
    AR="x86_64-w64-mingw32-gcc-ar",
    RANLIB="x86_64-w64-mingw32-gcc-ranlib",
    LIBPREFIX="",
    LIBSUFFIX=".lib",
    PROGSUFFIX=".exe",
)
mingw_env.Append(
    CPPDEFINES=["__USE_MINGW_ANSI_STDIO"],
    CCFLAGS=mingw_flags,
    LINKFLAGS=mingw_flags,
    LIBS=["bcrypt"],
)

arm_env = env.Clone(
    CC="arm-none-eabi-gcc",
    LINK="arm-none-eabi-gcc",
    AR="arm-none-eabi-gcc-ar",
    RANLIB="arm-none-eabi-gcc-ranlib",
)

arm_gnu_flags = [
    "-Wall",
    "-Wextra",
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

arm_env.Append(
    CCFLAGS=arm_gnu_flags,
    LINKFLAGS=arm_gnu_flags,
)


def new_x86_env(flags):
    if uname.system == "Windows":
        new_env = mingw_env.Clone()
    else:
        new_env = llvm_env.Clone()
    if uname.system == "Darwin" and uname.machine == "arm64":
        target_flag = "--target=x86_64-apple-darwin"
        new_env.Append(CCFLAGS=target_flag, LINKFLAGS=target_flag)
    new_env.Append(CCFLAGS=flags, LINKFLAGS=flags)
    return new_env


if uname.system == "Windows":
    host_env = mingw_env.Clone()
    arch_flag = "-march=skylake"
else:
    build_with_env("dist/mingw", mingw_env, test=False)
    host_env = llvm_env.Clone()
    if uname.system == "Darwin" and uname.machine == "arm64":
        arch_flag = "-mcpu=apple-a14"
    else:
        arch_flag = "-march=native"

host_env.Append(CCFLAGS=arch_flag, LINKFLAGS=arch_flag)

build_with_env("dist", host_env)

env16 = host_env.Clone()
env16.Append(CPPDEFINES={"LITH_X25519_WBITS": 16})
build_with_env("dist/16", env16)

env32 = host_env.Clone()
env32.Append(CPPDEFINES={"LITH_X25519_WBITS": 32})
build_with_env("dist/32", env32)

portable_asr_env = host_env.Clone()
portable_asr_env.Append(CPPDEFINES=["LITH_FORCE_PORTABLE_ASR"])
build_with_env("dist/portable_asr", portable_asr_env)

build_with_env("dist/arm", arm_env, test=False)

# no SSE
no_sse_env = new_x86_env("-mno-sse")
build_with_env("dist/no-sse", no_sse_env, test=True)

# SSE, etc., but no AVX
nehalem_env = new_x86_env("-march=nehalem")
build_with_env("dist/nehalem", nehalem_env, test=True)

# Everything except AVX512
skylake_env = new_x86_env("-march=skylake")
build_with_env("dist/skylake", skylake_env, test=False)

# AVX512
icelake_env = new_x86_env("-march=icelake-server")
build_with_env("dist/icelake", icelake_env, test=False)
