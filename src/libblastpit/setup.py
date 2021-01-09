#!/usr/bin/env python

from distutils.core import setup, Extension

setup(name='blastpy',
      version='0.1',
      author="Andrew Black",
      description="""Python interface to Rofin laser controller""",
      py_modules=["blastpy"],
      packages=[""],
      package_data={'': ['blastpy.py', '_blastpy.so']},
      )
