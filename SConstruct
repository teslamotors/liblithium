# SConstruct

import os
import platform
import subprocess

import SCons.Errors


def build_with_env(path, env, tests=True, examples=False, measure_size=False):
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
    # This is still enabled with -Weverything even though we are using a modern
    # C standard for the examples and tests, but disabling it globally also
    # disables it for -ansi mode, so only disable it here.
    lith_env.Append(CFLAGS=["-Wno-declaration-after-statement"])

    hydro_env = lith_env.Clone()
    hydro_env.Append(CPPPATH=[Dir("hydro")])
    libhydrogen = SConscript(
        dirs="hydro",
        variant_dir=os.path.join(path, "hydro", "lib"),
        exports={"env": hydro_env},
        duplicate=False,
    )
    hydro_env.Prepend(LIBS=[libhydrogen])

    if tests:
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

    if examples:
        SConscript(
            dirs="examples",
            variant_dir=path,
            exports={"env": lith_env},
            duplicate=False,
        )
        SConscript(
            dirs=".trustinsoft",
            variant_dir=os.path.join(path, "trustinsoft"),
            exports={"env": lith_env},
            duplicate=False,
        )
        SConscript(
            dirs="hydro/examples",
            variant_dir=os.path.join(path, "hydro"),
            exports={"env": hydro_env},
            duplicate=False,
        )

    if measure_size:
        lith_entrypoints = [
            lith_env.Program(
                target=os.path.join(path, "entrypoints", f),
                source=[],
                LINKFLAGS=lith_env["LINKFLAGS"] + ["-Wl,--entry=" + f],
            )
            for f in [
                "lith_sign_create",
                "lith_sign_verify",
                "gimli_aead_encrypt",
                "gimli_aead_decrypt",
                "gimli_hash",
            ]
        ]
        hydro_entrypoints = [
            hydro_env.Program(
                target=os.path.join(path, "entrypoints", f),
                source=[],
                LINKFLAGS=lith_env["LINKFLAGS"] + ["-Wl,--entry=" + f],
            )
            for f in [
                "hydro_sign_create",
                "hydro_sign_verify",
                "hydro_hash_hash",
            ]
        ]
        AlwaysBuild(
            env.Command(
                os.path.join(path, "entrypoint-sizes"),
                lith_entrypoints,
                "$SIZE $SOURCES",
            )
        )


all_targets = [
    "host",
    "arm-eabi",
    "powerpc-linux",
]

AddOption(
    "--target",
    dest="target",
    default="host",
    action="store",
    help=f"choose targets ({', '.join(all_targets)}) or specify \"all\"",
    metavar="TARGET1[,TARGET2...]",
)

targets_str = GetOption("target")
if targets_str == "all":
    targets = all_targets
else:
    targets = [t.strip() for t in targets_str.split(",")]
    for t in targets:
        if t not in all_targets:
            raise SCons.Errors.UserError(f"Unknown target {t}")

arch_default = "native"
if platform.machine() in ("arm64", "aarch64"):
    arch_default = "armv8.4-a"

AddOption(
    "--host-march",
    dest="host_march",
    default=arch_default,
    action="store",
    help=f"set the -march option for the host target, defaults to {arch_default}",
    metavar="ARCH",
)

AddOption(
    "--sanitize",
    dest="sanitize",
    default=False,
    action="store_true",
    help="enable sanitizers on the host target",
)

if platform.system() == "Windows":
    env = Environment(tools=["cc", "c++", "link", "ar"])
    env["ENV"]["PATH"] = os.environ["PATH"]
else:
    env = Environment()

# for color terminal output when available
if "TERM" in os.environ:
    env["ENV"]["TERM"] = os.environ["TERM"]


def test_stamp(target, source, env):
    try:
        subprocess.run([source[0].path]).check_returncode()
    except subprocess.CalledProcessError as e:
        raise SCons.Errors.BuildError(
            errstr=f"test failed with exit code {e.returncode}"
        )
    with open(target[0].path, "w") as f:
        pass


env.Append(
    BUILDERS={
        "TestStamp": SCons.Builder.Builder(
            action=test_stamp,
            suffix=".stamp",
            src_builder="Program",
        )
    }
)

if "host" in targets:

    if platform.system() != "Windows":
        host_env = env.Clone(CC="clang")
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
        if GetOption("sanitize"):
            host_env["ENV"]["MallocNanoZone"] = "0"
            llvm_flags.append("-fsanitize=address,undefined")

        host_env.Append(CCFLAGS=llvm_flags, LINKFLAGS=llvm_flags)

        if platform.system() == "Darwin":
            host_env.Append(LINKFLAGS=["-dead_strip"])
        elif platform.system() == "Linux":
            host_env.Append(LINKFLAGS=["-Wl,--gc-sections"])
            # need llvm-ar and llvm-ranlib for LLVM LTO to work on Linux
            host_env["AR"] = "llvm-ar"
            host_env["RANLIB"] = "llvm-ranlib"

    else:  # Windows
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
        host_env = env.Clone(
            CC="x86_64-w64-mingw32-gcc",
            AS="x86_64-w64-mingw32-as",
            AR="x86_64-w64-mingw32-gcc-ar",
            RANLIB="x86_64-w64-mingw32-gcc-ranlib",
            LIBPREFIX="",
            LIBSUFFIX=".lib",
            PROGSUFFIX=".exe",
            CPPDEFINES=["__USE_MINGW_ANSI_STDIO"],
            CCFLAGS=mingw_flags,
            LINKFLAGS=mingw_flags,
            LIBS=["bcrypt"],
        )

    arch_flag = f"-march={GetOption('host_march')}"
    host_env.Append(CCFLAGS=arch_flag, LINKFLAGS=arch_flag)

    build_with_env("build", host_env, examples=True)

    env16 = host_env.Clone()
    env16.Append(CPPDEFINES={"LITH_X25519_WBITS": 16})
    build_with_env("build/16", env16)

    env32 = host_env.Clone()
    env32.Append(CPPDEFINES={"LITH_X25519_WBITS": 32})
    build_with_env("build/32", env32)

    portable_asr_env = host_env.Clone()
    portable_asr_env.Append(CPPDEFINES=["LITH_FORCE_PORTABLE_ASR"])
    build_with_env("build/portable_asr", portable_asr_env)

    # disable architectural optimizations
    no_opt_env = host_env.Clone()
    no_opt_env.Append(
        CPPDEFINES={
            "LITH_LITTLE_ENDIAN": 0,
            "LITH_BIG_ENDIAN": 0,
            "LITH_SPONGE_WORDS": 0,
            "LITH_VECTORIZE": 0,
        }
    )
    build_with_env("build/no_opt", no_opt_env)


if "arm-eabi" in targets:
    arm_env = env.Clone(
        CC="arm-none-eabi-gcc",
        LINK="arm-none-eabi-gcc",
        AR="arm-none-eabi-gcc-ar",
        RANLIB="arm-none-eabi-gcc-ranlib",
        SIZE="arm-none-eabi-size",
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
        "-fdump-rtl-expand",
        "-Wl,--gc-sections",
    ]

    arm_env.Append(
        CCFLAGS=arm_gnu_flags,
        LINKFLAGS=arm_gnu_flags,
    )
    build_with_env("build/arm-eabi", arm_env, tests=False, measure_size=True)

if "powerpc-linux" in targets:
    ppc_env = env.Clone(
        CC="powerpc-linux-gnu-gcc",
        LINK="powerpc-linux-gnu-gcc",
        AR="powerpc-linux-gnu-gcc-ar",
        RANLIB="powerpc-linux-gnu-gcc-ranlib",
    )

    ppc_gnu_flags = [
        "-Wall",
        "-Wextra",
        "-Werror",
        "-mcpu=power9",
        "-O3",
        "-flto",
        "-ffat-lto-objects",
        "-g",
        "-ffunction-sections",
        "-fdata-sections",
        "-fstack-usage",
        "-fdump-rtl-expand",
        "-Wl,--gc-sections",
    ]

    ppc_env.Append(
        CCFLAGS=ppc_gnu_flags,
        LINKFLAGS=ppc_gnu_flags,
    )

    build_with_env("build/powerpc", ppc_env, tests=False)
