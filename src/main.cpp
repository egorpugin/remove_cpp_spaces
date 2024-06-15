#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>
#include <fstream>
#include <iostream>

using namespace clang;

void print(auto &&x) {
    std::cout << x;
}
void println(auto &&x) {
    print(x);
    print("\n");
}

struct dump_tokens : PreprocessorFrontendAction {
    std::string space;

    void ExecuteAction() override {
        Preprocessor &PP = getCompilerInstance().getPreprocessor();
        SourceManager &SM = PP.getSourceManager();

        // Start lexing the specified input file.
        llvm::MemoryBufferRef FromFile = SM.getBufferOrFake(SM.getMainFileID());
        Lexer RawLex(SM.getMainFileID(), FromFile, SM, PP.getLangOpts());
        RawLex.SetKeepWhitespaceMode(true);

        Token RawTok;
        RawLex.LexFromRawLexer(RawTok);
        while (RawTok.isNot(tok::eof)) {
            //PP.DumpToken(RawTok, true);
            //DumpToken(PP, RawTok, true);
            DumpTokenMy(PP, RawTok);
            //llvm::errs() << "\n";
            RawLex.LexFromRawLexer(RawTok);
        }
    }
    void DumpTokenMy(Preprocessor &PP, const Token &Tok) {
        if (Tok.isAnnotation()) {
            return;
        }
        if (Tok.getKind() == clang::tok::TokenKind::comment) {
            return;
        }
        if (Tok.getKind() == clang::tok::TokenKind::unknown) {
            space += PP.getSpelling(Tok);
            return;
        }
        if (!space.empty()) {
            if (space.contains('\n')) {
                print("\n");
            } else {
                print(" ");
            }
            space.clear();
        }
        print(PP.getSpelling(Tok));
    }
    void DumpToken(Preprocessor &PP, const Token &Tok, bool DumpFlags) const {
      llvm::errs() << tok::getTokenName(Tok.getKind());

      if (!Tok.isAnnotation())
        llvm::errs() << " '" << PP.getSpelling(Tok) << "'";

      if (!DumpFlags) return;

      llvm::errs() << "\t";
      if (Tok.isAtStartOfLine())
        llvm::errs() << " [StartOfLine]";
      if (Tok.hasLeadingSpace())
        llvm::errs() << " [LeadingSpace]";
      if (Tok.isExpandDisabled())
        llvm::errs() << " [ExpandDisabled]";
      if (Tok.needsCleaning()) {
        const char *Start = PP.getSourceManager().getCharacterData(Tok.getLocation());
        llvm::errs() << " [UnClean='" << StringRef(Start, Tok.getLength())
                     << "']";
      }

      llvm::errs() << "\tLoc=<";
      PP.DumpLocation(Tok.getLocation());
      llvm::errs() << ">";
    }
};

int main(int argc, char *argv[]) {
    std::ifstream ifile{"sw1.cpp"};
    std::stringstream buffer;
    buffer << ifile.rdbuf();
    clang::tooling::runToolOnCode(std::make_unique<dump_tokens>(), buffer.str().c_str());
}
