// Fillet on an imported BREP: BRepFilletAPI_MakeFillet (ChFi3d_FilBuilder, ChFiKPart, ChFi3d
// surface/edge intersection) with a constant radius on the first edges of the first solid.
#include "fuzz_brep_common.hxx"
#include <BRepFilletAPI_MakeFillet.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

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
    // BRepFilletAPI_MakeFillet's contract is a solid ("S is the shape on which fillets are built",
    // ChFi3d_Builder indexes its solids): fillet the first solid, reject inputs with none.
    TopExp_Explorer sol(s, TopAbs_SOLID);
    if (!sol.More()) return -1;
    const TopoDS_Shape solid = sol.Current();
    const double diag = occt_fuzz_diag(solid);
    if (diag <= 0.0) return -1;
    BRepFilletAPI_MakeFillet mf(solid);
    int n = 0;
    for (TopExp_Explorer ex(solid, TopAbs_EDGE); ex.More() && n < 12; ex.Next(), ++n)
      mf.Add(diag * 0.02, TopoDS::Edge(ex.Current()));
    mf.Build();
    if (mf.IsDone()) (void)mf.Shape().IsNull();
  OCCT_FUZZ_CATCH
  return 0;
}
