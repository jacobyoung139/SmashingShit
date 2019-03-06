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

import json
import os
import platform
import subprocess
import sys

import architectures
import build_env
import build_vars
import platforms
import project_vars
from build_model import Path, Composition, Command, InfoCommand, Buildable, BuildException, \
    Variable, Rule
from config import TOOLS_DIR, HOSTOS, OS_WIN
from diags import error, warn
from run_vars import COMPILER_PREFIX, LINKER_PREFIX
from toolchains._base import Toolchain

from abc import abstractmethod

MSVC_TOOLCHAIN_CLASS = None
MSVC_TOOLCHAIN_NAME = None

class MSVCBase(Toolchain):
    def __init__(self, name, version):
        Toolchain.__init__(self, name, version)
        self.tool_bin_path = {}
        self.system_libs_paths = {}
        self.system_include_paths = {}

        self.current_tool_bin_path = ""
        self.current_system_lib_paths = []
        self.current_system_include_paths = []

    @staticmethod
    @abstractmethod
    def get_toolchain_name():
        pass

    @abstractmethod
    def _get_vs_env(self, build_arch, host_arch):
        pass

    def _get_host_arch(self):
        # get the PC architecture
        machine = platform.machine()
        host_arch = architectures.x64.NAME \
            if machine.lower() in ['amd64', 'x86_64', 'x64'] \
            else architectures.x86.NAME

        return host_arch

    def _get_tool_arch(self, build_arch, host_arch):
        return "{}_{}".format(build_arch, host_arch)

    def detect(self):
        """
        Detect the desired tools in the machine or environment.

        This should populate self._tools.

        This method tell whether the receiver is available
        for real builds.  Even if this method fails, however,
        the receiver may still be used, e.g. for unit tests.
        :rtype: str for error or None for detected
        """
        # Only when building for Windows should we verify that the selected version of Visual Studio is installed.
        if HOSTOS == OS_WIN and not MSVC_TOOLCHAIN_CLASS.exists():
            error("No Visual Studio installation could be found for toolchain {}".format(MSVC_TOOLCHAIN_NAME))

        host_arch = self._get_host_arch()
        build_arch = build_env.spec() and build_env.spec().arch or host_arch

        if HOSTOS == OS_WIN:
            try:
                self._locate_vc(build_arch, host_arch)
            except BuildException as e:
                return str(e)

        else:
            return "MSVC toolchain only supported on Windows"

        def override_or_default(var, path, copy=None):
            user = self.user_override(var)
            if user:
                return self.detect_version_on_path_or_env(var, path, False)
            if copy:
                return self._tools[copy]

            # canonicalize as Unix-like program path
            return path.replace("\\", "/")

        vc_bin_path = self.current_tool_bin_path

        self._tools["CC"] = override_or_default("CC", os.path.join(vc_bin_path, "cl.exe"))
        self._tools["CXX"] = override_or_default("CXX", os.path.join(vc_bin_path, "cl.exe"), "CC")
        self._tools["LD"] = override_or_default("LD", os.path.join(vc_bin_path, "link.exe"))
        self._tools["AR"] = override_or_default("AR", os.path.join(vc_bin_path, "lib.exe"))
        self._tools["AS"] = override_or_default("AS", os.path.join(vc_bin_path, "ml{}.exe".format("64" if build_arch == architectures.x64.NAME else "")))

        self._find_cpp()

        return None

    def _convert_string_to_dict(self, input_string):
        lines = input_string.replace("\r", "").split("\n")
        output = {}

        for line in lines:
            if "=" in line:
                key, value = line.split("=", 1)
                output[key] = value

        return output

    def _locate_vc(self, build_arch, host_arch):
        tool_arch = self._get_tool_arch(build_arch, host_arch)

        if tool_arch not in self.tool_bin_path or tool_arch not in self.system_libs_paths:
            vs_env = self._get_vs_env(build_arch, host_arch)

            for key, value in vs_env.items():
                # The Visual Studio environment is just an augmentation of the existing system environment at the time
                # the vcvars batch file was called, so it's safe to overwrite the system environment with the new values.
                os.environ[key] = value

            bin_path = None
            system_lib_paths = []
            system_include_paths = []

            def split_path_string_into_list(path_string):
                # Canonicalize all paths to be in a Unix-like format.
                return [path.replace("\\", "/") for path in path_string.split(";") if path]

            # The vcvars batch file prepends the "Path" environment variable with the location of the build tools,
            # so we need to search the value to see which path we need to use.
            for key, value in os.environ.items():
                key_lowered = key.lower()

                if key_lowered == "path":
                    sysPaths = value.split(";")

                    for path in sysPaths:
                        hypotheical_compiler_path = os.path.join(path, "cl.exe")

                        # Check if the hypothetical path to the compiler is actually real.
                        if os.access(hypotheical_compiler_path, os.F_OK):
                            bin_path = path
                            break

                elif key_lowered == "lib":
                    system_lib_paths.extend(split_path_string_into_list(value))

                elif key_lowered == "include":
                    system_include_paths.extend(split_path_string_into_list(value))

            # Make sure the bin path was found.
            if not bin_path:
                error("MSVC binary path not found for toolchain {}".format(self.get_toolchain_name()))

            self.tool_bin_path[tool_arch] = bin_path
            self.system_libs_paths[tool_arch] = system_lib_paths
            self.system_include_paths[tool_arch] = system_include_paths

        self.current_tool_bin_path = self.tool_bin_path[tool_arch]
        self.current_system_lib_paths = self.system_libs_paths[tool_arch]
        self.current_system_include_paths = self.system_include_paths[tool_arch]

    def get_unique_artifacts(self, proj):
        """
        Get project artifacts specific to a toolchain.
        :type proj: projects.Project
        :return: List of project artifacts for the toolchain.
        :rtype: List[build_model.Path]
        """
        return [
            Path(proj.outdir_path(), "{}.ilk".format(proj.output_name)),
            Path(proj.outdir_path(), "{}.idb".format(proj.output_name)),
            Path(proj.outdir_path(), "{}.pdb".format(proj.output_name)),
        ]

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
        user_asmflags = proj.combined[project_vars.ASMFLAGS]

        # include paths
        inc_args = [Composition("-I", inc) for inc in proj.combined[project_vars.INCS]]

        # macro definitions
        def_args = [Composition("-D", defn) for defn in proj.combined[project_vars.DEFS]]

        #
        # MSVC assembly
        #
        args = [
            self.tool_var("AS"),

            # silence!
            "-nologo",

            # The output object MUST be specified BEFORE the input source file,
            # otherwise ml will silently fail to create it!
            Composition("-Fo", obj_path),

            # make a single object
            "-c", source_path,
        ] \
        + user_asmflags \
        + inc_args \
        + def_args

        assemble = Command(args)
        assemble.winargs = True

        return [
            InfoCommand("[{0}] Assembling '{1}'...".format(proj.name, source_path.filename)),
            assemble
        ]

    def create_compiler_commands(self, env, proj, kind, source_path, obj_path):
        """
        Create command(s) that generate the object file for a C/C++
        file in the given project along with generating dependency
        rules for future builds.
        :type env: build_env
        :type proj: projects.Project
        :type kind: kinds._base.Kind
        :type source_path: build_model.Path
        :type obj_path: build_model.Path
        :return: List[build_model.Command]
        """
        #
        # Dependency generation
        #
        args = [
            Variable("PYTHON", sys.executable),
            Variable("DEPGEN", os.path.join(TOOLS_DIR, HOSTOS, "depgen.py")),

            # input
            source_path,

            # depfile output
            proj.get_dependency_output(obj_path),

            # obj file path
            obj_path,
        ]

        # add the include paths
        for i in proj.combined[project_vars.INCS]:
            args.extend(["-i", i])

        # add the macro definitions
        for d in proj.combined[project_vars.DEFS]:
            args.extend(["-d", d])

        is_cxx = proj.is_cxx_file(source_path)
        user_cppflags = proj.combined[project_vars.CPPFLAGS]
        user_cflags = proj.combined[project_vars.CFLAGS]
        user_cxxflags = proj.combined[project_vars.CXXFLAGS]

        # add includes or defines passed as raw compiler flags
        likely_cpp_flags = user_cppflags + user_cflags + (user_cxxflags if is_cxx else [])
        for arg in likely_cpp_flags:
            if arg.startswith("-I"):
                args.extend(["-i", arg[2:]])
            elif arg.startswith("-D"):
                args.extend(["-d", arg[2:]])

        depgen = Command(args)

        # include paths
        inc_args = [Composition("-I", inc) for inc in proj.combined[project_vars.INCS] + self.current_system_include_paths]

        # macro definitions
        def_args = [Composition("-D", defn) for defn in proj.combined[project_vars.DEFS]]

        #
        # MSVC compilation
        #
        args = [
            Variable(COMPILER_PREFIX, ""),
            self.tool_var("CXX" if is_cxx else "CC"),

            # silence!
            "-nologo",

            # C vs. C++ switch
            "-TP" if is_cxx else "-TC",

            # make a single object
            "-c", source_path,
            Composition("-Fo", obj_path),

            # use mspdbserver to allow access to pdb file in parallel builds
            "-FS",

            # name program database after the output, to avoid
            # crossing the streams and littering the world with
            # .pdb files
            Composition("-Fd", Path(proj.outdir_path(), os.path.basename(proj.output_name) + '.pdb')),
        ]

        # remaining user flags
        cppflags = inc_args + def_args
        cppflags += user_cppflags

        # note: we don't share CPPFLAGS directly with the 'cpp' invocation above, since
        # the user CPPFLAGS flags may indeed have msvc specifics in it
        args.append(self._variable_arg(proj, build_vars.CPPFLAGS, cppflags))

        if is_cxx:
            cxxflags = user_cxxflags
            args.append(self._variable_arg(proj, build_vars.CXXFLAGS, cxxflags))
        else:
            cflags = user_cflags
            args.append(self._variable_arg(proj, build_vars.CFLAGS, cflags))

        compile = Command(args)
        compile.winargs = True

        return [
            InfoCommand("[{0}] Compiling '{1}'...".format(proj.name, source_path.filename)),
            depgen,
            compile
        ]

    def _gather_libs(self, model, libpaths):
        """
        Create arguments to link in the given static and shared libraries.
        :rtype: List[str]
        """

        platform = build_env.platform()

        dirs = []
        libs = []

        seen_dirs = set()

        def ensure_libdir(libdir):
            resolved_dir = libdir.resolve()
            if resolved_dir not in seen_dirs:
                seen_dirs.add(resolved_dir)
                dirs.append(Composition("-libpath:", libdir))

        # add user paths
        for libpath in libpaths:
            ensure_libdir(libpath)

        for ent in model.stlibs + model.shlibs:
            ldir, lib = ent
            if ldir:
                ensure_libdir(ldir)

            lib_name = platform.format_importlib_file(lib.makefile_str())
            libs.append(lib_name)

        return dirs + libs

    def _create_executable_link_commands(self, model,
                                         type_args, label):
        proj = model.proj
        outpath = model.outpath
        outdir = proj.outdir_path()

        args = [
            Variable(LINKER_PREFIX, ""),
            self.tool_var('LD'),

            # quiet!
            "-nologo",

            # output path...
            Composition("-out:", outpath.relative_to(outdir)),

            # architecture
            '-machine:' + build_env.architecture().name,
        ]

        args += type_args

        # objects
        obj_args = [obj.relative_to(outdir) for obj in model.objs]

        # bundle excess arguments into response files (only object files for now)
        self._make_response_file_as_needed(proj, outdir, outpath, obj_args)

        args.extend(obj_args)

        # library paths
        libs = self._gather_libs(model, proj.combined[project_vars.LIBPATHS] +
                                 [Path(p) for p in self.current_system_lib_paths])

        args.append(self._variable_arg(proj, build_vars.LIBS, libs))

        # add user linker flags
        ldflags = proj.combined[project_vars.LDFLAGS]

        args.append(self._variable_arg(proj, build_vars.LDFLAGS, ldflags))

        # ensure the output file's modification date changes
        # even if the MSVC incremental linker finds no change
        # (e.g. when an object file is touched but has no structural
        # difference for the executable)

        linktouch = Variable("LINKTOUCH", os.path.join(TOOLS_DIR, HOSTOS, "linktouch.sh").replace('\\', '/'))
        args = [linktouch, outpath] + args

        link = Command(args)
        link.winargs = True
        link.cwd = outdir

        return [
            InfoCommand("[{0}] {2} '{1}'...".format(
                proj.name,
                self._friendly_name(outpath),
                label

            )),
            link
        ]

    def create_program_link_commands(self, model):
        """
        Create command(s) that generate an executable file for a
        given list of object files, static libraries, and shared libraries.
        :type model: kinds._base.LinkModel
        :rtype: List[build_model.Command]
        """
        return self._create_executable_link_commands(model, [], "Linking program")

    def create_shared_link_commands(self, model):
        """
        Create command(s) that generate an shared object file for a
        given list of object files, static libraries, and shared libraries.
        :type model: kinds._base.LinkModel
        :rtype: List[build_model.Command]
        """
        commands = self._create_executable_link_commands(model,
                                                         ["-dll"],
                                                         "Linking DLL")

        # This format requires us to use pattern rules (see 'Rule.makefile_str()')
        # but *that* then exposes a bug in make where spaces are involved in
        # paths... let's just be lazy here for now.

        # *.lib and *.exp files also come out on Windows
        # lib_path = Path(model.proj.outdir_path(), build_env.platform().format_importlib_file(model.proj.name))
        # if lib_path != model.outpath:
        #     artifact_rule.add_aux_buildable(Buildable(lib_path))

        # exp_path = Path(model.proj.outdir_path(), model.proj.output_name + '.exp')
        # artifact_rule.add_aux_buildable(Buildable(exp_path))

        return commands

    def generate_extra_rules(self, env, proj):
        """
        Override if a particular kind produces other outputs
        which need their own rules.
        :type env: build_env
        :type proj: projects.Project
         :rtype: List[Rule]
        """
        if proj.is_shared():
            # make a .lib rule that depends on DLL
            dll_path = Path(proj.outdir_path(), env.platform().format_shared_file(proj.output_name))

            lib_path = Path(proj.outdir_path(), env.platform().format_importlib_file(proj.output_name))
            if lib_path != dll_path:
                rule = Rule(proj, Buildable(lib_path), [dll_path])
                return [rule]

        return []

    def create_static_link_commands(self, model):
        """
        Create command(s) that generate a static library file for a
        given list of object files, static libraries, and shared libraries.
        :type model: kinds._base.LinkModel
        :rtype: List[build_model.Command]
        """
        proj = model.proj
        outpath = model.outpath
        outdir = proj.outdir_path()

        # LIB appends items to archives, so we need to wipe it out to
        # avoid keeping e.g. renamed objects
        del_command = Command([Variable('RM', 'rm'), '-f', outpath])

        lib_args = [
            self.tool_var("AR"),

            # quiet!
            "-nologo",

            # make a shared library
            Composition("-out:", outpath.relative_to(outdir)),
        ]

        # objects
        obj_args = [obj.relative_to(outdir) for obj in model.objs]

        # bundle excess arguments into response files (only object files for now)
        self._make_response_file_as_needed(proj, outdir, outpath, obj_args)

        args = list(obj_args)

        # NOTE: ignoring static/shared libraries for static link

        # add archiver flags
        arflags = proj.combined[project_vars.ARFLAGS]

        args.append(self._variable_arg(proj, build_vars.ARFLAGS, arflags))

        link = Command(lib_args + args)
        link.winargs = True
        link.cwd = outdir

        return [
            InfoCommand("[{0}] Linking static library '{1}'....".format(
                proj.name,
                self._friendly_name(outpath)
            )),
            del_command,
            link
        ]


class MSVC2015(MSVCBase):
    _vcvars_path = None

    def __init__(self, name):
        MSVCBase.__init__(self, name, "14.0")

    @staticmethod
    def get_toolchain_name():
        return "msvc-2015"

    @staticmethod
    def locate():
        if HOSTOS == OS_WIN:
            comnToolsPath = os.getenv("VS140COMNTOOLS")

            if comnToolsPath and os.access(comnToolsPath, os.F_OK):
                vcvarsPath = os.path.abspath(os.path.join(comnToolsPath, "..", "..", "VC"))

                if os.access(os.path.join(vcvarsPath, "vcvarsall.bat"), os.F_OK):
                    MSVC2015._vcvars_path = vcvarsPath

    @staticmethod
    def exists():
        return True if MSVC2015._vcvars_path else False

    def _get_vs_env(self, build_arch, host_arch):
        args = {
            "x64": {
                "x64": "amd64",
                "x86": "amd64_x86",
            },
            "x86": {
                "x64": "x86_amd64",
                "x86": "x86",
            },
        }

        if host_arch not in args:
            raise BuildException("Host architecture {} is not supported".format(host_arch))

        if build_arch not in args[host_arch]:
            raise BuildException("Build architecture {} is not supported".format(build_arch))

        arg_arch = args[host_arch][build_arch]

        cmd = [os.path.join(MSVC2015._vcvars_path, "vcvarsall.bat"), arg_arch, "&", "set"]
        comp = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        comp.stdout = comp.stdout.decode("utf-8")
        comp.stderr = comp.stderr.decode("utf-8")

        if comp.returncode:
            raise BuildException(comp.stdout + comp.stderr)

        return self._convert_string_to_dict(comp.stdout)


class MSVC2017(MSVCBase):
    _vcvars_path = None

    def __init__(self, name):
        MSVCBase.__init__(self, name, "15.0")

    @staticmethod
    def get_toolchain_name():
        return "msvc-2017"

    @staticmethod
    def locate():
        if HOSTOS == OS_WIN:
            vswhere_file_path = os.path.join(
                os.environ["ProgramFiles(x86)"],
                "Microsoft Visual Studio",
                "Installer",
                "vswhere.exe"
            )

            if os.access(vswhere_file_path, os.F_OK):
                cmd = [
                    vswhere_file_path,
                    "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                    "-format", "json",
                ]

                out = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

                if out and out.returncode == 0:
                    json_string = out.stdout.decode("utf-8", "replace") if out.stdout else "[]"
                    json_data = json.loads(json_string)

                    # Parse the install information.
                    for install_data in json_data:
                        version = install_data["installationVersion"]

                        # Consider all sub-versions of version 15.
                        if version.startswith("15."):
                            install_name = install_data["displayName"]
                            install_path = install_data["installationPath"]

                            # Verify the install path exists.
                            if os.access(install_path, os.F_OK):
                                common_tools_path = os.path.join(install_path, "Common7", "Tools")

                                # Verify the common tools directory exists within the install path.
                                if os.access(common_tools_path, os.F_OK):
                                    MSVC2017._vcvars_path = common_tools_path
                                    break
                                else:
                                    warn("{} does not have the required common tools path: \"{}\"", install_name, common_tools_path)

                            else:
                                warn("Found {} with invalid installation path: \"{}\"".format(install_name, install_path))

    @staticmethod
    def exists():
        return True if MSVC2017._vcvars_path else False

    def _get_vs_env(self, build_arch, host_arch):
        cmd = [
            os.path.join(MSVC2017._vcvars_path, "VsDevCmd.bat"),
            "-no_logo",
            "-arch={}".format(build_arch),
            "-host_arch={}".format(host_arch),
            "&",
            "set",
        ]
        comp = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        comp.stdout = comp.stdout.decode("utf-8", "replace")
        comp.stderr = comp.stderr.decode("utf-8", "replace")

        if comp.returncode or comp.stdout.startswith("[ERROR:"):
            raise BuildException(comp.stdout + comp.stderr)

        return self._convert_string_to_dict(comp.stdout)


MSVC2015.locate()
MSVC2017.locate()

# When running from Visual Studio or the Developer Command Prompt, the "VisualStudioVersion" environment
# variable will always be present. We can use this to encourage mabu to use the appropriate version.
# Users can also set this manually to make mabu select the right version.
version = os.getenv("VisualStudioVersion")

if version:
    versionToTupleMap = {
        "14.0": (MSVC2015, MSVC2015.get_toolchain_name()),
        "15.0": (MSVC2017, MSVC2017.get_toolchain_name()),
    }

    versionTuple = versionToTupleMap.get(version)

    if not versionTuple:
        error("Visual Studio version not supported: {}".format(version))

    MSVC_TOOLCHAIN_CLASS = versionTuple[0]
    MSVC_TOOLCHAIN_NAME = versionTuple[1]

else:
    # TODO: Enable selection of 2017; defaulting to 2015 for now to ease the transition.
    MSVC_TOOLCHAIN_CLASS = MSVC2015
    MSVC_TOOLCHAIN_NAME = MSVC_TOOLCHAIN_CLASS.get_toolchain_name()

# Sanity check: MSVC_TOOLCHAIN_CLASS and MSVC_TOOLCHAIN_NAME should always be defined.
if not MSVC_TOOLCHAIN_CLASS or not MSVC_TOOLCHAIN_NAME:
    error("No MSVC toolchain selected. This should never happen, please log a bug against it.")

Toolchain.register(MSVC_TOOLCHAIN_CLASS(MSVC_TOOLCHAIN_NAME))
Toolchain.register_alias("msvc", MSVC_TOOLCHAIN_NAME)
