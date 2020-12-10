from setuptools import setup
from glob import glob

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir


__version__ = "0.0.1"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension(
        "invind",
        glob("src/*.cpp"),
        # Example: passing in the version to the compiled code
        define_macros=[("VERSION_INFO", __version__)],
        include_dirs=["include"],
        cxx_std="11",
    )
]

install_requires = [
    "pybind11>=2.6.0",
    "numpy >= 1.11",
]


setup(
    name="invind",
    version=__version__,
    author="Tomoki Ohtsuki",
    author_email="tomoki.ohtsuki.19937@outlook.jp",
    description="",
    long_description="",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    setup_requires=install_requires,
    install_requires=install_requires,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)