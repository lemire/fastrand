from setuptools import setup, Extension

module1 = Extension('fastrand', sources = ['fastrandmodule.c'])

setup (name = 'fastrand',
        author="Daniel Lemire",
        version = '1.7.1',
        author_email='daniel@lemire.me',
        long_description = open('README.md', 'r', encoding="utf-8").read(),
        long_description_content_type = 'text/markdown',
        description = 'Fast random number generation in Python',
        ext_modules = [module1])
