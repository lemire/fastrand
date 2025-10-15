from setuptools import setup, Extension

# Minimal setup.py for C extension configuration
# All package metadata is in pyproject.toml
#
# NOTE: This file is required to support Python 3.7.
# With Python 3.8+, C extensions can be configured directly in pyproject.toml
# using setuptools>=74.1.0 with [tool.setuptools] ext-modules, but that
# requires dropping Python 3.7 support.
setup(
    ext_modules=[
        Extension('fastrand', sources=['fastrandmodule.c'])
    ]
)
