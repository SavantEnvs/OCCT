#!/usr/bin/env bash
# CMAKE_CXX_COMPILER_LAUNCHER for mayhem/build.sh: runs the compiler unchanged (by name, through PATH),
# except that it turns ONE UBSan check off for each of the few upstream files below, which trip it on
# benign code for every well-formed input, so a halting build would abort before any reader logic ran.
# The extra flag is on the compile line, so it is part of ccache's key. Every check stays on for all
# other files.
#  - LDOM_BasicElement.cxx:257 computes `1 << (hash & 63)` on an int for every XML attribute
#    (shift-exponent).
#  - XmlObjMgt.cxx:72 static_casts an LDOM_Node to `const LDOM_Text&` for every element text value
#    (object-size: "downcast ... with insufficient space for an object of type 'const LDOM_Text'").
case " $* " in
  *"/LDOM/LDOM_BasicElement.cxx "*) exec "$@" -fno-sanitize=shift-exponent ;;
  *"/XmlObjMgt/XmlObjMgt.cxx "*)    exec "$@" -fno-sanitize=object-size ;;
  *) exec "$@" ;;
esac
