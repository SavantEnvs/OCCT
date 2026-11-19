// STL reader (ASCII and binary, auto-detected): RWStl::ReadStream.
#include "fuzz_common.hxx"
#include <RWStl.hxx>
#include <Poly_Triangulation.hxx>

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::istringstream is(std::string(reinterpret_cast<const char*>(data), size), std::ios::in | std::ios::binary);
  OCCT_FUZZ_TRY
    occ::handle<Poly_Triangulation> t = RWStl::ReadStream(is);
    if (!t.IsNull()) (void)t->NbTriangles();
  OCCT_FUZZ_CATCH
  return 0;
}
