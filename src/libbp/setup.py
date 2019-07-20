#!/usr/bin/env python

"""
setup.py file for SWIG example
"""

from distutils.core import setup, Extension


blastpy_module = Extension('_blastpy',
                           sources=['blastpy_wrap.c', 'blastpit.c'],
                           )

setup (name = 'example',
       version = '0.1',
       author      = "SWIG Docs",
       description = """Simple swig example from docs""",
       ext_modules = [blastpy_module],
       py_modules = ["blastpy"],
       )
