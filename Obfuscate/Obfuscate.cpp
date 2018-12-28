// We choose in this obfuscation pass to obfuscate recursive calls by adding a conditional branch around the call.
// The conditional predicate implemented here is a comparison between two equivalent logic formulas 
// A \/ (B /\ C) and (A \/ B) \/ (A \/ C). Two of these terms is chosen to be an operand of the recursive call.
// One is stored in a register and the other in memory. The use of memory access does not change the predicate
// but is used to add extra complexity since access optimizations cannot be performed without a pointer analysis. 
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
	BasicBlock* jumpBlock = BB->BasicBlock::splitBasicBlock(I, "jump trap");
	
	// The call instruction is the first one in the new block. Old iterator being invalidated.
	Instruction* Inst = &*(jumpBlock->begin());
	Type* ty = IntegerType::get(BB->getContext(), 32);
	Instruction *OrigInst = Inst;
	Value* op = Inst->getOperand(0);
 

   	Constant* any = ConstantInt::get(ty, 1 + RandAny(Generator));

	AllocaInst* memOp = new AllocaInst(ty, 0, "tmp", Inst);
	AllocaInst* memAny = new AllocaInst(ty, 0, "tmp", Inst);
	StoreInst* memOpStore = new StoreInst(op, memOp, false, Inst);
	StoreInst* memAnyStore = new StoreInst(any, memAny, false, Inst);

	LoadInst* loadOp = new LoadInst(memOp, "tmp", Inst);
	LoadInst* loadAny = new LoadInst(memAny, "tmp", Inst);

	LLVM_DEBUG(dbgs() << "generated any1 " <<  *any << "\n");

	IRBuilder<NoFolder> builder(Inst);

	Value* tmp0 = builder.CreateAnd(loadOp, loadAny); // loadOp /\ loadAny
	Value* rhs = builder.CreateOr(op, tmp0); // op \/ (loadOp /\ loadAny)

	Value* lhsTerm0 = builder.CreateOr(op, loadOp); //op \/ loadOp
	Value* lhsTerm1 = builder.CreateOr(op, loadAny); //op \/ loadAny

	Value* lhs = builder.CreateAnd(lhsTerm0, lhsTerm1); //(op \/ loadOp) /\ (op \/ loadAny)
	Value* comp = builder.CreateICmp(CmpInst::Predicate::ICMP_NE, lhs, rhs);

	TerminatorInst *ThenTerm = nullptr, *ElseTerm = nullptr;
  	SplitBlockAndInsertIfThenElse(comp, Inst, &ThenTerm, &ElseTerm);

	BasicBlock* ThenBlock = ThenTerm->getParent();
	BasicBlock* ElseBlock = ElseTerm->getParent();
	BasicBlock* MergeBlock = OrigInst->getParent();

	ThenBlock->setName("if.true");
	ElseBlock->setName("if.false");
	MergeBlock->setName("if.end.icp");

	builder.SetInsertPoint(ThenTerm);
	
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
