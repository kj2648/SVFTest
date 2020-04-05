#include "WPA/FlowSensitive.h"
#include "WPA/Andersen.h"
#include "WPA/AndersenSFR.h"
#include "WPA/WPAPass.h"
#include "MSSA/SVFGNode.h"

#include "llvm/Support/CommandLine.h"	// for cl
#include "llvm/IRReader/IRReader.h"	// IR reader for bit file
#include "llvm/Support/SourceMgr.h" // for SMDiagnostic
#include "llvm/IR/LLVMContext.h"		// for llvm LLVMContext
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"

using namespace llvm;
using namespace std;

static cl::opt<std::string> InputFilename(cl::Positional,
        cl::desc("<input bitcode>"), cl::init("-"));

string getINIndent(int depth) {
    string ret;
    while(depth--) {
        ret += "<-";
    }
    return ret;
}
string getOUTIndent(int depth) {
    string ret;
    while(depth--) {
        ret += "->";
    }
    return ret;
}

void printINs(PAGNode* node, int depth) {
    if(auto V = dyn_cast<Function>(node->getValue())) {
        errs() << "in from function: " << V->getName() << "\n";
        return;
    }
    for(auto in : node->getInEdges()) {
        if(in->getSrcNode()->getFunction() != node->getFunction()) {
            if(!in->getSrcNode()->getFunction()) {
                errs() << "cannot get function of src node : ";
            }
            else {
                errs() << "in from function: " << in->getSrcNode()->getFunction()->getName() << ": ";
            }
            errs() << "(" << in->getEdgeKind() << ") ";
            errs() << *in->getValue() << "\n";
            continue;
        }
        errs() << getINIndent(depth) << "(" << in->getEdgeKind() << ") ";
        errs() << *in->getValue() << "\n";
        printINs(in->getSrcNode(), depth+1);
    }
}

void printOuts(PAGNode* node, int depth) {
    if(auto V = dyn_cast<Function>(node->getValue())) {
        errs() << "out to function: " << V->getName() << "\n";
        return;
    }
    for(auto out : node->getOutEdges()) {
        errs() << getOUTIndent(depth) << "(" << out->getEdgeKind() << ") ";
        errs() << *out->getValue() << "\n";
        printOuts(out->getDstNode(), depth+1);
    }
}

int main(int argc, char ** argv) {
    LLVMContext Context;
    SMDiagnostic Err;
    unique_ptr<Module> M = parseIRFile(argv[1], Err, Context);
    llvm::PassBuilder PB;
    ModuleAnalysisManager MAM;
    FunctionAnalysisManager FAM;
    LoopAnalysisManager LAM;
    CGSCCAnalysisManager CGAM;
    ModulePassManager MPM;
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
    MPM.addPass(createModuleToFunctionPassAdaptor(PromotePass()));
    MPM.run(*M, MAM);

    SVFModule svfModule(*M);

    Andersen pta;
    SVFGBuilder memSSA(true);
    pta.analyze(svfModule);
    SVFG *svfg = memSSA.buildFullSVFG((BVDataPTAImpl*)&pta);
    PAG *pag = pta.getPAG();

    // FlowSensitive pta;
    // pta.analyze(svfModule);
    // SVFG *svfg = pta.getSVFG();
    // PAG *pag = pta.getPAG();

    for(auto& F : M->getFunctionList()) {
        errs() << "[FUNC] " << F.getName() << "\n";
        errs() << F << "\n";

        if(pag->hasFunArgsMap(&F))
        for(const PAGNode* A : pag->getFunArgsList(&F)) {
            errs() << *A->getValue() << "\n";

            // PAG only : flow-insensitive
            // errs() << "IN:\n";
            // printINs(const_cast<PAGNode *>(A), 0);
            // errs() << "OUT:\n";
            // printOuts(const_cast<PAGNode *>(A), 0);

            // SVFG : flow-sensitive
            SmallSetVector<SVFGNode *, 16> WorkList;
            SmallPtrSet<SVFGNode *, 16> Visited;
            SVFGNode* node = const_cast<SVFGNode *>(svfg->getDefSVFGNode(A));
            // auto topNode = svfg->getLHSTopLevPtr(node);
            WorkList.insert(node);
            while(!WorkList.empty()) {
                SVFGNode* node = WorkList.pop_back_val();
                if(Visited.find(node) != Visited.end()) {
                    continue;
                }
                Visited.insert(node);
                errs() << node->getNodeKind() << "\n";
                if(auto V = dyn_cast<StmtVFGNode>(node)) {
                    PAGNode* src = V->getPAGSrcNode();
                    PAGNode* dst = V->getPAGDstNode();
                    errs() << "(" << src->getNodeKind() << "," << dst->getNodeKind() << ") " << *V->getInst() << "\n";
                }
                for(VFGEdge* out : node->getOutEdges()) {
                    auto outNode = out->getDstNode();
                    if(outNode->getBB()->getParent() == &F) {
                        WorkList.insert(outNode);
                    }
                    else {
                        errs() << "out to function: " << outNode->getBB()->getParent()->getName() << "\n";
                    }
                }
            }

        }
    }
    errs() << "finished\n";
    return 0;
}