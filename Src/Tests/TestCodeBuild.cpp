#include "Precompile.h"

#include "Core/CodeBuild.h"
#include "Core/MSVSDependencyParser.h"
#include "Core/CodeBuild.h"
#include "Utils/FileDictionary.h"

SUITE(TestCodeBuild)
{
  TEST(TestFunctional)
  {
    MSVSDependencyParser parser;
    FileDictionary dict(FileDictionary::e_Compare_NoCase, 32u);
    DependencyGraph dependencies;

    boost::filesystem::path codeDir = boost::filesystem::absolute("..\\..\\Src\\Tests\\TestData");
    codeDir.normalize();

    const char filename[] = "CodeBuild.test";
    {

      std::fstream file;
      file.open(filename, std::ios_base::out);
      file << "1>------ Rebuild All started: Project: ProjectA, Configuration: Debug Win32 ------" << std::endl;
      file << "1>InitializeBuildStatus:" << std::endl;
      file << "1>  main.cpp" << std::endl;
      file << "1>  Note: including file: " << codeDir.string() << "\\projecta\\code\\App.h" << std::endl;
      file << "1>  App.cpp" << std::endl;
      file << "1>  Note: including file: " << codeDir.string() << "\\projecta\\code\\App.h" << std::endl;
      file << "1>  Note: including file: " << codeDir.string() << "\\projecta\\code\\defines.h" << std::endl;
      file << "1>  Note: including file: EXTERNAL_PATH\\include\\stdio.h" << std::endl;
      file << "1>  Note: including file:  EXTERNAL_PATH\\include\\crtdefs.h" << std::endl;
      file << "1>  Note: including file:   EXTERNAL_PATH\\include\\sal.h" << std::endl;
      file << "1>  Note: including file:    EXTERNAL_PATH\\include\\codeanalysis\\sourceannotations.h" << std::endl;
      file << "1>  Note: including file:   EXTERNAL_PATH\\include\\vadefs.h" << std::endl;
      file << "1>  Note: including file:  EXTERNAL_PATH\\include\\swprintf.inl" << std::endl;
      file << "1>FinalizeBuildStatus:" << std::endl;
      file.close();
    }

    int returnCode = parser.ParseDenendencies(filename, dependencies, dict);
    CHECK_EQUAL(MSVSDependencyParser::e_OK, returnCode);

    boost::filesystem::path solutionDir = codeDir;
    solutionDir /= "Test.sln";

    CodeBuild* build = CodeBuild::Create(CodeBuild::e_MSVSSolution, "..\\..\\Src\\Tests\\TestData\\Test.sln");
    build->CreateMapping(dict);

    size_t mainHanlde = dict.FindHandle("LIBRARY\\ProjectA\\main.cpp");
    size_t appHanlde = dict.FindHandle("LIBRARY\\ProjectA\\App.cpp");
    
    std::string dir = codeDir.string();
    size_t appHeaderHanlde = dict.FindHandle(dir + "\\projecta\\code\\App.h");
    size_t definesHanlde = dict.FindHandle( dir + "\\projecta\\code\\defines.h");
    
    size_t externalHandle = dict.FindHandle("EXTERNAL_PATH\\include\\sal.h");
    static const size_t EXTERNAL_MODULE = 0xFFFFFFFF;

    CHECK_EQUAL("ProjectA", build->GetModuleName(0) );
    CHECK_EQUAL("EXTERNAL", build->GetModuleName(0xFFFFFFFF) );

    CHECK_EQUAL(0, build->GetFileModule(mainHanlde));
    CHECK_EQUAL(0, build->GetFileModule(appHanlde));
    CHECK_EQUAL(0, build->GetFileModule(appHeaderHanlde));
    CHECK_EQUAL(0, build->GetFileModule(definesHanlde));
    CHECK_EQUAL(EXTERNAL_MODULE, build->GetFileModule(externalHandle));

    std::string name;
    CHECK_EQUAL("ProjectA\\main.cpp",  build->ResolveName(dict, mainHanlde, name));
    CHECK_EQUAL("ProjectA\\App.cpp",   build->ResolveName(dict, appHanlde, name));
    CHECK_EQUAL("ProjectA\\App.h",     build->ResolveName(dict, appHeaderHanlde, name));
    CHECK_EQUAL("ProjectA\\defines.h", build->ResolveName(dict, definesHanlde, name));
    CHECK_EQUAL("EXTERNAL_PATH\\include\\sal.h", build->ResolveName(dict, externalHandle, name));

    delete build;
  }
}
