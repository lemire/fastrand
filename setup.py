from setuptools import setup, Extension

module1 = Extension('fastrand', sources = ['fastrandmodule.c'])

setup (name = 'fastrand',
        version = '1.4.0',
        description = 'Fast random number generation in Python',
        ext_modules = [module1])
