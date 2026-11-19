// Shape healing on an untrusted BREP: BRepTools::Read then ShapeFix_Shape::Perform
// (ShapeFix_Face/Wire/Edge, periodic face healing) and a ShapeFix_Face pass with no context.
#include "fuzz_common.hxx"
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::istringstream is(std::string(reinterpret_cast<const char*>(data), size));
  OCCT_FUZZ_TRY
    TopoDS_Shape s;
    BRep_Builder b;
    BRepTools::Read(s, is, b);
    if (s.IsNull()) return 0;
    int n = 0;
    for (TopExp_Explorer ex(s, TopAbs_FACE); ex.More() && n < 16; ex.Next(), ++n) {
      ShapeFix_Face ff(TopoDS::Face(ex.Current()));
      ff.Perform();
    }
    occ::handle<ShapeFix_Shape> fix = new ShapeFix_Shape(s);
    fix->Perform();
    (void)fix->Shape();
  OCCT_FUZZ_CATCH
  return 0;
}
