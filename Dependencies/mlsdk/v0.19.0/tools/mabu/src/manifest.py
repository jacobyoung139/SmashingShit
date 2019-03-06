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
import xml.parsers.expat

import re

import diags
from run_vars import runtime_values, MLSDK
from build_model import BuildException
from config import DATA_DIR, SCRIPT_DIR
from diags import warn


class Component(object):
    def __init__(self, name, binary_file):
        self._name = name
        self._binary_file = binary_file

    @property
    def name(self):
        return self._name

    @property
    def binary_file(self):
        return self._binary_file


global_known_privileges = []


class Manifest(object):

    def __init__(self):
        self._path = None
        self._package_name = None
        self._components = []
        # :type: List[Component]
        self._developer_id = None

        self._min_api_level = None

        self._privileges = []

    @property
    def path(self):
        return self._path

    def load(self, path):
        self._path = path
        self._privileges.clear()

        with open(path, 'rb') as f:
            p = xml.parsers.expat.ParserCreate()

            el_stack = []

            def start_handler(name, attrs):
                if len(el_stack) == 0:
                    if name != 'manifest':
                        raise BuildException("unexpected manifest format (expected <manifest> root) in {}".format(path))
                    self._package_name = attrs.get('ml:package')
                    self._developer_id = attrs.get('ml:developer_id')
                elif len(el_stack) == 1 and name == 'application':
                    self._min_api_level = attrs.get('ml:min_api_level')
                elif len(el_stack) == 2 and name == 'component':
                    comp = Component(attrs.get('ml:name'), attrs.get('ml:binary_name'))
                    self._components.append(comp)
                elif len(el_stack) == 2 and name == 'uses-privilege':
                    # check below
                    priv = attrs.get('ml:name')
                    if priv:
                        self._privileges.append(priv)

                el_stack.append(name)

            def end_handler(_):
                el_stack.pop()

            p.StartElementHandler = start_handler
            p.EndElementHandler = end_handler

            try:
                p.ParseFile(f)
            except xml.parsers.expat.ExpatError as e:
                raise BuildException("failed to parse " + path) from e

        if not self._package_name:
            warn('Expected a <manifest ml:package="..."> attribute in {0}'.format(path))

    def exists(self):
        return os.path.exists(self._path)

    @property
    def package_name(self):
        return self._package_name

    @property
    def components(self):
        return self._components

    @property
    def developer_id(self):
        return self._developer_id

    @property
    def min_api_level(self):
        return self._min_api_level

    def validate_schema(self, verbose=True, strict=True):
        """
        Validate the manifest according to the schame and report a list of warnings and errors.
        This can run validation twice: once in a "lax" mode that supports forward
        compatibility by allowing unknown attributes and elements, then,
        if no errors are found, again in a "strict" mode that complains about
        any unrecognized attributes and elements, and reports those as warnings.

        :return: warnings and errors
        :rtype: (list[str],list[str])
        """
        warnings = []
        errors = []

        warnings += self._validate_privileges()

        root = os.path.join(DATA_DIR, "device")

        try:
            import xmlschema

            with open(os.path.join(root, "manifest-lax.xsd"), 'r') as f:
                schema_lax = xmlschema.XMLSchema(f)
            with open(os.path.join(root, "manifest.xsd"), 'r') as f:
                schema_strict = xmlschema.XMLSchema(f)

        except ImportError as e:
            diags.error("installation problem, cannot validate manifest: " + str(e), die=False)
            return warnings, errors

        except IOError as e:
            diags.error("failed to locate manifest schema; cannot validate manifest: " + str(e), die=False)
            return warnings, errors

        def respell(msg):
            msg = msg.replace(" a un", " an un")  # fix grammar error
            msg = msg.replace('failed', 'issues')  # "failed validating" -> "issues validating"
            return msg

        def massage(exc, verbose):
            if not verbose:
                msg = getattr(exc, 'reason')
                if msg:
                    return [msg]

            # not an XMLSchemaChildrenValidationError?
            msg = str(exc)
            msg = respell(msg)

            if not verbose:
                # only show the low-level reason
                msg_lines = msg.split('\n')
                reasons = [msg for msg in msg_lines if "Reason: " in msg]
                if reasons:
                    return [reason.replace("Reason: ", "") for reason in reasons]

            # fall through
            return [msg]

        if strict:
            # must match exactly
            with open(self.path, 'rb') as f:
                for error in schema_strict.iter_errors(f):
                    errors += massage(error, verbose)

        else:
            # lax validation: make sure required stuff is present...
            with open(self.path, 'rb') as f:
                for error in schema_lax.iter_errors(f):
                    errors += massage(error, verbose)

            if not errors:
                # ... and if that passes, just warn about unexpected content
                # found by the strict scan
                with open(self.path, 'rb') as f:
                    for error in schema_strict.iter_errors(f):
                        warnings += massage(error, False)

        return warnings, errors

    def _validate_privileges(self):
        global global_known_privileges
        if not global_known_privileges:
            privs_dat = os.path.join(DATA_DIR, 'device/privileges.dat')
            try:
                with open(privs_dat, 'rb') as f:
                    import pickle
                    global_known_privileges[:] = pickle.load(f)

            except IOError as e:
                diags.warn("cannot open '{}': <uses-privilege> elements cannot be verified\n{}".format(privs_dat, e))
                return []

        messages = []
        for priv in self._privileges:
            if priv not in global_known_privileges:
                messages.append('unknown privilege in <uses-privilege ml:name="{}">'.format(priv))
        return messages
