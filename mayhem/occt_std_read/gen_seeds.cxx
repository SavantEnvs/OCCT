// One-off generator for mayhem/occt_std_read/testsuite (not built by build.sh): writes BREP samples
// from data/occ as Std shape-schema files with each FSD driver, the way DDocStd's `fsdwrite` does,
// with and without a FILE_FORMAT user-info line so TDocStd_Application::Open dispatches them to the
// MDTV-Standard / OCC-StdLite retrieval drivers. OCCT has no Std storage driver any more, so real
// OCAF .std documents cannot be produced. Usage: gen_seeds <out-dir> <file.brep>...
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <FSD_BinaryFile.hxx>
#include <FSD_CmpFile.hxx>
#include <FSD_File.hxx>
#include <ShapePersistent_TopoDS.hxx>
#include <StdStorage.hxx>
#include <StdStorage_Data.hxx>
#include <StdStorage_HeaderData.hxx>
#include <StdStorage_Root.hxx>
#include <StdStorage_RootData.hxx>
#include <TopoDS_Shape.hxx>
#include <cstdio>
#include <string>

int main(int argc, char** argv)
{
  for (int a = 2; a < argc; ++a) {
    TopoDS_Shape s; BRep_Builder b;
    if (!BRepTools::Read(s, argv[a], b) || s.IsNull()) { fprintf(stderr, "skip %s\n", argv[a]); continue; }
    std::string base = argv[a]; base = base.substr(base.find_last_of('/') + 1); base = base.substr(0, base.find('.'));
    const char* fmts[] = {"", "MDTV-Standard", "OCC-StdLite"};
    for (int d = 0; d < 3; ++d) for (int f = 0; f < 3; ++f) {
      occ::handle<Storage_BaseDriver> drv;
      const char* dn = d == 0 ? "gen" : d == 1 ? "cmp" : "bin";
      if (d == 0) drv = new FSD_File; else if (d == 1) drv = new FSD_CmpFile; else drv = new FSD_BinaryFile;
      std::string out = std::string(argv[1]) + "/" + base + "_" + dn + (f ? (f == 1 ? "_mdtv" : "_stdl") : "") + ".std";
      if (drv->Open(out.c_str(), Storage_VSWrite) != Storage_VSOk) return 1;
      occ::handle<StdStorage_Data> data = new StdStorage_Data;
      data->HeaderData()->SetApplicationName(TCollection_ExtendedString("occt_std_read seeds"));
      if (f) data->HeaderData()->AddToUserInfo(TCollection_AsciiString("FILE_FORMAT: ") + fmts[f]);
      NCollection_DataMap<occ::handle<Standard_Transient>, occ::handle<StdObjMgt_Persistent>> map;
      Handle(ShapePersistent_TopoDS::HShape) p = ShapePersistent_TopoDS::Translate(s, map, ShapePersistent_WithTriangle);
      data->RootData()->AddRoot(new StdStorage_Root(base.c_str(), p));
      Storage_Error e = StdStorage::Write(drv, data);
      drv->Close();
      printf("%s -> %d\n", out.c_str(), (int)e);
    }
  }
  return 0;
}
