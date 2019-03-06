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

import os
from abc import abstractmethod, ABCMeta

import math

import build_env
import run_vars
from build_model import BuildObject, Variable, Command, split_by_length, Composition, Literal, Node, Path
from config import HOSTOS
from diags import warn, debug
from utils import which


class Toolchain(BuildObject):
    __metaclass__ = ABCMeta

    """
    A toolchain is the set of tools (compiler, linker, assembler, etc.)
    that processes source and object code to construct build artifacts.
    """
    @classmethod
    def register(cls, bo, force=False):
        """ Register a new instance of a build object.
        An object of the same name must not already be registered. """
        assert issubclass(type(bo), cls)
        name = bo.platformed_name
        if not force: assert name not in cls._ents
        debug("Registering {0} named {1}".format(cls.__name__, name))
        cls._ents[name] = bo

    @classmethod
    def remove(cls, name_or_key):
        """ Remove any object or alias with the given name """
        try:
            if name_or_key.platformed_name in cls._ents:
                del cls._ents[name_or_key.platformed_name]
            if name_or_key.name in cls._ents:
                del cls._ents[name_or_key.name]
        except AttributeError:
            del cls._ents[cls.find(name_or_key).platformed_name]

    @classmethod
    def find(cls, name, platform=None):
        platform = cls.default_platform(platform)
        aliases = cls._alias.get(platform, {})
        while name in aliases:
            name = aliases[name]

        # toolchains have platform-specific names
        platform_name = platform + '_' + name
        if platform_name in cls._ents:
            return cls._ents[platform_name]

        return None

    def __init__(self, name, version=None, platform=None):
        BuildObject.__init__(self, name)

        self._version = version
        self._platform = platform or HOSTOS

        self._tools = {}
        self._command_search_paths = []

    @property
    def version(self):
        return self._version

    @version.setter
    def version(self, v):
        self._version = v

    @property
    def platformed_name(self):
        return self._platform + '_' + self.name

    @property
    def tools(self):
        """
        Get the tools this toolchain uses, as a mapping of
        variable name (e.g. "CC", "LD") to Node or string.
        :rtype: Dict[str, Node|str]
        """
        return self._tools

    @property
    def command_search_paths(self):
        return self._command_search_paths

    def tool_var(self, name):
        """
        Create a variable for the tool_var 'name'
        :param name:
        :return:
        """
        return Variable(name, self._tools.get(name, 'not-found-' + name))

    @abstractmethod
    def detect(self):
        """
        Detect the desired tools in the machine or environment.

        This should populate self._tools.

        This method tell whether the receiver is available
        for real builds.  Even if this method fails, however,
        the receiver may still be used, e.g. for unit tests.
        :rtype: str for error or None for detected
        """
        raise NotImplemented

    def user_override(self, var):
        # check command-line args
        user = run_vars.runtime_values.get(var)
        if not user:
            user = os.getenv(var)
        return user

    def detect_version_on_path_or_env(self, var, base, needs_version=True, allow_unversioned=True, version=None):
        # try override first
        user = self.user_override(var)
        if user:
            file = user

            path = which(file)
            if not path:
                warn("user-specified '{0}' refers to '{1}' not found on PATH".format(var, file))
                path = file

        else:
            if not version:
                version = self.version
            if needs_version and version:
                file = base + '-' + version
            else:
                file = base

            path = which(file)

            if not path and needs_version and allow_unversioned:
                # one more try for host tools
                path = which(base)

            if not path:
                return "did not find '{0}' on PATH".format(file)

        # canonicalize for Unix-like program path
        path = os.path.normpath(path).replace('\\', '/')

        self._tools[var] = path

        return None

    def _find_cpp(self):
        from config import MINGW_DIR, HOSTOS, OS_WIN
        if HOSTOS == OS_WIN:
            # this entry ships with our tools
            self._tools['CPP'] = os.path.abspath(os.path.join(MINGW_DIR, 'cpp.exe'))
            return None
        else:
            return self.detect_version_on_path_or_env('CPP', 'cpp' + (getattr(self, '_suffix') or ''), False)

    @property
    def versioned_name(self):
        if self._version and self._version not in self._name:
            return self._name + '-' + self._version
        return self._name

    @abstractmethod
    def get_unique_artifacts(self, proj):
        """
        Get project artifacts specific to a toolchain.
        :type proj: projects.Project
        :return: List of project artifacts for the toolchain.
        :rtype: List[build_model.Path]
        """
        raise NotImplementedError()

    @abstractmethod
    def create_assembler_commands(self, env, proj, kind, source_path, obj_path):
        """
        Create command(s) that generate the object file for an assembly file.
        :type env: build_env
        :type proj: build_model.Project
        :type kind: kinds._base.Kind
        :type source_path: build_model.Path
        :type obj_path: build_model.Path
        :rtype: List[build_model.Command]
        """
        raise NotImplementedError()

    @abstractmethod
    def create_compiler_commands(self, env, proj, kind, source_path, obj_path):
        """
        Create command(s) that generate the object file for a C/C++
        file in the given project along with generating dependency
        rules for future builds.
        :type env: build_env
        :type proj: build_model.Project
        :type kind: kinds._base.Kind
        :type source_path: build_model.Path
        :type obj_path: build_model.Path
        :rtype: List[build_model.Command]
        """
        raise NotImplementedError()

    @abstractmethod
    def create_program_link_commands(self, model):
        """
        Create command(s) that generate an executable file for a
        given list of object files, static libraries, and shared libraries.
        :type model: kinds._base.LinkModel
        :rtype: List[build_model.Command]
        """
        raise NotImplementedError()

    def create_shared_link_commands(self, model):
        """
        Create command(s) that generate an shared object file for a
        given list of object files, static libraries, and shared libraries.
        :type model: kinds._base.LinkModel
        :rtype: List[build_model.Command]
        """
        raise NotImplementedError()

    def create_static_link_commands(self, model):
        """
        Create command(s) that generate a static library file for a
        given list of object files, static libraries, and shared libraries.
        :type model: kinds._base.LinkModel
        :rtype: List[build_model.Command]
        """
        raise NotImplementedError()

    def generate_extra_rules(self, env, proj):
        """
        Override if a particular kind produces other outputs
        which need their own rules.
        :type env: build_env
        :type proj: projects.Project
         :rtype: List[Rule]
        """
        return []

    def _friendly_name(self, path):
        return os.path.split(str(path))[1]

    def _variable_arg(self, proj, basename, value):
        """
        Create a variable named <project name>_<name>
        encompassing options in @value.
        :param proj: project target
        :param basename: base name of variable
        :param value: list of options
        """
        return Variable(proj.name_var() + '_' + basename, Command(value))

    def _make_response_file_as_needed(self, project, cwd, out_path, args):
        """
        Package a long list of arguments into a response file.
        The @project holds the response file as a generated text file.
        The @cwd is where the command is expected to run.
        @out_path is the build artifact used as a base for the response file.
        @args are the incoming args to bake into the response file if needed.
        :type project: projects.Project
        :type cwd: build_model.Path
        :type args: List[Node]
        :type out_path: build_model.Path
        """
        text = ""

        def expand(node_or_str):
            x = node_or_str.resolve() if isinstance(node_or_str, Node) else str(node_or_str)
            return x

        out_args = []

        full_resp_path = out_path.change_extension(".args", append=True)

        def responsify(arglist):
            # expand each element to raw text and place each on a new line
            nonlocal text
            for node in arglist:
                exp = expand(node)
                if text:
                    text += '\n'
                text += exp

        # TODO: reenable this when there's time to expand build variables
        # like CPPFLAGS, CXXFLAGS, LDFLAGS without losing that reference

        # # first, extract any troublesome arguments that `make` on Windows
        # # will mangle all to ?&!*&#$
        # i = 0
        # while i < len(args):
        #     exp = expand(args[i])
        #     if '"' in exp:
        #         responsify([exp])
        #         args[i:i+1] = []
        #     else:
        #         i += 1

        # then take any very long sequence of args into a response file
        # -- DTOOLS-1708 involves bugs in MinGW make/sh which encounter
        # a bug around char 16000, so stop well short of that
        chunks = split_by_length(args, max_cmd_line=10000)

        # was anything response-able because of weird arguments yet,
        # or is the command line already known to be too long?
        if not text and len(chunks) == 1:
            return False

        # use the response file (full path since we don't know what the caller uses as cwd)
        out_args.append(Composition("@", full_resp_path))

        # break up chunks into the response file
        for chunk in chunks:
            responsify(chunk)

        args[:] = out_args

        project.generated_files[full_resp_path] = text
        return True


def version_compare(a, b):
    a_version_bits = a.split(r'.')
    b_version_bits = b.split(r'.')

    # compare major/minor/build by element
    for index in range(min(len(a_version_bits), len(b_version_bits))):
        try:
            diff = int(a_version_bits[index]) - int(b_version_bits[index])
            if diff < 0:
                return -1
            if diff > 0:
                return 1
        except ValueError:
            break

    # fallthrough -- if all was equal (or nonnumeric), just compare strings
    if a < b:
        return -1
    if a > b:
        return 1
    return 0


Toolchain.init_class()


