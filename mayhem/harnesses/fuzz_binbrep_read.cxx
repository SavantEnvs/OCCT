// Binary BREP shape reader: BinTools::Read(shape, istream).
#include "fuzz_common.hxx"
#include <BinTools.hxx>
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
    TopoDS_Shape s;
    BinTools::Read(s, is);
  OCCT_FUZZ_CATCH
  return 0;
}
