# coding: utf-8
# vim:softtabstop=4:ts=4:sw=4:expandtab:textwidth=120
import os
from fabric.api import lcd, local


_DEF_CFG = 'Release'
_DEVENV = '"/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 9.0/Common7/IDE/devenv.com"'
_TOP_DIR = os.path.dirname(__file__)
_DEPS_PATH = os.path.join(_TOP_DIR, '3rdparty')



def clean(config=_DEF_CFG):
    'Clean all of winsparkle'
    strs = {'devenv': _DEVENV, 'cfg': config}
    with lcd(_TOP_DIR):
        local('%(devenv)s WinSparkle.sln /clean %(cfg)s /project WinSparkle.vcproj' % strs)
    with lcd(_DEPS_PATH):
        local('%(devenv)s WinSparkleDeps.sln /clean %(cfg)s' % strs)


def build(config=_DEF_CFG):
    'Build all of winsparkle.'
    strs = {'devenv': _DEVENV, 'cfg': config}
    with lcd(_DEPS_PATH):
        local('%(devenv)s WinSparkleDeps.sln /build %(cfg)s' % strs)
    with lcd(_TOP_DIR):
        local('%(devenv)s WinSparkle.sln /build %(cfg)s /project WinSparkle.vcproj' % strs)

