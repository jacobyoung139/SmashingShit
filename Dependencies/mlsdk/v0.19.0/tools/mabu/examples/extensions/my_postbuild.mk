#
# This provides postbuild rules, which happen each time a 
# project is built (whether or not anything changed).
#
# All the rules based on "postbuild" need to use the double-colon form.
#

# $(INFO) may be used to echo unless "-q" is passed, and $(ECHO) used for verbose messages when "-v" is passed.

# This adds a postbuild step, which dumps the sizes of the "program" executable.
# The .mabu base name (with any spaces converted to '_') is used in the _BASE variable's name.
#
# (program_BASE may also be used, as in prebuilds)
#
postbuild:: $(program_OUTPUT)/$(PROGRAM_PREFIX)program$(PROGRAM_EXT)
	$(INFO) Postbuild step
ifneq ($(HOST), win64)
	size "$<" > "$<.txt"
else
	dumpbin -summary "$<" > "$<.txt"
endif

# Specific projects can have prebuild/postbuild steps, which run immediately
# after the named project is built.  The rule name is the .mabu base filename with
# any spaces converted to underscores, followed by "-postbuild".
shared-postbuild:: 
	@echo "Build complete for 'shared'"


