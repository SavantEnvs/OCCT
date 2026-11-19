// Shared helpers for the OCCT fuzz harnesses.
// OCCT reports malformed input with C++ exceptions (Standard_Failure); those are the library's
// documented error path, so the harnesses catch them. Memory errors and UB are caught by ASan/UBSan.
#pragma once
#include <Standard_Failure.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <exception>
#include <sstream>
#include <string>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

// OCCT's static destructors run in an order that UBSan's vptr check reports
// (Standard_Type::~Standard_Type touching the already-destroyed type registry, Standard_Type.cxx:166)
// at normal process exit, after every input has been processed. Skip static destruction: this
// atexit hook is registered after OCCT's static initialisation, so it runs before those destructors.
// It only matters for the -standalone reproducers (libFuzzer itself leaves via _Exit).
static void occt_fuzz_fast_exit()
{
  fflush(nullptr);
  _exit(0);
}

// Silence OCCT's default messenger (console printer) once; this is read-only process state.
static inline void occt_fuzz_quiet()
{
  static bool done = false;
  if (done) return;
  done = true;
  atexit(occt_fuzz_fast_exit);
  const occ::handle<Message_Messenger>& m = Message::DefaultMessenger();
  m->RemovePrinters(STANDARD_TYPE(Message_Printer));
}

#define OCCT_FUZZ_TRY try {
#define OCCT_FUZZ_CATCH } catch (const Standard_Failure&) {} catch (const std::exception&) {}
