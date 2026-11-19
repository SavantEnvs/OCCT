// IGES reader: IGESControl_Reader::ReadFile (IGESFile C reader has no stream API, so the input is
// written to a private temp file under $TMPDIR) + TransferRoots (IGESToBRep / ShapeFix).
#include "fuzz_common.hxx"
#include <IGESControl_Reader.hxx>
#include <IGESControl_Controller.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopoDS_Shape.hxx>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

static std::string g_path;

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  IGESControl_Controller::Init();
  const char* tmp = getenv("TMPDIR");
  std::string tmpl = std::string(tmp && *tmp ? tmp : "/tmp") + "/occt_iges_XXXXXX";
  int fd = mkstemp(&tmpl[0]);
  if (fd < 0) abort();
  close(fd);
  g_path = tmpl;
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  FILE* f = fopen(g_path.c_str(), "wb");
  if (!f) abort();
  if (size && fwrite(data, 1, size, f) != size) abort();
  fclose(f);
  OCCT_FUZZ_TRY
    IGESControl_Reader r;
    if (r.ReadFile(g_path.c_str()) == IFSelect_RetDone) {
      r.TransferRoots();
      TopoDS_Shape s = r.OneShape();
      (void)s.IsNull();
    }
  OCCT_FUZZ_CATCH
  return 0;
}
