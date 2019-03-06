# %BANNER_BEGIN%
# ---------------------------------------------------------------------
# %COPYRIGHT_BEGIN%
#
# Copyright (c) 2018 Magic Leap, Inc. (COMPANY) All Rights Reserved.
# Magic Leap, Inc. Confidential and Proprietary
#
#  NOTICE:  All information contained herein is, and remains the property
#  of COMPANY. The intellectual and technical concepts contained herein
#  are proprietary to COMPANY and may be covered by U.S. and Foreign
#  Patents, patents in process, and are protected by trade secret or
#  copyright law.  Dissemination of this information or reproduction of
#  this material is strictly forbidden unless prior written permission is
#  obtained from COMPANY.  Access to the source code contained herein is
#  hereby forbidden to anyone except current COMPANY employees, managers
#  or contractors who have executed Confidentiality and Non-disclosure
#  agreements explicitly covering such access.
#
#  The copyright notice above does not evidence any actual or intended
#  publication or disclosure  of  this source code, which includes
#  information that is confidential and/or proprietary, and is a trade
#  secret, of  COMPANY.   ANY REPRODUCTION, MODIFICATION, DISTRIBUTION,
#  PUBLIC  PERFORMANCE, OR PUBLIC DISPLAY OF OR THROUGH USE  OF THIS
#  SOURCE CODE  WITHOUT THE EXPRESS WRITTEN CONSENT OF COMPANY IS
#  STRICTLY PROHIBITED, AND IN VIOLATION OF APPLICABLE LAWS AND
#  INTERNATIONAL TREATIES.  THE RECEIPT OR POSSESSION OF  THIS SOURCE
#  CODE AND/OR RELATED INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS
#  TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE,
#  USE, OR SELL ANYTHING THAT IT  MAY DESCRIBE, IN WHOLE OR IN PART.
#
# %COPYRIGHT_END%
# ---------------------------------------------------------------------
# %BANNER_END%
import subprocess

import re

import platforms
from defaults import DEVICE_DEFAULT
from diags import debug
from toolchains._base import Toolchain, version_compare
from toolchains._gcc_like import GCCLike


class CLang(GCCLike):
    # initialize to earliest supported, then
    # let detection update to latest incrementally
    VERSION_CLANG_LATEST = '3.8'

    def __init__(self, name, version=None, platform=None):
        GCCLike.__init__(self, name, version, platform)
        self._suffix = name[5:]  # after 'clang'

    def detect(self):
        """
        Detect the desired tools in the machine or environment.

        This should populate self._tools.

        This method tell whether the receiver is available
        for real builds.  Even if this method fails, however,
        the receiver may still be used, e.g. for unit tests.
        :rtype: str for error or None for detected
        """
        GCCLike.detect(self)

        if self._platform != platforms.lumin.NAME:
            err = self.detect_version_on_path_or_env('CPP', 'cpp', False)
            if err:
                return err
            err = self.detect_version_on_path_or_env('CC', 'clang',
                                                     needs_version=self._suffix != '',
                                                     allow_unversioned=not self._suffix)
            if err:
                return err
            err = self.detect_version_on_path_or_env('CXX', 'clang++',
                                                     needs_version=self._suffix != '',
                                                     allow_unversioned=not self._suffix)
            if err:
                return err
            err = self.detect_version_on_path_or_env('AS', 'llvm-as',
                                                     needs_version=self._suffix != '',
                                                     allow_unversioned=not self._suffix)
            if err:
                err = self.detect_version_on_path_or_env('AS', 'as', False)
                if err:
                    return err
            err = self.detect_version_on_path_or_env('AR', 'ar', False)
            if err:
                return err
        else:
            err = self.add_cross_toolchain_tool('CPP', 'cpp')
            if err:
                return err
            err = self.add_cross_toolchain_tool('CC', 'clang')
            if err:
                return err
            err = self.add_cross_toolchain_tool('CXX', 'clang++')
            if err:
                return err
            err = self.add_cross_toolchain_tool('AS', 'as')
            if err:
                return err
            err = self.add_cross_toolchain_tool('AR', 'gcc-ar')
            if err:
                return err
            err = self.add_cross_toolchain_tool('OBJCOPY', 'objcopy')
            if err:
                return err
            err = self.add_cross_toolchain_tool('STRIP', 'strip')
            if err:
                return err

        return None

    @classmethod
    def find_and_register(cls, name, vers):
        """
        See if a host clang toolchain with a specific version is available,
        by searching for e.g. 'clang-3.8'.  If there is no vers,
        look for generic `clang` then use `clang --version` to
        parse the specific version it claims to be.
        :param name: the executable to check
        :param vers: the version to check
        """
        # locate the given tool using its name as an executable,
        # and adjust the version to match the detected version (major.minor)

        tc = CLang(name, vers)

        if tc.version is None:
            debug("looking for {}".format(tc.name))
            err = tc.detect_version_on_path_or_env('CC', tc.name, needs_version=False,
                                                   allow_unversioned=(vers is None))
            if err is not None:
                return

            try:
                comp = subprocess.run([tc.name, '--version'],
                                      stdout=subprocess.PIPE, stderr=subprocess.PIPE)

                # ignore stray intl chars
                comp.stdout = str(comp.stdout, encoding='ascii', errors='ignore')
                comp.stderr = str(comp.stderr, encoding='ascii', errors='ignore')
            except OSError as e:
                # e.g. PermissionError, from trying to run Cygwin softlink
                debug("failed to invoke '{}', error '{}'".format(tc.name, e))
                return

            if comp.returncode == 0:
                # e.g. clang version 3.8.0-2ubuntu3~trusty5 (tags/RELEASE_380/final)
                # e.g. clang version 3.8.1 (branches/release_38)
                # e.g. clang version 5.0.1-svn319952-1~exp1 (branches/release_50)
                # and the outliers:
                # e.g. Apple LLVM version 8.0.0 (clang-800.0.42.1)
                # e.g. Apple LLVM version 10.0.0 (clang-1000.11.45.2)
                # the version does not correspond to any actual upstream LLVM version :-p
                stdout = comp.stdout
                m = re.match(r'.*?\s+version\s+(\d+\.\d+)(\.\d+)?.*', stdout, re.M)
                if m:
                    version = m.group(1)
                    debug("matched {}".format(version))

                    # then rename this tool to be more specific
                    tc.name = 'clang-' + version
                    tc.version = version
                else:
                    debug("did not find version information in output: {}".format(
                        comp.stdout + comp.stderr))
            else:
                debug("failed to run '{} --version': {}".format(tc.name, comp.stderr))

        if tc.version:
            if version_compare(tc.version, cls.VERSION_CLANG_LATEST) > 0:
                cls.VERSION_CLANG_LATEST = tc.version

            Toolchain.register(tc, force=True)


# try to register known versions
CLang.find_and_register('clang-3.8', '3.8')
CLang.find_and_register('clang-3.9', '3.9')
CLang.find_and_register('clang-5.0', '5.0')

# register an unversioned one...
CLang.find_and_register('clang', None)

if CLang.VERSION_CLANG_LATEST:
    # and register the generic alias for the newest release
    Toolchain.register_alias('clang', 'clang-' + CLang.VERSION_CLANG_LATEST)

# Lumin OS target
Toolchain.register(CLang('clang-3.8', 'clang-3.8', DEVICE_DEFAULT))
Toolchain.register_alias('clang', 'clang-3.8', DEVICE_DEFAULT)

