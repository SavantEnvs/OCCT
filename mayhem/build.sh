#!/usr/bin/env bash
#
# mayhem/build.sh — build OCCT (the data-exchange / OCAF / healing toolkits the harnesses reach) as
# static libraries instrumented with $SANITIZER_FLAGS + SanCov, then link every harness twice
# (libFuzzer + standalone reproducer) and the known-answer oracle used by mayhem/test.sh.
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
# UBSan's `alignment` check is relaxed (only it): OCCT's NCollection_IncAllocator hands out
# 4-byte-aligned blocks for 8-byte nodes, which fires during static initialisation of TKDEVRML
# (VrmlData_Node.cxx global VrmlData_Scene -> NCollection_List::Append) before any input is read. The
# misaligned access is in the NCollection container templates inlined into every caller, not in the
# allocator's own file, so it cannot be confined to one source file.
# UBSan's `shift-exponent` stays ON except for one upstream file, LDOM_BasicElement.cxx (see
# mayhem/cxx_launcher.sh). ASan and every other UBSan check stay on and halting.
[ -z "$SANITIZER_FLAGS" ] || SANITIZER_FLAGS="$SANITIZER_FLAGS -fno-sanitize=alignment"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"
BUILD="$SRC/build-mayhem"       # gitignored by upstream's /build*
PREFIX="$SRC/install-mayhem"    # gitignored by upstream's /install*
OUT="$SRC/build-mayhem-out"     # gitignored by upstream's /build*

# SanCov on the library so the fuzzer sees coverage of OCCT itself, not just the harness.
LIBFLAGS="$SANITIZER_FLAGS -fsanitize=fuzzer-no-link $DEBUG_FLAGS"

# Only the toolkits the harnesses reach (dependencies are pulled in by OCCT's CMake); all GUI,
# Draw/Tcl, OpenGL, X11 and FreeType options are off so nothing outside apt's base is needed.
cmake -S "$SRC" -B "$BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="$LIBFLAGS" -DCMAKE_CXX_FLAGS="$LIBFLAGS" \
  -DCMAKE_C_FLAGS_RELEASE="-O1" -DCMAKE_CXX_FLAGS_RELEASE="-O1" \
  -DCMAKE_CXX_COMPILER_LAUNCHER="$SRC/mayhem/cxx_launcher.sh" \
  -DBUILD_LIBRARY_TYPE=Static \
  -DINSTALL_DIR="$PREFIX" \
  -DBUILD_MODULE_FoundationClasses=OFF -DBUILD_MODULE_ModelingData=OFF \
  -DBUILD_MODULE_ModelingAlgorithms=OFF -DBUILD_MODULE_Visualization=OFF \
  -DBUILD_MODULE_ApplicationFramework=OFF -DBUILD_MODULE_DataExchange=OFF \
  -DBUILD_MODULE_Draw=OFF -DBUILD_MODULE_DETools=OFF \
  -DBUILD_ADDITIONAL_TOOLKITS="TKDESTEP TKDEIGES TKDESTL TKDEOBJ TKDEVRML TKBinXCAF TKXmlXCAF TKShHealing TKStd TKStdL" \
  -DUSE_TK=OFF -DUSE_TCL=OFF -DUSE_FREETYPE=OFF -DUSE_OPENGL=OFF -DUSE_GLES2=OFF -DUSE_XLIB=OFF \
  -DUSE_FREEIMAGE=OFF -DUSE_FFMPEG=OFF -DUSE_OPENVR=OFF -DUSE_TBB=OFF -DUSE_VTK=OFF \
  -DUSE_RAPIDJSON=OFF -DUSE_DRACO=OFF -DUSE_D3D=OFF \
  -DBUILD_DOC_Overview=OFF -DBUILD_SAMPLES_QT=OFF -DBUILD_GTEST=OFF
cmake --build "$BUILD" -j"$MAYHEM_JOBS"
cmake --install "$BUILD" >/dev/null

INC="$(dirname "$(find "$PREFIX" -name Standard_Failure.hxx | head -1)")"
LIBDIR="$(dirname "$(find "$PREFIX" -name libTKernel.a | head -1)")"
LIBS="-Wl,--start-group $(ls "$LIBDIR"/libTK*.a | tr '\n' ' ') -Wl,--end-group -lpthread -ldl -lrt"
CXXF="-std=c++17 -I$INC $SANITIZER_FLAGS $DEBUG_FLAGS"

mkdir -p "$OUT"
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c "$SRC/mayhem/lsan_off.cc" -o "$OUT/lsan_off.o"
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$OUT/standalone_main.o"

build_target() { # <name> <source> [extra flags]
  local name="$1" src="$2"; shift 2
  $CXX $CXXF "$@" -c "$src" -o "$OUT/$name.o"
  $CXX $SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE "$OUT/$name.o" "$OUT/lsan_off.o" $LIBS -o "/mayhem/$name"
  $CXX $SANITIZER_FLAGS $DEBUG_FLAGS "$OUT/$name.o" "$OUT/standalone_main.o" "$OUT/lsan_off.o" $LIBS -o "/mayhem/$name-standalone"
}

H="$SRC/mayhem/harnesses"
build_target occt_brep_read    "$H/fuzz_brep_read.cxx"
build_target occt_binbrep_read "$H/fuzz_binbrep_read.cxx"
build_target occt_stl_read     "$H/fuzz_stl_read.cxx"
build_target occt_obj_read     "$H/fuzz_obj_read.cxx"
build_target occt_vrml_read    "$H/fuzz_vrml_read.cxx"
build_target occt_step_read    "$H/fuzz_step_read.cxx"
build_target occt_iges_read    "$H/fuzz_iges_read.cxx"
build_target occt_xbf_open     "$H/fuzz_ocaf_open.cxx"
build_target occt_xmlocaf_open "$H/fuzz_ocaf_open.cxx" -DOCAF_XML
build_target occt_shapefix     "$H/fuzz_shapefix.cxx"
build_target occt_std_read     "$H/fuzz_std_read.cxx"

# Known-answer oracle for mayhem/test.sh. It links the same OCCT libraries: a second, unsanitized
# build of OCCT would double an already ~20-minute compile.
$CXX $CXXF $COVERAGE_FLAGS "$SRC/mayhem/oracle/occt_oracle.cxx" "$OUT/lsan_off.o" $LIBS $COVERAGE_FLAGS -o "$OUT/occt_oracle"
echo "build.sh: done"
