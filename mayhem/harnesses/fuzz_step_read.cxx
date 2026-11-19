// STEP reader: STEPControl_Reader::ReadStream (StepFile lexer/parser, RWStep* readers) + TransferRoots
// (StepToGeom / StepToTopoDS / ShapeFix).
#include "fuzz_common.hxx"
#include <STEPControl_Reader.hxx>
#include <STEPControl_Controller.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopoDS_Shape.hxx>

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  STEPControl_Controller::Init();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::istringstream is(std::string(reinterpret_cast<const char*>(data), size), std::ios::in | std::ios::binary);
  OCCT_FUZZ_TRY
    STEPControl_Reader r;
    if (r.ReadStream("input.stp", is) == IFSelect_RetDone) {
      r.TransferRoots();
      TopoDS_Shape s = r.OneShape();
      (void)s.IsNull();
    }
  OCCT_FUZZ_CATCH
  return 0;
}
