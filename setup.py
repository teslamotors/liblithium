from setuptools import setup

# Get the long description from README.md
with open("README.md") as f:
    readme = f.read()

setup(
    name="pylithium",
    use_scm_version=True,
    description="Python bindings for liblithium",
    long_description=readme,
    url="https://github.com/teslamotors/liblithium",
    author="Chris Copeland",
    author_email="chris@chrisnc.net",
    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Topic :: Security :: Cryptography",
        "Topic :: Software Development :: Embedded Systems",
        "Programming Language :: C",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "License :: OSI Approved :: Apache Software License",
        "License :: OSI Approved :: MIT License",
    ],
    keywords=["x25519", "gimli", "signatures"],
    package_dir={"lithium": "py"},
    packages=["lithium"],
    include_package_data=True,
    package_data={"build_tools": ["ffibuilder.py"]},
    install_requires=["cffi>=1.17.1"],
    setup_requires=["cffi>=1.17.1", "setuptools_scm"],
    cffi_modules=["ffibuilder.py:ffibuilder"],
    entry_points={
        "console_scripts": [
            "gimli-hash = lithium.entry_points.hash:main",
            "lith-keygen = lithium.entry_points.keygen:main",
            "lith-sign = lithium.entry_points.sign:main",
            "lith-verify = lithium.entry_points.verify:main",
        ],
    },
)
