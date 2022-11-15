"""A setuptools based setup module.

See:
https://packaging.python.org/en/latest/distributing.html
https://github.com/pypa/sampleproject
"""

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
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Topic :: Security :: Cryptography",
        "Topic :: Software Development :: Embedded Systems",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "License :: OSI Approved :: Apache Software License",
        "License :: OSI Approved :: MIT License",
    ],
    keywords=["x25519", "gimli", "signatures"],
    package_dir={"lithium": "py"},
    packages=["lithium"],
    include_package_data=True,
    package_data={"build_tools": ["ffibuilder.py"]},
    install_requires=["cffi>=1.12.0"],
    setup_requires=["cffi>=1.12.0", "setuptools_scm"],
    cffi_modules=["ffibuilder.py:ffibuilder"],
    entry_points={
        "console_scripts": [
            "lith-keygen = lithium.entry_points.lith_keygen:main",
            "lith-sign = lithium.entry_points.lith_sign:main",
            "lith-verify = lithium.entry_points.lith_verify:main",
            "gimli-hash = lithium.entry_points.gimli_hash:main",
        ],
    },
)
