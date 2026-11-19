// Legacy Std persistence reader (TKStd/TKStdL): `.std`/`.stdl` files written by FSD_File (text),
// FSD_CmpFile or FSD_BinaryFile. Two public entry points read the same file:
//  1. StdStorage::Read(path) -- the shape-schema reader (header, type, root, ref and data sections,
//     ShapePersistent_* objects), then ShapePersistent_TopoDS::HShape::Import() on each root;
//  2. TDocStd_Application::Open(path) with StdDrivers/StdLDrivers registered -- the OCAF document
//     retrieval driver (StdLDrivers_DocumentRetrievalDriver). Its stream overload throws
//     Standard_NotImplemented, so the input goes through a per-input scratch file.
#include "fuzz_common.hxx"
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <PCDM_ReaderStatus.hxx>
#include <StdDrivers.hxx>
#include <StdLDrivers.hxx>
#include <StdStorage.hxx>
#include <StdStorage_Data.hxx>
#include <StdStorage_RootData.hxx>
#include <StdStorage_Root.hxx>
#include <ShapePersistent_TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>

static occ::handle<TDocStd_Application> g_app;

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  g_app = new TDocStd_Application();
  StdLDrivers::DefineFormat(g_app);
  StdDrivers::DefineFormat(g_app);
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  const char* tmp = getenv("TMPDIR");
  std::string path = std::string(tmp && *tmp ? tmp : "/tmp") + "/occt_std_XXXXXX";
  int fd = mkstemp(&path[0]);
  if (fd < 0) return 0;
  size_t off = 0;
  while (off < size) {
    ssize_t n = write(fd, data + off, size - off);
    if (n <= 0) break;
    off += (size_t)n;
  }
  close(fd);

  OCCT_FUZZ_TRY
    occ::handle<StdStorage_Data> aData;
    if (StdStorage::Read(TCollection_AsciiString(path.c_str()), aData) == Storage_VSOk && !aData.IsNull()) {
      occ::handle<NCollection_HSequence<occ::handle<StdStorage_Root>>> aRoots = aData->RootData()->Roots();
      if (!aRoots.IsNull()) {
        for (NCollection_HSequence<occ::handle<StdStorage_Root>>::Iterator it(*aRoots); it.More(); it.Next()) {
          Handle(ShapePersistent_TopoDS::HShape) aHShape =
            Handle(ShapePersistent_TopoDS::HShape)::DownCast(it.Value()->Object());
          if (!aHShape.IsNull()) {
            TopoDS_Shape aShape = aHShape->Import();
            (void)aShape;
          }
        }
      }
    }
  OCCT_FUZZ_CATCH

  OCCT_FUZZ_TRY
    occ::handle<TDocStd_Document> doc;
    PCDM_ReaderStatus st = g_app->Open(TCollection_ExtendedString(path.c_str()), doc);
    (void)st;
    // Close every opened document so no state carries into the next input.
    if (!doc.IsNull()) g_app->Close(doc);
  OCCT_FUZZ_CATCH

  unlink(path.c_str());
  return 0;
}
