// Shared by the modeling-algorithm harnesses: the input is an untrusted BREP text file, read with
// BRepTools::Read (the same reader occt_brep_read fuzzes), and the algorithm then runs on that shape.
#pragma once
#include "fuzz_common.hxx"
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <TopoDS_Shape.hxx>
#include <cmath>

// Returns a null shape when the input is not a BREP the reader accepts.
static inline TopoDS_Shape occt_fuzz_read_brep(const uint8_t* data, size_t size)
{
  std::istringstream is(std::string(reinterpret_cast<const char*>(data), size));
  TopoDS_Shape s;
  BRep_Builder b;
  BRepTools::Read(s, is, b);
  return s;
}

// Diagonal of the shape's bounding box, or 0 when it is void or infinite: the algorithms' size
// parameters (fillet radius, mesh deflection) are scaled to it so they mean the same at any scale.
static inline double occt_fuzz_diag(const TopoDS_Shape& s)
{
  Bnd_Box box;
  BRepBndLib::Add(s, box);
  if (box.IsVoid() || box.IsOpen()) return 0.0;
  double d = std::sqrt(box.SquareExtent());
  return std::isfinite(d) ? d : 0.0;
}
