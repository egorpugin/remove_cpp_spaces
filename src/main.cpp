#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>
#include <fstream>
#include <iostream>

struct dump_tokens : clang::PreprocessorFrontendAction {
    enum class prev_line_type {
        no_prev_line,
        normal,
        pp,
    };

    void ExecuteAction() override {
        auto &PP = getCompilerInstance().getPreprocessor();
        auto &SM = PP.getSourceManager();

        llvm::MemoryBufferRef FromFile = SM.getBufferOrFake(SM.getMainFileID());
        clang::Lexer RawLex(SM.getMainFileID(), FromFile, SM, PP.getLangOpts());
        RawLex.SetKeepWhitespaceMode(true);

        clang::Token t;
        prev_line_type prev_line{};
        char prev_sym{};
        auto print_newline = [&](char c) {
            if (!prev_sym || prev_sym == '\n') {
                prev_sym = c;
                return;
            }
            prev_sym = c;
            std::cout << c;
        };
        while (1) {
            RawLex.LexFromRawLexer(t);
            if (t.isAnnotation()) {
                continue;
            }
            switch (t.getKind()) {
            case clang::tok::eof:
                return;
            case clang::tok::comment:
            case clang::tok::unknown:
                if (PP.getSpelling(t).contains('\n')) {
                    if (prev_line == prev_line_type::pp) {
                        print_newline('\n');
                    }
                    prev_line = prev_line_type::no_prev_line;
                }
                continue;
            case clang::tok::hash:
                if (prev_line == prev_line_type::no_prev_line) {
                    print_newline('\n');
                    prev_line = prev_line_type::pp;
                }
            default:
                if (prev_line == prev_line_type::no_prev_line) {
                    prev_line = prev_line_type::normal;
                }
                std::cout << PP.getSpelling(t);
                print_newline(' ');
                continue;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    std::ifstream ifile{"sw1.cpp"};
    std::stringstream buffer;
    buffer << ifile.rdbuf();
    clang::tooling::runToolOnCode(std::make_unique<dump_tokens>(), buffer.str().c_str());
}
