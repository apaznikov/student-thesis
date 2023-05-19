//==============================================================================
// FILE:
//    CodeRefactor.cpp
//
// DESCRIPTION: CodeRefactor will rename a specified member method in a class
// (or a struct) and in all classes derived from it. It will also update all
// call sites in which the method is used so that the code remains semantically
// correct. For example we can use CodeRefactor to rename Base::foo as
// Base::bar.
//
// USAGE:
//    1. As a loadable Clang plugin:
//      clang -cc1 -load <BUILD_DIR>/lib/libCodeRefactor.dylib  -plugin  '\'
//      CodeRefactor -plugin-arg-CodeRefactor -class-name '\'
//      -plugin-arg-CodeRefactor Base  -plugin-arg-CodeRefactor -old-name '\'
//      -plugin-arg-CodeRefactor run  -plugin-arg-CodeRefactor -new-name '\'
//      -plugin-arg-CodeRefactor walk test/CodeRefactor_Class.cpp
//    2. As a standalone tool:
//       <BUILD_DIR>/bin/ct-code-refactor --class-name=Base --new-name=walk '\'
//        --old-name=run test/CodeRefactor_Class.cpp
//
// License: The Unlicense
//==============================================================================
#include "CodeRefactor.h"

#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Tooling/Refactoring/Rename/RenamingAction.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <stdio.h>
#include <stdint.h>

using namespace clang;
using namespace ast_matchers;

//-----------------------------------------------------------------------------
// CodeRefactorMatcher - implementation
//-----------------------------------------------------------------------------
void CodeRefactorMatcher::run(const MatchFinder::MatchResult &Result) {

    FILE *file = fopen("../../test/mallocfile.txt", "a");
    const CallExpr *callMalloc =
            Result.Nodes.getNodeAs<clang::CallExpr>("functionCallDeclaration");

    if (callMalloc) {
        std::string s = callMalloc->getBeginLoc().printToString(Result.Context->getSourceManager());
        int start_index = s.find_first_of(":") + 1;
        int end_index = s.find_last_of(":");
        fprintf(file, "%s ", s.substr(start_index, end_index - start_index).c_str());
    }

    const VarDecl *callMalloc1 =
            Result.Nodes.getNodeAs<clang::VarDecl>("declaration");

    if (callMalloc1) {
        callMalloc1->getType().dump();
        callMalloc1->printName(llvm::outs());
        fprintf(file, "%s\n", callMalloc1->getNameAsString().c_str());
    }

    const CallExpr *callMalloc2 =
            Result.Nodes.getNodeAs<clang::CallExpr>("functionCallAssigment");

    if (callMalloc2) {
        callMalloc2->getBeginLoc().dump(Result.Context->getSourceManager());
        std::string s = callMalloc2->getBeginLoc().printToString(Result.Context->getSourceManager());
        int start_index = s.find_first_of(":") + 1;
        int end_index = s.find_last_of(":");
        fprintf(file, "%s ", s.substr(start_index, end_index - start_index).c_str());
    }

    const BinaryOperator *callMalloc3 =
            Result.Nodes.getNodeAs<clang::BinaryOperator>("assignment");

    if (callMalloc3) {
        if (const auto *DRE = dyn_cast<DeclRefExpr>(callMalloc3->getLHS())) {
            const auto DREType = DRE->getDecl()->getType();
            DREType.dump();
            DRE->getDecl()->printName(llvm::outs());
            fprintf(file, "%s\n", DRE->getDecl()->getNameAsString().c_str());
        }
    }
    fclose(file);
}

void CodeRefactorMatcher::onEndOfTranslationUnit() {
  // Output to stdout
  CodeRefactorRewriter
      .getEditBuffer(CodeRefactorRewriter.getSourceMgr().getMainFileID())
      .write(llvm::outs());
}

CodeRefactorASTConsumer::CodeRefactorASTConsumer(Rewriter &R)
    : CodeRefactorHandler(R)
    {

        const auto mallocDeclaration = callExpr(
                callee(functionDecl(hasName("malloc"))),
                hasAncestor(
                        varDecl().bind("declaration"))
        ).bind("functionCallDeclaration");

        Finder.addMatcher(mallocDeclaration, &CodeRefactorHandler);

        const auto mallocAssignment = callExpr(
                callee(functionDecl(hasName("malloc"))),
                hasAncestor(
                        binaryOperator(hasOperatorName("=")).bind("assignment")
                )
        ).bind("functionCallAssigment");

        Finder.addMatcher(mallocAssignment, &CodeRefactorHandler);

}

//-----------------------------------------------------------------------------
// FrontendAction
//-----------------------------------------------------------------------------
class CodeRefactorAddPluginAction : public PluginASTAction {
public:
  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    // Example error handling.
    DiagnosticsEngine &D = CI.getDiagnostics();
    return true;
  }
  static void PrintHelp(llvm::raw_ostream &ros) {
    ros << "Help for CodeRefactor plugin goes here\n";
  }

  // Returns our ASTConsumer per translation unit.
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    RewriterForCodeRefactor.setSourceMgr(CI.getSourceManager(),
                                         CI.getLangOpts());
    return std::make_unique<CodeRefactorASTConsumer>(RewriterForCodeRefactor);
  }

private:
  Rewriter RewriterForCodeRefactor;
};

//-----------------------------------------------------------------------------
// Registration
//-----------------------------------------------------------------------------
static FrontendPluginRegistry::Add<CodeRefactorAddPluginAction>
    X(/*Name=*/"CodeRefactor",
      /*Desc=*/"Change the name of a class method");
