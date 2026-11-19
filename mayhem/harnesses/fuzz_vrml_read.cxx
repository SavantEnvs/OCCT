// VRML 1/2 reader: VrmlData_Scene << istream, then conversion to a shape.
#include "fuzz_common.hxx"
#include <VrmlData_Scene.hxx>
#include <VrmlData_DataMapOfShapeAppearance.hxx>
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
    VrmlData_Scene scene;
    scene << is;
    if (scene.Status() == VrmlData_StatusOK) {
      VrmlData_DataMapOfShapeAppearance m;
      TopoDS_Shape s = scene.GetShape(m);
      (void)s.IsNull();
    }
  OCCT_FUZZ_CATCH
  return 0;
}
