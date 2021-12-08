# from setuptools import setup
# from distutils.core import Extension

# module = Extension('tongrams',
#                     define_macros = [('MAJOR_VERSION', '1'),
#                                      ('MINOR_VERSION', '0')],
#                     include_dirs = ['./include', './external/emphf', './external/essentials'],
#                     libraries = ['boost_iostreams', 'boost_regex'],
#                     extra_compile_args=['-std=c++17'],
#                     sources = ['python/tongrams.cpp'])

# setup(ext_modules = [module])

# exit()
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
import os

include_dirs = ['./include', './external/emphf', './external/essentials',
                './boost/include', './boost/boost/include']

ext_modules = [
    Pybind11Extension("tongrams",
                      sources=['./python/tongrams_pybind.cpp'],
                      cxx_std=17,
                      include_dirs=include_dirs,
                      libraries=['boost_iostreams', 'boost_regex'],
                      library_dirs=['./lib'])
]
setup(
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
