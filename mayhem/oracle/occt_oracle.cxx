// Known-answer functional oracle for the OCCT readers the fuzz targets exercise.
// Reads sample files shipped in upstream's data/ tree and prints one "<check> <value>" line per
// check; mayhem/test.sh diffs the output against mayhem/oracle/expected.txt line by line.
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BinTools.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Controller.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Controller.hxx>
#include <RWStl.hxx>
#include <Poly_Triangulation.hxx>
#include <VrmlData_Scene.hxx>
#include <VrmlData_DataMapOfShapeAppearance.hxx>
#include <Standard_Failure.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>

static std::string D; // data dir

static int count(const TopoDS_Shape& s, TopAbs_ShapeEnum t)
{
  if (s.IsNull()) return -1;
  TopTools_IndexedMapOfShape m;
  TopExp::MapShapes(s, t, m);
  return m.Extent();
}

static void topo(const char* name, const TopoDS_Shape& s)
{
  printf("%s.faces %d\n", name, count(s, TopAbs_FACE));
  printf("%s.edges %d\n", name, count(s, TopAbs_EDGE));
  printf("%s.vertices %d\n", name, count(s, TopAbs_VERTEX));
}

int main(int argc, char** argv)
{
  D = argc > 1 ? argv[1] : "data";
  Message::DefaultMessenger()->RemovePrinters(STANDARD_TYPE(Message_Printer));
  try {
    // BREP text
    TopoDS_Shape bottle;
    BRep_Builder b;
    printf("brep.read %d\n", BRepTools::Read(bottle, (D + "/occ/bottle.brep").c_str(), b) ? 1 : 0);
    topo("brep", bottle);
    printf("brep.valid %d\n", BRepCheck_Analyzer(bottle).IsValid() ? 1 : 0);
    GProp_GProps p;
    BRepGProp::VolumeProperties(bottle, p);
    printf("brep.volume %.1f\n", p.Mass());

    // Binary BREP round trip
    std::stringstream bs(std::ios::in | std::ios::out | std::ios::binary);
    BinTools::Write(bottle, bs);
    TopoDS_Shape back;
    BinTools::Read(back, bs);
    topo("binbrep", back);

    // STEP
    STEPControl_Controller::Init();
    STEPControl_Reader sr;
    printf("step.read %d\n", (int)sr.ReadFile((D + "/step/screw.step").c_str()));
    printf("step.roots %d\n", sr.NbRootsForTransfer());
    sr.TransferRoots();
    topo("step", sr.OneShape());

    // IGES
    IGESControl_Controller::Init();
    IGESControl_Reader ir;
    printf("iges.read %d\n", (int)ir.ReadFile((D + "/iges/bearing.iges").c_str()));
    ir.TransferRoots();
    topo("iges", ir.OneShape());

    // STL (binary sample read through the stream API)
    std::ifstream stl(D + "/stl/bearing.stl", std::ios::binary);
    occ::handle<Poly_Triangulation> t = RWStl::ReadStream(stl);
    printf("stl.triangles %d\n", t.IsNull() ? -1 : t->NbTriangles());
    printf("stl.nodes %d\n", t.IsNull() ? -1 : t->NbNodes());

    // VRML
    std::ifstream vf(D + "/vrml/face.vrml", std::ios::binary);
    VrmlData_Scene scene;
    scene << vf;
    printf("vrml.status %d\n", (int)scene.Status());
    VrmlData_DataMapOfShapeAppearance am;
    topo("vrml", scene.GetShape(am));
  } catch (const Standard_Failure& e) {
    printf("exception %s\n", e.GetMessageString());
    fflush(stdout);
    _exit(1);
  }
  // Skip OCCT's static destructors (upstream destruction-order issue reported by UBSan vptr at exit).
  fflush(stdout);
  _exit(0);
}
