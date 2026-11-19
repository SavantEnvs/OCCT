// OCAF document loader from a stream: TDocStd_Application::Open(istream) with the binary (XBF/BinOcaf)
// or XML (XmlXCAF/XmlOcaf) storage drivers registered. Built twice: OCAF_XML selects the XML drivers.
#include "fuzz_common.hxx"
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <PCDM_ReaderStatus.hxx>
#ifdef OCAF_XML
#include <XmlDrivers.hxx>
#include <XmlLDrivers.hxx>
#include <XmlXCAFDrivers.hxx>
#else
#include <BinDrivers.hxx>
#include <BinLDrivers.hxx>
#include <BinXCAFDrivers.hxx>
#endif

static occ::handle<TDocStd_Application> g_app;

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  g_app = new TDocStd_Application();
#ifdef OCAF_XML
  XmlDrivers::DefineFormat(g_app);
  XmlLDrivers::DefineFormat(g_app);
  XmlXCAFDrivers::DefineFormat(g_app);
#else
  BinDrivers::DefineFormat(g_app);
  BinLDrivers::DefineFormat(g_app);
  BinXCAFDrivers::DefineFormat(g_app);
#endif
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::istringstream is(std::string(reinterpret_cast<const char*>(data), size), std::ios::in | std::ios::binary);
  OCCT_FUZZ_TRY
    occ::handle<TDocStd_Document> doc;
    PCDM_ReaderStatus st = g_app->Open(is, doc);
    // Close every opened document so no state carries into the next input.
    if (!doc.IsNull()) {
      (void)st;
      g_app->Close(doc);
    }
  OCCT_FUZZ_CATCH
  return 0;
}
