// Meshing an imported BREP: BRepMesh_IncrementalMesh (BRepMesh_ModelBuilder, curve/face discretisation,
// BRepMesh_Delaun triangulation, model healing), with deflections scaled to the shape.
#include "fuzz_brep_common.hxx"
#include <BRepMesh_IncrementalMesh.hxx>

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  OCCT_FUZZ_TRY
    TopoDS_Shape s = occt_fuzz_read_brep(data, size);
    if (s.IsNull()) return -1;
    // Relative linear deflection 0.05, angular 0.5 rad, single-threaded so each run is deterministic.
    BRepMesh_IncrementalMesh mesh(s, 0.05, true, 0.5, false);
    (void)mesh.IsDone();
  OCCT_FUZZ_CATCH
  return 0;
}
