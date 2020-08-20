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
    name="lithium",
    use_scm_version=True,
    description="Python bindings for liblithium",
    long_description=readme,
    url="https://github.com/teslamotors/liblithium",
    author="Chris Copeland",
    author_email="chris@chrisnc.net",
    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        # How mature is this project? Common values are
        #   1 - Planning
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        "Development Status :: 3 - Alpha",
        # Indicate who your project is intended for
        "Intended Audience :: Developers",
        "Topic :: Security :: Cryptography",
        "Topic :: Software Development :: Embedded Systems",
        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "License :: OSI Approved :: Apache Software License",
    ],
    keywords=["x25519", "gimli", "signatures"],
    package_dir={"lithium": "py_src"},
    packages=["lithium"],
    include_package_data=True,
    package_data={"build_tools": ["ffibuilder.py"]},
    install_requires=["cffi>=1.12.0"],
    setup_requires=["cffi>=1.12.0", "setuptools_scm"],
    cffi_modules=["ffibuilder.py:ffibuilder"],
)
