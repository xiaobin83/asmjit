// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// This file is used to test opcodes generated by AsmJit. Output can be
// disassembled in your IDE or by your favorite disassembler. Instructions
// are grouped by category and then sorted alphabetically.

// [Dependencies]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./asmjit.h"
#include "./asmjit_test_opcode.h"

using namespace asmjit;

struct OpcodeDumpInfo {
  uint32_t archType;
  bool useRex1;
  bool useRex2;
};

static const char* archTypeToString(uint32_t archType) {
  switch (archType) {
    case ArchInfo::kTypeNone : return "None";
    case ArchInfo::kTypeX86  : return "X86";
    case ArchInfo::kTypeX64  : return "X64";
    case ArchInfo::kTypeArm32: return "ARM32";
    case ArchInfo::kTypeArm64: return "ARM64";

    default:
      return "<unknown>";
  }
}

struct TestErrorHandler : public ErrorHandler {
  virtual bool handleError(Error err, const char* message, CodeEmitter* origin) {
    printf("ERROR 0x%08X: %s\n", err, message);
    return true;
  }
};

typedef void (*VoidFunc)(void);

int main(int argc, char* argv[]) {
  TestErrorHandler eh;

  OpcodeDumpInfo infoList[] = {
    { ArchInfo::kTypeX86, false, false },
    { ArchInfo::kTypeX64, false, false },
    { ArchInfo::kTypeX64, false, true  },
    { ArchInfo::kTypeX64, true , false },
    { ArchInfo::kTypeX64, true , true  }
  };

  for (int i = 0; i < ASMJIT_ARRAY_SIZE(infoList); i++) {
    const OpcodeDumpInfo& info = infoList[i];

    printf("Opcodes [ARCH=%s REX1=%s REX2=%s]\n",
      archTypeToString(info.archType),
      info.useRex1 ? "true" : "false",
      info.useRex2 ? "true" : "false");

    CodeHolder code;
    code.init(CodeInfo(info.archType));
    code.setErrorHandler(&eh);

#if !defined(ASMJIT_DISABLE_LOGGING)
    FileLogger logger(stdout);
    logger.addOptions(Logger::kOptionBinaryForm);
    code.setLogger(&logger);
#endif // ASMJIT_DISABLE_LOGGING

    X86Assembler a(&code);
    asmtest::generateOpcodes(a.asEmitter(), info.useRex1, info.useRex2);

    // If this is the host architecture the code generated can be executed
    // for debugging purposes (the first instruction is ret anyway).
    if (code.getArchType() == ArchInfo::kTypeHost) {
      JitRuntime runtime;
      VoidFunc p;
      Error err = runtime.add(&p, &code);
      if (err == kErrorOk) p();
    }
  }

  return 0;
}
