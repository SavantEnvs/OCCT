// STEP-XDE: STEPCAFControl_Reader::ReadStream + Transfer into an XCAF document (assembly structure,
// names, colours, layers, validation props, GD&T, materials, views), then the STEP write back out of
// that document with STEPCAFControl_Writer (the read/write round trip).
#include "fuzz_common.hxx"
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <STEPCAFControl_Controller.hxx>
#include <XCAFApp_Application.hxx>
#include <TDocStd_Document.hxx>
#include <IFSelect_ReturnStatus.hxx>

static occ::handle<XCAFApp_Application> g_app;

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  STEPCAFControl_Controller::Init();
  g_app = XCAFApp_Application::GetApplication();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::istringstream is(std::string(reinterpret_cast<const char*>(data), size), std::ios::in | std::ios::binary);
  occ::handle<TDocStd_Document> doc;
  OCCT_FUZZ_TRY
    STEPCAFControl_Reader r;
    r.SetColorMode(true);
    r.SetNameMode(true);
    r.SetLayerMode(true);
    r.SetPropsMode(true);
    r.SetGDTMode(true);
    r.SetMatMode(true);
    r.SetViewMode(true);
    if (r.ReadStream("input.stp", is) != IFSelect_RetDone) return -1;
    g_app->NewDocument("MDTV-XCAF", doc);
    if (r.Transfer(doc)) {
      STEPCAFControl_Writer w;
      if (w.Transfer(doc)) {
        std::ostringstream os;
        (void)w.WriteStream(os);
      }
    }
  OCCT_FUZZ_CATCH
  // Close the document so nothing carries into the next input.
  if (!doc.IsNull()) {
    OCCT_FUZZ_TRY
      g_app->Close(doc);
    OCCT_FUZZ_CATCH
  }
  return 0;
}
