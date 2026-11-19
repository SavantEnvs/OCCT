// Lofting on an imported BREP: BRepOffsetAPI_ThruSections (BRepFill_Generator, BRepFill_CompatibleWires,
// GeomFill_AppSurf / AppSkin approximation) through the wires (and end vertices) of the read shape.
#include "fuzz_brep_common.hxx"
#include <BRepOffsetAPI_ThruSections.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if (size < 1) return -1;
  // The first byte picks the mode (solid/shell, ruled/smooth); the rest is the BREP file.
  const bool isSolid = data[0] & 1, ruled = data[0] & 2;
  OCCT_FUZZ_TRY
    TopoDS_Shape s = occt_fuzz_read_brep(data + 1, size - 1);
    if (s.IsNull()) return -1;
    BRepOffsetAPI_ThruSections ts(isSolid, ruled);
    int nw = 0;
    for (TopExp_Explorer ex(s, TopAbs_WIRE); ex.More() && nw < 8; ex.Next(), ++nw)
      ts.AddWire(TopoDS::Wire(ex.Current()));
    // ThruSections needs at least two sections (BRepOffsetAPI_ThruSections.hxx: "at least two").
    if (nw < 2) return -1;
    ts.CheckCompatibility(true);
    ts.Build();
    if (ts.IsDone()) (void)ts.Shape().IsNull();
  OCCT_FUZZ_CATCH
  return 0;
}
