void build(Solution &s) {
    auto &remove_cpp_spaces = s.addExecutable("remove_cpp_spaces");
    {
        auto &t = remove_cpp_spaces;
        t.PackageDefinitions = true;
        t += cpplatest;
        t += "src/.*"_rr;
        //t += "pub.egorpugin.primitives.sw.main"_dep;
        t += "pub.egorpugin.primitives.templates2"_dep;
        t += "org.sw.demo.llvm_project.clang.tools.libclang"_dep;
    }
}
