#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>
#include <fstream>
#include <iostream>

using namespace clang;

void print(auto &&x) {
    std::cout << x;
}

struct dump_tokens : PreprocessorFrontendAction {
    enum class space_type {
        start_of_file,
        none,
        hash,
        space,
        newline,
    };
    enum class prev_line_type {
        no_prev_line,
        normal,
        pp,
    };

    void ExecuteAction() override {
        Preprocessor &PP = getCompilerInstance().getPreprocessor();
        SourceManager &SM = PP.getSourceManager();

        // Start lexing the specified input file.
        llvm::MemoryBufferRef FromFile = SM.getBufferOrFake(SM.getMainFileID());
        Lexer RawLex(SM.getMainFileID(), FromFile, SM, PP.getLangOpts());
        RawLex.SetKeepWhitespaceMode(true);

        Token t;
        space_type space{};
        prev_line_type prev_line{};
        char prev_sym{};
        auto print_newline = [&](char c) {
            if (!prev_sym || prev_sym == '\n') {
                prev_sym = c;
                return;
            }
            prev_sym = c;
            print(c);
        };
        while (1) {
            RawLex.LexFromRawLexer(t);
            if (t.isAnnotation()) {
                continue;
            }
            switch (t.getKind()) {
            case tok::eof:
                return;
            case tok::comment:
            case tok::unknown:
                if (PP.getSpelling(t).contains('\n')) {
                    if (prev_line == prev_line_type::pp) {
                        print_newline('\n');
                    }
                    prev_line = prev_line_type::no_prev_line;
                }
                continue;
            case tok::hash:
                if (prev_line == prev_line_type::no_prev_line) {
                    print_newline('\n');
                    prev_line = prev_line_type::pp;
                }
            default:
                if (prev_line == prev_line_type::no_prev_line) {
                    prev_line = prev_line_type::normal;
                }
                print(PP.getSpelling(t));
                print_newline(' ');
                continue;
            }

            /*while (!t.is(tok::eof) && (false
                || t.is(clang::tok::TokenKind::comment)
                || t.is(clang::tok::TokenKind::unknown)
                )) {
                RawLex.LexFromRawLexer(t);
            }
            if (t.is(tok::eof)) {
                break;
            }
            if (!t.getFlag(clang::Token::TokenFlags::StartOfLine)) {
                throw std::runtime_error{"not a line start"};
            }

            prev_line = t.is(clang::tok::TokenKind::hash) ? prev_line_type::pp : prev_line_type::normal;

            print(PP.getSpelling(t));
            print(" ");
            RawLex.LexFromRawLexer(t);

            while (!t.getFlag(clang::Token::TokenFlags::StartOfLine) && !t.is(tok::eof)) {
                if (false
                    || t.is(clang::tok::TokenKind::comment)
                    || t.is(clang::tok::TokenKind::unknown)
                ) {
                    RawLex.LexFromRawLexer(t);
                    continue;
                }
                print(PP.getSpelling(t));
                print(" ");
                RawLex.LexFromRawLexer(t);
            }
            if (t.is(tok::eof)) {
                break;
            }
            if (prev_line == prev_line_type::pp) {
                print("\n");
                prev_line = prev_line_type::no_prev_line;
            }
            print(PP.getSpelling(t));
            print(" ");
            RawLex.LexFromRawLexer(t);*/
        }
    }
};

int main(int argc, char *argv[]) {
    std::ifstream ifile{"sw1.cpp"};
    std::stringstream buffer;
    buffer << ifile.rdbuf();
    clang::tooling::runToolOnCode(std::make_unique<dump_tokens>(), buffer.str().c_str());
}
