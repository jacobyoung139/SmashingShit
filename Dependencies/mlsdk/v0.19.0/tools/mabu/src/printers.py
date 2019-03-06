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

import hashlib
import json
import os
import re
import stat
import subprocess
import sys

import build_env
import diags
import platforms
import project_vars
from build_model import Variable, Command, VariableDefinition, Path, Node
from config import HOSTOS, OS_WIN, OS_OSX, OS_LINUX
from makegen import MakeGen
from run_vars import runtime_values, MLSDK
from utils import format_command_line, remove_duplicates


class IndexerPrefix(object):
    Define = "-D"
    IncludePath = "-I"
    Library = "-l"
    LibraryPath = "-L"
    SearchPath = "-S"


def print_tools():
    env = build_env
    toolchain = env.toolchain()
    if not toolchain:
        print("toolchain '{0}' not registered".format(env.spec().toolchain), file=sys.stderr)
        return

    err = toolchain.detect()
    if err:
        print(err, file=sys.stderr)
        # but continue...

    names = list(toolchain.tools.keys())
    names.sort()

    # TODO: print referenced variables before their uses
    # e.g. VCINSTALLDIR=...  +  CL=$(VCINSTALLDIR)\bin\cl.exe
    for name in names:
        print("{0}={1}".format(name, toolchain.tools[name]))


def print_components():
    env = build_env
    env.load_all_components()
    seen = set()
    for path, proj in env.components().items():
        print('{0:16}\t{1}{2}'.format(proj.name, path, ' (masked)' if proj.name in seen else ''))
        seen.add(proj.name)


def print_options(opts):
    env = build_env
    env.load_configs_options()
    options = list(env.options())
    options.sort()
    for opt in options:
        if not opts.verbose and (opt.startswith('platforms/') or opt.startswith('kinds/')):
            continue
        option = env.find_option(opt)
        print('{0:16}\t{1}'.format(opt, option.path))


def print_configs():
    env = build_env
    env.load_configs_options()
    configs = list(env.configs())
    configs.sort()
    for cfg in configs:
        config = env.find_config(cfg)
        print('{0:16}\t{1}'.format(cfg, config.path))


def _print_build_vars(proj_name, proj_outname, proj_ext, my_var_map):
    """
    :param proj_name: project name
    :type my_var_map: Dict[str,build_model.Node]
    """
    prefix = proj_name + '_'

    print('#', proj_outname + proj_ext)

    my_vars = [(k, v) for k, v in my_var_map.items()]
    my_vars.sort(key=lambda k: k)

    for pfx_name, val in my_vars:
        if not pfx_name.startswith(prefix):
            # raw variable
            continue

        name = pfx_name[len(prefix):]

        from build_vars import BASE, OUTPUT, SPEC
        if name in [SPEC, OUTPUT, BASE]:
            # not really a build variable
            continue

        var_def = name + '='

        class Collapser(object):
            def __init__(self):
                self.text = ''

            def __call__(self, node):
                if isinstance(node, Variable):
                    if not node.name.startswith(prefix):
                        # emit its flattened value
                        try:
                            text = node.raw_value.resolve()
                        except AttributeError:
                            text = node.raw_value.value()

                        self.text += text
                    else:
                        # a project variable -- reduce as much as we can, but
                        # not other project variables
                        inner = Collapser()
                        node.raw_value.visit(inner)
                        self.text += inner.text
                else:
                    self.text += node.makefile_str()

                return None

        collapser = Collapser()
        val.rhs().visit(collapser)
        var_def += collapser.text

        print(var_def)

    print()


def _fetch_scoped_projects(scope):
    """
    Get all the projects accessible with the given scope
    :param scope: 0=all, -1=local, 1=refs
    :return: project set
    """
    visited_project_paths = set()
    visited_projects = list()       # keep ordered!
    env = build_env

    if scope <= 0:
        for proj in env.user_projects():
            if proj.path not in visited_project_paths:
                visited_project_paths.add(proj.path)
                visited_projects.append(proj)

    if scope >= 0:
        for proj_path in env.projects():
            proj = env.project_or_package(proj_path)

            if proj in env.user_projects():
                # already handled above
                continue

            if proj.path not in visited_project_paths:
                visited_project_paths.add(proj.path)
                visited_projects.append(proj)

    return visited_projects


def print_build_vars(scope):
    """
    Print the build variables contributing to this project.
    :param scope: 0=all, -1=local, 1=refs
    """
    env = build_env
    mg = MakeGen(env.projects(), env.components(), scope)
    mg.generate(env.user_projects())
    var_map = mg.gather_vars()

    for proj in _fetch_scoped_projects(scope):
        if proj.is_package():
            # no "build" flags for packages
            continue

        my_vars = var_map[proj.name]

        _print_build_vars(proj.name, proj.output_name, proj.extension, my_vars)


def _get_indexer_config_data(proj_name, my_var_map):
    """
    :param proj_name: project name
    :type my_var_map: Dict[str,build_model.Node]
    :rtype: Set[str]
    """
    prefix = proj_name + '_'

    my_vars = [(k, v) for k, v in my_var_map.items()]
    my_vars.sort(key=lambda k: k)

    out_vars = []
    stdlib = None
    cstd = None
    cxxstd = None

    for pfx_name, val in my_vars:
        if not pfx_name.startswith(prefix):
            # raw variable
            continue

        name = pfx_name[len(prefix):]

        from build_vars import BASE, OUTPUT, SPEC
        if name in [SPEC, OUTPUT, BASE]:
            # not really a build variable
            continue

        class Collapser(object):
            def __init__(self):
                self.vars = []
                self.stdlib = None
                self.cstd = None
                self.cxxstd = None

            def __call__(self, node):
                if isinstance(node, Variable):
                    if node.name.startswith(prefix):
                        # a project variable -- reduce as much as we can, but
                        # not other project variables
                        inner = Collapser()
                        node.raw_value.visit(inner)
                        self.vars.extend(inner.vars)
                        self.stdlib = inner.stdlib
                        self.cstd = inner.cstd
                        self.cxxstd = inner.cxxstd
                if isinstance(node, Command):
                    for arg in node.args:
                        if arg.startswith("-stdlib"):
                            self.stdlib = arg
                        elif arg.startswith("-std"):
                            if "c++" in arg:
                                self.cxxstd = arg
                            else:
                                self.cstd = arg
                        elif os.path.splitext(arg)[1] == ".lib":
                            self._add_var(IndexerPrefix.Library, arg[:-4])
                        elif arg.startswith("-L"):
                            path = self._resolve_path(arg[2:])
                            self._add_var(IndexerPrefix.LibraryPath, path)
                        elif arg.startswith("-libpath:"):
                            path = self._resolve_path(arg[9:])
                            self._add_var(IndexerPrefix.LibraryPath, path)
                        elif arg.startswith("-I"):
                            path = self._resolve_path(arg[2:])
                            self._add_var(IndexerPrefix.IncludePath, path)
                        elif arg.startswith("-l"):
                            self._add_var(IndexerPrefix.Library, arg[2:])
                        elif arg.startswith("-D"):
                            self._add_var(IndexerPrefix.Define, arg[2:])
                        elif arg.startswith("--sysroot="):
                            path = self._resolve_path(arg[10:])
                            self._add_var(IndexerPrefix.IncludePath, os.path.join(path, "usr", "include"))
                            self._add_var(IndexerPrefix.LibraryPath, os.path.join(path, "usr", "lib"))

                return None

            def _add_var(self, var_prefix, var):
                self.vars.append("{}{}".format(var_prefix, var))

            def _resolve_path(self, path):
                path_segments = path.split("/")
                resolved_segments = []
                for segment in path_segments:
                    # Look for variables in the path segment.
                    if segment.startswith("$("):
                        segment = segment[2:-1]
                        if segment in my_var_map:
                            segment = self._resolve_variable(my_var_map[segment])
                            resolved_segments.append(segment)
                    else:
                        resolved_segments.append(segment)
                return os.path.normpath(os.path.join(*resolved_segments))

            def _resolve_variable(self, var):
                # Variable definitions should only resolve the right-hand side.
                if isinstance(var, VariableDefinition):
                    var = var.rhs().resolve()
                else:
                    try:
                        # It's possibly a variable, so try to resolve it.
                        var = var.resolve()
                    except:
                        # If all else fails, just use the __str__ of whatever object this is.
                        var = str(var)
                return var

        collapser = Collapser()
        val.rhs().visit(collapser)
        out_vars.extend(collapser.vars)
        if collapser.stdlib:
            stdlib = collapser.stdlib
        if collapser.cstd:
            cstd = collapser.cstd
        if collapser.cxxstd:
            cxxstd = collapser.cxxstd

    return out_vars, stdlib, cstd, cxxstd


def print_indexer_config(scope):
    """
    Print the indexer config variables contributing to this project.
    :param scope: 0=all, -1=local, 1=refs
    """
    env = build_env
    mg = MakeGen(env.projects(), env.components(), scope)
    mg.generate(env.user_projects())
    var_map = mg.gather_vars()

    # Make sure MLSDK exists in the project variable maps since it's needed for constructing some paths in device builds.
    from run_vars import runtime_values, MLSDK
    for proj_name, proj_var_map in var_map.items():
        if proj_name and MLSDK not in proj_var_map:
            proj_var_map[MLSDK] = VariableDefinition(MLSDK, "=", Variable(MLSDK, runtime_values[MLSDK]))

    class LanguageInfo(object):
        def __init__(self):
            self.proj_with_cpp = set()
            self.stdlib = None
            self.cstd = None
            self.cxxstd = None

    out_vars = {}
    language_info = LanguageInfo()

    toolchain = env.toolchain()

    for proj in _fetch_scoped_projects(scope):
        if proj.is_package():
            # no "build" flags for packages
            continue

        my_vars = var_map[proj.name]

        if proj.name not in out_vars:
            out_vars[proj.name] = []

        my_vars, language_info.stdlib, language_info.cstd, language_info.cxxstd = _get_indexer_config_data(proj.name, my_vars)
        out_vars[proj.name] += my_vars

        # Check the source files for c++ files.
        for src_proj, src_file in proj.all_source_files():
            if proj.is_cxx_file(src_file):
                language_info.proj_with_cpp.add(proj.name)

    class ImplicitHandler(object):
        def __init__(self, toolchain):
            self.toolchain = toolchain
            self.cached_vars = {}
            self.search_path_vars = ["{}{}".format(IndexerPrefix.SearchPath, x) for x in self.toolchain.command_search_paths]

        def get_vars(self, proj_name, lang_info):
            # MSVC does not support retrieval of built-in defines or include paths.
            if "msvc" in self.toolchain.name:
                return []

            implicit_vars = []

            # Find the path to the compiler.
            compiler_tool = os.path.abspath(self.toolchain.tools["CC"])
            if HOSTOS == OS_WIN and not os.path.splitext(compiler_tool)[1]:
                tool_path_exe = "{}.exe".format(compiler_tool)
                if os.access(tool_path_exe, os.F_OK):
                    compiler_tool = tool_path_exe
                else:
                    tool_path_cmd = "{}.cmd".format(compiler_tool)
                    if os.access(tool_path_cmd, os.F_OK):
                        compiler_tool = tool_path_cmd
                    else:
                        compiler_tool = None

            if compiler_tool:
                # Figure out the command arguments relating to language.
                is_cxx_proj = proj_name in lang_info.proj_with_cpp
                lang = "c++" if is_cxx_proj else "c"
                lang_std = lang_info.cxxstd if is_cxx_proj else lang_info.cstd
                stdlib = lang_info.stdlib if is_cxx_proj and "clang" in os.path.basename(compiler_tool) else None

                null_path = "NUL" if HOSTOS == OS_WIN else "/dev/null"

                # Construct the command string.
                cmd = [compiler_tool, "-dM", "-E", "-x", lang, lang_std, stdlib, "-v", "-", "<", null_path]
                cmd = format_command_line(cmd)

                # If this command string has already been run, its results will be cached
                # and we can return that instead.
                if cmd in self.cached_vars:
                    return self.cached_vars[cmd]

                fd = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                output, _ = fd.communicate()
                output = output.decode("utf-8")

                is_parsing_includes = False

                define_regex = re.compile(r"#define\s+(\w+)\s*([\w\'\"\.\s\-\+\(\)]*)", 0)
                for line in output.splitlines():
                    if not is_parsing_includes:
                        if line.startswith("#include <"):
                            is_parsing_includes = True
                        else:
                            # Watch for defines.
                            matches = define_regex.match(line)

                            if matches:
                                key = matches.group(1)
                                value = matches.group(2)
                                define = "{}={}".format(key, value) if value else key
                                implicit_vars.append("{}{}".format(IndexerPrefix.Define, define))
                    else:
                        if line == "End of search list.":
                            is_parsing_includes = False
                        else:
                            # Watch for include paths, but discard any reported as framework directories
                            # since those aren't real include paths.
                            if "(framework directory)" not in line:
                                include_path = os.path.normpath(line.strip())
                                implicit_vars.append("{}{}".format(IndexerPrefix.IncludePath, include_path))

                # Add the search path vars to the overall list of implicit vars.
                implicit_vars.extend(self.search_path_vars)

                # Cache the constructed list of implicit vars.
                self.cached_vars[cmd] = implicit_vars

                return implicit_vars

            # No implicit vars available.
            return []

    implicit_handler = ImplicitHandler(toolchain)

    for proj_name, proj_out_vars in out_vars.items():
        implicit_vars = implicit_handler.get_vars(proj_name, language_info)

        # Eliminate duplicate entries.
        proj_out_vars = sorted(set(proj_out_vars + implicit_vars))

        print("[{}]".format(proj_name))

        # Print each entry to stdout.
        for var in proj_out_vars:
            print(var)


def filter_matching_paths_preceding(old_paths, new_paths, tester, path_compare):
    """
    Remove entries from @new_paths for which @tester returns True and
    @path_compare considers the entry to match one in @old_paths,
    as long as the new path is found in @old_paths _after_ such a match.

    :param tester: functor to tell if a path is "important"
    :param path_compare: functor to compare two paths (or entries therein) for equality
    """

    # find candidate
    candidate = None
    for new_path in new_paths:
        if tester(new_path):
            candidate = new_path
            break

    if not candidate:
        # no matches, nothing to worry about
        return None, None

    match = None
    for old_path in old_paths:
        if tester(old_path):
            if not path_compare(old_path, candidate):
                # ok, different entry, and it came in the same order -- all is well
                if not match:
                    match = old_path
            else:
                # else, old_paths wants to override
                new_paths.remove(candidate)
                return candidate, match

    # no override or conflict detected
    return None, None


def _filter_runtime_library_paths(opts, lib_path_var, env_lib_paths, lib_paths):
    """
    Avoid promoting the MLSDK/lib/<os> library ahead of existing
    entries on the OS shared library path, if such an entry
    already exists.  This ensures that ML Remote will be able
    to override those libraries (assuming mlvdsetup was run).
    """
    env_lib_paths = remove_duplicates(
        [os.path.abspath(path) for path in env_lib_paths])

    # see if we find a project link library path that may conflict
    mlremote_lib = build_env.platform().format_shared_file('ml_remote')

    removed, match = filter_matching_paths_preceding(
        env_lib_paths, lib_paths,
        lambda path: os.path.isfile(os.path.join(path, mlremote_lib)),
        lambda a, b: os.path.samefile(a, b))

    if removed and opts.verbose:
        if match:
            diags.info("ignoring LIBPATHS entry '{}' in "
                       "favor of overriding entry in {}: '{}'"
                       .format(removed, lib_path_var, match))
        else:
            diags.info("ignoring LIBPATHS entry '{}' since it already appears in {}"
                       .format(removed, lib_path_var))


def print_invoke_commands(opts):
    """
    Create scripts to invoke a process from a .mabu or .package file,
    either for the device or the host.
    """
    env = build_env

    mg = MakeGen(env.projects(), env.components(), opts.build_scope)
    mg.generate(env.user_projects())

    def get_commands(package, proj, is_device_build, invoke_args):
        """
        :type  proj: projects.Project
        """
        source_lib_path_var = lib_path_var = "DYLD_LIBRARY_PATH" if HOSTOS == OS_OSX else \
            "LD_LIBRARY_PATH" if HOSTOS == OS_LINUX else \
            "PATH"

        if HOSTOS == OS_OSX:
            # favor ML_LIBRARY_PATH over an empty/missing DYLD_LIBRARY_PATH,
            # which macOS deletes for our ever-loving security
            if not os.getenv('DYLD_LIBRARY_PATH') and os.getenv('ML_LIBRARY_PATH'):
                diags.info("macOS deleted $DYLD_LIBRARY_PATH, using $ML_LIBRARY_PATH instead")
                source_lib_path_var = 'ML_LIBRARY_PATH'

        lib_paths = proj.combined.get(project_vars.LIBPATHS, []) if proj.combined else []
        filename = "invoke-{}".format(package.package_name if package else proj.name)
        out_dir = os.path.normpath(proj.output_directory)

        dfs = proj.dependency_list(env.projects(), env.components())

        ref_paths = [Path(dep_proj.output_directory) for dep_proj in dfs if dep_proj.is_program() or dep_proj.is_shared()]
        lib_paths += ref_paths

        if lib_paths:
            # Resolve all the lib paths and eliminate any duplicate paths.
            lib_paths = remove_duplicates([os.path.abspath(path.resolve()) for path in lib_paths])

            if not is_device_build and not opts.no_mlremote_adjust:
                env_lib_paths = os.getenv(source_lib_path_var, '').split(os.pathsep)

                _filter_runtime_library_paths(opts, source_lib_path_var, env_lib_paths, lib_paths)

                if source_lib_path_var != lib_path_var:
                    # since macOS deletes the env var, we need to explicitly copy
                    # environment paths back into the full list for the invoke script
                    lib_paths += env_lib_paths

        cmds = []

        executable_name = build_env.platform().format_program_file(proj.output_name)

        if is_device_build:
            package_filename = "{}.mpk".format(proj.package_name)
            installed_package_name = proj.manifest.package_name
            envsetup_basename_path = os.path.abspath(os.path.join(runtime_values[MLSDK], "envsetup"))
            device_install_cmd = "mldb install -u \"{}\"".format(os.path.join(out_dir, package_filename))
            device_launch_cmd = "mldb launch -f {}".format(installed_package_name)
            launch_directory = "."
            executable_path = executable_name
        else:
            envsetup_basename_path = ""
            device_install_cmd = ""
            device_launch_cmd = ""
            launch_directory = package.package_layout_path.resolve() if package else out_dir
            executable_path = os.path.join(out_dir, executable_name)

        if HOSTOS == OS_WIN:
            # Create batch commands for a Windows host.
            filename += ".bat"

            cmds.extend([
                "@echo off",
                "setlocal",
                "pushd \"{}\"".format(launch_directory),
            ])

            if is_device_build:
                cmds.extend([
                    "call \"{}.bat\"".format(envsetup_basename_path),
                    device_install_cmd,
                    "set __RETCODE__=%ERRORLEVEL%",
                    "if %__RETCODE__% EQU 0 (",
                    "\t{}".format(device_launch_cmd),
                    "\tset __RETCODE__=%ERRORLEVEL%",
                    ")",
                ])
            else:
                if lib_paths:
                    # no quotes in PATH var
                    new_path_var = ";".join(lib_paths + ["%PATH%"])
                    cmds.append("PATH={}".format(new_path_var))

                cmds.extend([
                    format_command_line([executable_path] + (invoke_args or [])),
                    "set __RETCODE__=%ERRORLEVEL%",
                ])

            cmds.extend([
                "popd",
                "exit /b %__RETCODE__%",
            ])
        else:
            # Create bash command for Unix-like hosts.
            filename += ".sh"

            cmds.extend([
                "#!/usr/bin/env bash",
                "__OLDPWD__=$PWD",
                "cd \"{}\"".format(launch_directory),
            ])

            if is_device_build:
                cmds.extend([
                    "source \"{}.sh\"".format(envsetup_basename_path),
                    device_install_cmd,
                    "__RETCODE__=$?",
                    "if [ $__RETCODE__ -eq 0 ]; then",
                    "\t{}".format(device_launch_cmd),
                    "\t__RETCODE__=$?",
                    "fi",
                ])
            else:
                exe_cmd = [executable_path] + (invoke_args or [])

                if lib_paths:
                    # add quotes around the entire definition, not each path, and ensure the
                    # quote follows "=" so format_command_line() won't bork it
                    exe_cmd.insert(0, '{0}="{1}:${0}"'.format(lib_path_var, os.pathsep.join(lib_paths)))

                cmds.extend([
                    format_command_line(exe_cmd),
                    "__RETCODE__=$?",
                ])

            cmds.extend([
                "cd $__OLDPWD__",
                "exit $__RETCODE__",
            ])

        return filename, cmds + [""]

    def handle_project(package, proj, is_device_build):
        is_device_program_build = is_device_build and proj.is_program()
        is_non_device_package_build = not is_device_build and proj.is_package()
        is_non_program_and_non_package = not proj.is_program() and not proj.is_package()

        if is_device_program_build or is_non_device_package_build or is_non_program_and_non_package:
            # Ignore builds that cannot be invoked.
            return None

        # Create the output directory if it doesn't exist.
        if not os.access(proj.output_directory, os.F_OK):
            os.makedirs(proj.output_directory)

        filename, cmds = get_commands(package, proj, is_device_build, opts.invoke_args)
        out_file_path = os.path.normpath(os.path.join(proj.output_directory, filename))

        with open(out_file_path, "w") as f:
            f.write("\n".join(cmds))

        # Get the invoke file permissions.
        st = os.stat(out_file_path)

        # Make the invoke file executable.
        os.chmod(out_file_path, st.st_mode | stat.S_IEXEC)

        if not opts.quiet:
            diags.info("invoke file written to: {}".format(out_file_path))
        return out_file_path

    platform = build_env.platform()
    is_device_build = (platform.name == platforms.lumin.NAME)

    invoke_file_paths = []

    # track the current package
    # (we will see, for a .package that REFS .mabu,
    # first the .package, then the .mabu)
    package = None

    # Find any invokable projects.
    for proj in _fetch_scoped_projects(0):
        out_file_path = handle_project(package, proj, is_device_build)
        if out_file_path:
            invoke_file_paths.append(out_file_path)
        if proj.is_package():
            package = proj

    if not invoke_file_paths:
        diags.error("no invokable projects found.")

    return invoke_file_paths


def _get_package_mappings_for_output(force_package, scope):
    """
    :param scope: 0 for all, -1 for local, 1 for refs
    :return: dict[str,dict[str, str]]
    """
    env = build_env

    # ensure we know the ins and outs of the projects
    mg = MakeGen(env.projects(), env.components(), scope)
    mg.generate(env.user_projects())

    maps = {}
    for proj in env.user_projects():
        build_mapping, data_mapping, errors = proj.get_package_mappings(force_package, scope)
        # just print, not fatal
        for err in errors:
            diags.error(err, die=False, file=proj.path)

        if proj.is_package() and scope <= 0 and (env.platform().uses_packages() or force_package):
            # add the generated package to the outputs
            pkg_file = proj.package_archive_path
            build_mapping.append((pkg_file.resolve(), pkg_file.filename.resolve()))

        build_mapping += data_mapping

        maps[proj.output_name + proj.extension] = build_mapping

    return maps


def print_outputs(force_package, scope):
    proj_to_mappings = _get_package_mappings_for_output(force_package, scope)
    for proj, mappings in proj_to_mappings.items():
        print("# " + proj)
        for file, package_file in mappings:
            if package_file:
                print(file + "\t" + package_file)
            else:
                print(file + "\t# not packaged")


def print_output_directories(scope):
    """
    Print the directories where each project generates its output.
    :param scope: 0=all, -1=local, 1=refs
    """
    env = build_env
    mg = MakeGen(env.projects(), env.components(), scope)
    mg.generate(env.user_projects())

    visited_projects = _fetch_scoped_projects(scope)

    for proj in sorted(visited_projects, key=lambda p: p.name):
        if proj.is_package():
            # no "output directory" for a project
            continue

        print("{}\t{}".format(proj.path, proj.output_directory))


def print_layout_directories(scope):
    """
    Print the layout directories for each package
    """
    env = build_env

    # needed to ensure we parsed and found OUT, but don't need to recurse
    mg = MakeGen(env.projects(), env.components(), scope)
    mg.generate(env.user_projects())

    visited_projects = _fetch_scoped_projects(scope)

    for proj in sorted(visited_projects, key=lambda p: p.name):
        if proj.is_package():
            print("{}\t{}".format(proj.path, proj.package_layout_path.resolve()))


def print_package_outputs(scope):
    """
    Print the .mpk paths for each package
    """
    env = build_env

    # needed to ensure we parsed and found OUT/manifest/etc, but don't need to recurse
    mg = MakeGen(env.projects(), env.components(), scope)
    mg.generate(env.user_projects())

    visited_projects = _fetch_scoped_projects(scope)

    for proj in sorted(visited_projects, key=lambda p: p.name):
        if proj.is_package():
            print("{}\t{}".format(proj.path, proj.package_archive_path.resolve()))


def print_project_files(scope):
    """
    Print all the input files contributing to this project.
    :param scope: 0=all, -1=local, 1=refs
    """
    env = build_env

    mg = MakeGen(env.projects(), env.components(), scope)
    mg.generate(env.user_projects())

    def getReferencePaths(project):
        return [x.path for x in project.dependency_list(env.projects(), None)]

    def getComponentPaths(project):
        return [x.path for x in project.dependency_list(None, env.components())]

    top_level_projects = []
    all_projects = set()
    all_components = set()

    if scope <= 0:
        for proj in env.user_projects():
            top_level_projects.append(proj)

    if scope >= 0:
        for proj_path in env.projects():
            proj = env.project_or_package(proj_path)
            top_level_projects.append(proj)

    # Collect a complete listing of each project, component, option, and config involved in the build.
    for proj in top_level_projects:
        proj_references = getReferencePaths(proj)
        proj_components = getComponentPaths(proj)

        all_projects.add(proj.path)
        all_projects.update(set(proj_references))
        all_components.update(set(proj_components))

    all_projects = [env.project_or_package(x) for x in sorted(all_projects)]
    all_components = [env.components()[x] for x in sorted(all_components)]

    for proj in all_projects + all_components:
        direct_deps = sorted(proj.refs + proj.uses + proj.direct_options + proj.direct_configs)

        if direct_deps:
            print("# {} file(s) read from {}".format(len(direct_deps), proj.path))

            for sub_proj_path in direct_deps:
                print(sub_proj_path)

            print("")


def print_resolved_projects(projs):
    """
    Print the build settings in the resolved projects
    :param projs: the Projects
    :type projs: list[projects.Project]
    """
    env = build_env

    # ensure all settings resolved
    for proj in env.projects().values():
        proj.resolve_settings(env.config())
    for proj in env.packages().values():
        proj.resolve_settings(env.config())

    # these are the vars we want to emit, in
    # the order we want to display them
    emit_vars = list(project_vars.ALL_VARS)
    emit_vars.remove(project_vars.REFS)
    emit_vars.remove(project_vars.USES)

    for name in projs:
        proj = env.project_or_package(name)

        print("#",proj.name,"at",proj.path)

        def quote(v):
            if isinstance(v, Node):
                s = v.makefile_str()
            else:
                s = str(v)
            if ' ' in s:
                s = '"' + s + '"'
            return s

        def quote_path(proj_path):
            if proj_path[0]:
                return quote(Path(proj_path[0].base_path(), proj_path[1]))
            else:
                return quote(proj_path[1])

        for key in emit_vars:
            val = proj.combined[key]
            if not val:
                continue

            s = ''
            if key == project_vars.DATAS:
                for f, t in val:
                    if s:
                        s += ' \\\n\t'
                    if str(f) != str(t):
                        s += quote(f) + ' : ' + quote(t)
                    else:
                        s += quote(f)
            elif key == project_vars.KIND:
                s = val
            elif val and isinstance(val[0], tuple):
                # project+path list
                s += ' \\\n\t'.join(quote_path(v) for v in val)
            else:
                s += ' \\\n\t'.join(quote(v) for v in val)
            print(key,'=',s)

        print()


def print_signing_data(opts):
    from package import fetch_signing_data
    from package import print_signing_data

    in_mpk = opts.print_signing_data
    main_taildata, entry_taildata = fetch_signing_data(opts, in_mpk)
    print_signing_data(in_mpk, main_taildata, opts.verbose)

    for path, taildata in entry_taildata.items():
        print_signing_data(path, taildata, opts.verbose)