#
# This provides prebuild rules, which run before a project is built
# (whether or not anything changed).
#
# All the rules based on "prebuild" need to use the double-colon form.
#

# $(INFO) may be used to echo unless "-q" is passed, and $(ECHO) used for verbose messages when "-v" is passed.

# This runs at the top level before any other projects in the build tree.
prebuild::
	$(ECHO) "Running at prebuild..."

# Specific projects can have prebuild/postbuild steps, which run immediately
# before the project is built.  The rule name is the .mabu base filename with
# any spaces converted to underscores, followed by "-prebuild".
shared-prebuild:: 
	$(INFO) "Prebuilding for 'shared'"

# This can be used to generate a file before anything in the build needs it.
# The .mabu base name (with any spaces converted to '_') is used in the _BASE variable's name.
#
# (program_OUTPUT may also be used, as in postbuilds)
#
prebuild:: $(program_BASE)/generated/gen_program_header.h

$(program_BASE)/generated/gen_program_header.h:
	@echo "#define PROGRAM_MACRO 123" > "$@"


