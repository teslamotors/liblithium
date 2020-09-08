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


env = Environment(tools=["cc", "c++", "ar", "link"])
# for color terminal output when available
if "TERM" in os.environ:
    env["ENV"]["TERM"] = os.environ["TERM"]

platform = env["PLATFORM"]
if platform == "win32":
    env["ENV"]["PATH"] = os.environ["PATH"]

llvm_env = env.Clone()

llvm_flags = [
    "-Weverything",
    "-Wno-unknown-warning-option",
    "-Wno-poison-system-directories",
    "-Wno-c99-extensions",
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

llvm_env["CC"] = "clang"

llvm_env.Append(CCFLAGS=llvm_flags, LINKFLAGS=llvm_flags)

if platform == "darwin":
    llvm_env.Append(LINKFLAGS=["-dead_strip"])
elif platform == "posix":
    llvm_env.Append(LINKFLAGS=["-Wl,--gc-sections"])
    # need llvm-ar and llvm-ranlib for LLVM LTO to work on Linux
    llvm_env["AR"] = "llvm-ar"
    llvm_env["RANLIB"] = "llvm-ranlib"

mingw_flags = [
    "-Wall",
    "-Wextra",
    "-Wpedantic",
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
arm_flags = [
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
]
arm_env.Append(CCFLAGS=arm_flags, LINKFLAGS=arm_flags + ["-Wl,--gc-sections"])

if platform == "win32":
    host_env = mingw_env
else:
    host_env = llvm_env
    build_with_env("dist/mingw", mingw_env, test=False)

build_with_env("dist", host_env)

half_env = host_env.Clone()
half_env.Append(CPPDEFINES={"LITH_X25519_WBITS": 16})
build_with_env("dist/half", half_env)

portable_asr_env = host_env.Clone()
portable_asr_env.Append(CPPDEFINES=["LITH_FORCE_PORTABLE_ASR"])
build_with_env("dist/portable_asr", portable_asr_env)

build_with_env("dist/arm", arm_env, test=False)
