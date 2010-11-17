from distutils.core import setup, Extension

module = Extension('dtrans',
                   sources = ['dtransmodule.cpp', 'DistanceTransform.cpp'])

setup (name = 'DistanceTransform',
       version = '0.0',
       description = 'blah blah blah',
       ext_modules = [module])
