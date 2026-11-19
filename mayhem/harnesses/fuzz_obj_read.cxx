// Wavefront OBJ reader: RWObj_TriangulationReader::Read(istream) with shape creation.
#include "fuzz_common.hxx"
#include <RWObj_TriangulationReader.hxx>
#include <Message_ProgressRange.hxx>
#include <TopoDS_Shape.hxx>

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::istringstream is(std::string(reinterpret_cast<const char*>(data), size), std::ios::in | std::ios::binary);
  OCCT_FUZZ_TRY
    RWObj_TriangulationReader r;
    r.SetCreateShapes(true);
    if (r.Read(is, "input.obj", Message_ProgressRange())) {
      TopoDS_Shape s = r.ResultShape();
      (void)s.IsNull();
    }
  OCCT_FUZZ_CATCH
  return 0;
}
