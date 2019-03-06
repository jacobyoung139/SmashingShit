
# This invokes flex to create a lexer file.
#
# Useful variables are:
#
#   PROJECT (basename of current project)
#     $(PROJECT)_BASE (base directory of this project)
#     $(PROJECT)_OUTPUT (output directory for this build target)
#   ECHO (precede commands with this, for '-v')
#   INFO (precede commands with this to echo build status)
#   SPEC (full build specification)
#
# note that $($(PROJECT)_BASE) should be included to avoid
# conflicting with similar patterns from other projects.

FLEX=flex

$($(PROJECT)_BASE)/generated/%.c $($(PROJECT)_BASE)/generated/%.h: $($(PROJECT)_BASE)/%.l
	$(INFO) Running flex...
	$(FLEX) -CeF --outfile="$@" -v "$<"

