# Note: This script should not be run, just sourced into an
# shell ("source envsetup.sh" or ". ./envsetup.sh").
# it has no effect when running it ("./envsetup.sh").

if [ -n "$BASH" ]; then
  [[ "${BASH_SOURCE[0]}" == "${0}" ]] && echo "This script should only be sourced, not executed!"

  mydir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
  if [ -f "$mydir/internal/tools/mldb/mldb.bash" ]; then
    . "$mydir/internal/tools/mldb/mldb.bash"
  elif [ -f "$mydir/tools/mldb/mldb.bash" ]; then
    . "$mydir/tools/mldb/mldb.bash"
  fi
else
  # Handle other bourne (sh) compatible shells. E.g. dash, posh, zsh
  # and others.
  # Note: This script does not support csh/tcsh and other types of shells.
  mydir="$( cd "$( dirname "${0}" )" && pwd )"
fi

export PATH=$mydir:$mydir/internal/tools/mldb:$mydir/tools/mldb:$PATH

