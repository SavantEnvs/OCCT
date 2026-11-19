// Extrema on an imported BREP: BRepExtrema_DistShapeShape (BRepExtrema_DistanceSS, Extrema_ExtCC /
// ExtCS / ExtSS / ExtPS) between the first two top-level sub-shapes of the read shape, then
// BRepExtrema_ExtCC/ExtFF style queries via the solution supports.
#include "fuzz_brep_common.hxx"
#include <BRepExtrema_DistShapeShape.hxx>
#include <TopoDS_Iterator.hxx>

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
    // Two operands: the first two children of the read shape (a compound in the usual BREP file).
    TopoDS_Shape a, b;
    TopoDS_Iterator it(s);
    if (it.More()) { a = it.Value(); it.Next(); }
    if (it.More()) { b = it.Value(); }
    if (a.IsNull() || b.IsNull()) return -1;
    BRepExtrema_DistShapeShape dss(a, b);
    if (dss.IsDone()) {
      volatile double v = dss.Value();
      (void)v;
      for (int i = 1; i <= dss.NbSolution() && i <= 8; ++i) {
        (void)dss.PointOnShape1(i);
        (void)dss.PointOnShape2(i);
        (void)dss.SupportOnShape1(i);
      }
    }
  OCCT_FUZZ_CATCH
  return 0;
}
