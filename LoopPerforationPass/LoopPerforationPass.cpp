//===-- Frequent Path Loop Invariant Code Motion Pass --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
  struct LoopCountPass : public PassInfoMixin<LoopCountPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
      auto &LI = FAM.getResult<LoopAnalysis>(F);
      unsigned int loopCount = 0;

      raw_ostream &out = llvm::errs();
      out << "Function \"" << F.getName() << "\" has the following loops:\n";

      for (Loop *L : LI) {
        loopCount++;
        out << "Loop level " << L->getLoopDepth() << " with "
            << L->getNumBlocks() << " blocks.\n";
      }

      out << "Total loops found: " << loopCount << "\n";

      return PreservedAnalyses::all();
    }
  };

  struct LoopPerforationPass : public PassInfoMixin<LoopPerforationPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
      auto &LI = FAM.getResult<LoopAnalysis>(F);
      raw_ostream &out = llvm::errs();
      bool changed = false;

      out << "Function \"" << F.getName() << "\" has the following loops:\n";

      for (Loop *L : LI) {
        out << "Loop level " << L->getLoopDepth() << " with "
            << L->getNumBlocks() << " blocks.\n";
        if (Loop *ParentLoop = L->getParentLoop())
          continue; // Only perforate innermost loops for simplicity

        for (BasicBlock *BB : L->getBlocks()) {
          for (Instruction &I : *BB) {
            // Look for the increment statement of the induction variable
            if (auto *II = dyn_cast<BinaryOperator>(&I)) {
              if (II->getOpcode() == Instruction::Add && II->getOperand(1)->getType()->isIntegerTy()) {
                Value *increment = II->getOperand(1);
                if (auto *constInc = dyn_cast<ConstantInt>(increment)) {
                  if (constInc->isOne()) { // Ensure we are incrementing by 1
                    // Change the increment from +1 to +2 to skip iterations
                    out << "Old instruction: " << *II << "\n";
                    Value *newIncrement = ConstantInt::get(increment->getType(), 2);
                    II->setOperand(1, newIncrement);
                    changed = true;
                    out << "New instruction: " << *II << "\n";
                    out << "Perforated a loop in function \"" << F.getName() << "\" at block " << BB->getName() << ".\n";
                  }
                }
              }
            }
          }
        }
      }

      if (changed) {
        out << "Changes were applied to the function \"" << F.getName() << "\".\n";
      }
      return PreservedAnalyses::all();
    }
  };

} // namespace;

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "LoopPerforationPass",
    LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "loop-count-pass") {
            FPM.addPass(LoopCountPass());
            return true;
          }
          if (Name == "loop-perforation-pass") {
            FPM.addPass(LoopPerforationPass());
            return true;
          }
          return false;
        }
      );
    }
  };
}
