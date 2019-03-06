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

import subprocess, re
import platforms
from defaults import DEVICE_DEFAULT
from diags import debug
from toolchains._base import Toolchain, version_compare
from toolchains._gcc_like import GCCLike


class GCC(GCCLike):
    # initialize to earliest supported, then
    # let detection update to latest incrementally
    VERSION_GCC_LATEST = '4.9'

    def __init__(self, name, version, platform=None):
        GCCLike.__init__(self, name, version, platform)
        self._suffix = name[3:]  # after 'gcc'

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
            err = self.detect_version_on_path_or_env('CC', 'gcc',
                                                     needs_version=self._suffix != '',
                                                     allow_unversioned=not self._suffix,
                                                     version=self._suffix[1:])
            if err:
                return err
            err = self.detect_version_on_path_or_env('CXX', 'g++',
                                                     needs_version=self._suffix != '',
                                                     allow_unversioned=not self._suffix,
                                                     version=self._suffix[1:])
            if err:
                return err

            err = self._find_cpp()
            if err:
                return err

            err = self.detect_version_on_path_or_env('AS', 'as', False)
            if err:
                return err
            err = self.detect_version_on_path_or_env('AR', 'ar', False)
            if err:
                return err
        else:
            err = self.add_cross_toolchain_tool('CC', 'gcc')
            if err:
                return err
            err = self.add_cross_toolchain_tool('CXX', 'g++')
            if err:
                return err
            err = self.add_cross_toolchain_tool('CPP', 'cpp')
            if err:
                return err
            err = self.add_cross_toolchain_tool('AS', 'as')
            if err:
                return err
            err = self.add_cross_toolchain_tool('AR', 'gcc-ar')
            if err:
                return err
            err = self.add_cross_toolchain_tool('STRIP', 'strip')
            if err:
                return err
            err = self.add_cross_toolchain_tool('OBJCOPY', 'objcopy')
            if err:
                return err

        return None

    @classmethod
    def find_and_register(cls, name, vers):
        """
        See if a host gcc toolchain with a specific version is available,
        by searching for e.g. 'gcc-5', then using `gcc-5 --version` to
        parse the specific major.minor version it claims to be.
        If no version is passed, find a generic executable and again
        run with `--version` to determine its real version.

        :type name: toolchain name
        :type vers: version to check (or None)
        """
        tc = GCC(name, vers)

        if tc.version is None:
            err = tc.detect_version_on_path_or_env('CC', tc.name, allow_unversioned=False)
            if err is not None:
                return

            debug("looking for {}".format(tc.name))
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
                if "Apple LLVM" in comp.stdout:
                    # ah, an impostor; let clang detection figure it out
                    return

                # e.g. gcc (Ubuntu 4.8.5-2ubuntu1~14.04.1) 4.8.5 [YYMMDD]
                m = re.match(r'.*?(\d+\.\d+)(\.\d+)?(\s+\d{8})?.*', comp.stdout, re.M)
                if m:
                    version = m.group(1)
                    debug("matched {}".format(version))

                    if tc.name != 'gcc-' + version:
                        # map e.g. 'gcc-5' to 'gcc-5.4'
                        Toolchain.register_alias(tc.name, 'gcc-' + version)

                    # then rename this tool to be more specific
                    tc.name = 'gcc-' + version
                    tc.version = version

                    if version_compare(version, cls.VERSION_GCC_LATEST) > 0:
                        cls.VERSION_GCC_LATEST = version
                else:
                    debug("did not find version information in output: {}".format(
                        comp.stdout + comp.stderr))
                    return
            else:
                debug("failed to run '{} --version': {}".format(tc.name, comp.stderr))
                return

        if tc.version:
            Toolchain.register(tc, force=True)


# try to register known versions
GCC.find_and_register('gcc-4.8', '4.8')
GCC.find_and_register('gcc-4.9', '4.9')
GCC.find_and_register('gcc-5', None)
GCC.find_and_register('gcc-6', None)
GCC.find_and_register('gcc-7', None)

# register the system version
GCC.find_and_register('gcc', None)

# register generic one...
Toolchain.register_alias('gcc', 'gcc-' + GCC.VERSION_GCC_LATEST)

# register known SDK tools for Lumin OS targeting
Toolchain.register(GCC('gcc-4.9', '4.9', DEVICE_DEFAULT))

Toolchain.register_alias('gcc', 'gcc-4.9', DEVICE_DEFAULT)
