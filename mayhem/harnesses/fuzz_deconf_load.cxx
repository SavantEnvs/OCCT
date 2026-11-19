// DE configuration strings: DE_ConfigurationContext::LoadStr (the "scope.key : value" resource parser)
// then DE_Wrapper::Load, which hands the parsed values to every bound format node (STEP, IGES, STL,
// OBJ, VRML: their typed GetReal/GetInteger/GetBoolean/GetStringSeq reads), and Save back to text.
// A fresh wrapper per input: the global wrapper (DE_Wrapper::GlobalWrapper) is never touched.
#include "fuzz_common.hxx"
#include <DE_ConfigurationContext.hxx>
#include <DE_Wrapper.hxx>
#include <DESTEP_ConfigurationNode.hxx>
#include <DEIGES_ConfigurationNode.hxx>
#include <DESTL_ConfigurationNode.hxx>
#include <DEOBJ_ConfigurationNode.hxx>
#include <DEVRML_ConfigurationNode.hxx>
#include <TCollection_AsciiString.hxx>

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  occt_fuzz_quiet();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  // TCollection_AsciiString is NUL-terminated text; the resource format is text.
  const std::string text(reinterpret_cast<const char*>(data), size);
  if (text.find('\0') != std::string::npos) return -1;
  OCCT_FUZZ_TRY
    occ::handle<DE_ConfigurationContext> ctx = new DE_ConfigurationContext();
    if (!ctx->LoadStr(TCollection_AsciiString(text.c_str()))) return 0;
    occ::handle<DE_Wrapper> w = new DE_Wrapper();
    w->Bind(new DESTEP_ConfigurationNode());
    w->Bind(new DEIGES_ConfigurationNode());
    w->Bind(new DESTL_ConfigurationNode());
    w->Bind(new DEOBJ_ConfigurationNode());
    w->Bind(new DEVRML_ConfigurationNode());
    if (w->Load(ctx, true)) {
      TCollection_AsciiString out = w->Save();
      (void)out.Length();
    }
  OCCT_FUZZ_CATCH
  return 0;
}
