// BREP (text) shape reader: BRepTools::Read(shape, istream, builder) + BRepCheck_Analyzer.
#include "fuzz_common.hxx"
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRepCheck_Analyzer.hxx>
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
    if (!s.IsNull()) {
      BRepCheck_Analyzer an(s);
      (void)an.IsValid();
    }
  OCCT_FUZZ_CATCH
  return 0;
}
