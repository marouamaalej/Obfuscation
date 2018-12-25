// We choose in this obfuscation pass to obfuscate recursive calls by adding a conditional branch around the call.
// The conditional predicate implemented here is a simple comparison between two randomly generated constants.
// Any more complicated predicate may be used for the purpose.
 
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CallPromotionUtils.h"

#define  DEBUG_TYPE "Obfuscate"
#include "llvm/Support/Debug.h"

#include <random>

using namespace llvm;


namespace {
  class Obfuscate : public FunctionPass {
  std::default_random_engine Generator;
  public:
    static char ID;
    Obfuscate() : FunctionPass(ID) {}


  // RunOnFunction pass used because the program may not have only SESE blocks (also avoid using the RGPassManager interface).
  bool runOnFunction(Function &F) override {

  std::uniform_int_distribution<size_t> RandAny(1, 10);
  // The split variable is used to avoid splitting at the same instruction twice. The bb being split, the very instruction is moved to the
  // new bb, hence is encountred later when iterating the new bb.
  Instruction* split = NULL;
for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
  for (BasicBlock::iterator I = BB->begin(), end = BB->end(); I != end; ++I) {
    Instruction* Inst = &*I;
    if (isa<CallInst>(Inst) && cast<CallInst>(Inst)->getCalledFunction() == &F && Inst != split){
	split = &*I;

	// Create a jump block
	BasicBlock* jumpBlock = BB->BasicBlock::splitBasicBlock(I, "jump before");
	
	// The call instruction is the first one in the new block. Old iterator being invalidated.
	Instruction* Inst = &*(jumpBlock->begin());
	Type* ty = IntegerType::get(BB->getContext(), 32);
	Instruction *OrigInst = Inst;

   	Constant* any1 = ConstantInt::get(ty, 1 + RandAny(Generator));
	Constant* any2 = ConstantInt::get(ty, 1 + RandAny(Generator));
	LLVM_DEBUG(dbgs() << "generated any1 " <<  *any1 << "\n");
	LLVM_DEBUG(dbgs() << "generated any2 " <<  *any2 << "\n");

	IRBuilder<NoFolder> builder(Inst);
	Value *comp = builder.CreateICmp(CmpInst::Predicate::ICMP_EQ, any1, any2);
	   
	TerminatorInst *ThenTerm = nullptr, *ElseTerm = nullptr;
  	SplitBlockAndInsertIfThenElse(comp, Inst, &ThenTerm, &ElseTerm);

	BasicBlock *ThenBlock = ThenTerm->getParent();
	BasicBlock *ElseBlock = ElseTerm->getParent();
	BasicBlock *MergeBlock = OrigInst->getParent();

	ThenBlock->setName("if.true");
	ElseBlock->setName("if.false");
	MergeBlock->setName("if.end.icp");
	
	ThenTerm->eraseFromParent();
	BranchInst::Create (jumpBlock, ThenBlock); 

	break;

    }

  }

  }
return true; //true because the the function was modified by the transformation.
}
}; 
}  // end of anonymous namespace

char Obfuscate::ID = 0;
static RegisterPass<Obfuscate> X("obfuscate", "Obfuscate Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
