#!/usr/bin/env python

from rlpswrapper import AbstractMain
from rlpswrapper import Error
from rlpswrapper import Success

import sys

class QuickStartMain(AbstractMain):
    def __init__(self):
        AbstractMain.__init__(self)

    def initialize(self):
        print("Initializing Python Quickstart Plugin...")
        return Success()

    def getPluginName(self):
        return "pyquickstart"

def main():
    mainObj = QuickStartMain()
    return mainObj.run(len(sys.argv), sys.argv)

if __name__ == "__main__":
    main()