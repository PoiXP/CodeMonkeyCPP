#include "Precompile.h"

#include "Core/CodeBuild.h"
#include "Core/MSVSDependencyParser.h"
#include "Core/ProjectDependencyView.h"
#include "Core/DependencyView.h"
#include "Utils/FileDictionary.h"

struct ModuleDepends
{
  size_t moduleIndex;
  size_t dependenciesCount;
  struct Dependency
  {
    size_t moduleIdx;
    size_t fileCount;
    size_t files[16];
  } dependency[8];
};

bool SortDepends(const ModuleDepends& a, const ModuleDepends& b) { return a.moduleIndex < b.moduleIndex; } 


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
      file << "1>  Note: including file: " << codeDir.string() << "\\projecta\\code\\..\\..\\ProjectB\\Message.h" << std::endl;
      file << "1>  Note: including file: EXTERNAL_PATH\\include\\stdio.h" << std::endl;
      file << "1>  Note: including file:  EXTERNAL_PATH\\include\\crtdefs.h" << std::endl;
      file << "1>  Note: including file:   EXTERNAL_PATH\\include\\sal.h" << std::endl;
      file << "1>  Note: including file:    EXTERNAL_PATH\\include\\codeanalysis\\sourceannotations.h" << std::endl;
      file << "1>  Note: including file:   EXTERNAL_PATH\\include\\vadefs.h" << std::endl;
      file << "1>  Note: including file:  EXTERNAL_PATH\\include\\swprintf.inl" << std::endl;
      file << "1>FinalizeBuildStatus:" << std::endl;
      file << "2>------ Rebuild All started: Project: ProjectB, Configuration: Debug Win32 ------" << std::endl;
      file << "2>InitializeBuildStatus:" << std::endl;
      file << "2>  Message.cpp" << std::endl;
      file << "2>  Note: including file: " << codeDir.string() << "\\projectb\\Message.h" << std::endl;
      file << "2>FinalizeBuildStatus:" << std::endl;
      file.close();
    }

    int returnCode = parser.ParseDenendencies(filename, dependencies, dict);
    CHECK_EQUAL(MSVSDependencyParser::e_OK, returnCode);

    boost::filesystem::path solutionDir = codeDir;
    solutionDir /= "Test.sln";

    CodeBuild* build = CodeBuild::Create(CodeBuild::e_MSVSSolution, "..\\..\\Src\\Tests\\TestData\\Test.sln");
    build->CreateMapping(dict);

    size_t mainHandle = dict.FindHandle("LIBRARY\\ProjectA\\main.cpp");
    size_t appHandle = dict.FindHandle("LIBRARY\\ProjectA\\App.cpp");
    size_t messageHandle = dict.FindHandle("LIBRARY\\ProjectB\\Message.cpp");
    
    std::string dir = codeDir.string();
    size_t appHeaderHandle = dict.FindHandle(dir + "\\projecta\\code\\App.h");
    size_t definesHanlde = dict.FindHandle( dir + "\\projecta\\code\\defines.h");
    size_t messageHeaderHandle = dict.FindHandle( dir + "\\projectb\\Message.h");
    size_t externalHandle = dict.FindHandle("EXTERNAL_PATH\\include\\sal.h");
    static const size_t EXTERNAL_MODULE = 0xFFFFFFFF;

    CHECK_EQUAL("ProjectA", build->GetModuleName(0) );
    CHECK_EQUAL("ProjectB", build->GetModuleName(1) );
    CHECK_EQUAL("EXTERNAL", build->GetModuleName(0xFFFFFFFF) );

    CHECK_EQUAL(0, build->GetFileModule(mainHandle));
    CHECK_EQUAL(0, build->GetFileModule(appHandle));
    CHECK_EQUAL(0, build->GetFileModule(appHeaderHandle));
    CHECK_EQUAL(0, build->GetFileModule(definesHanlde));
    CHECK_EQUAL(1, build->GetFileModule(messageHandle));
    CHECK_EQUAL(1, build->GetFileModule(messageHeaderHandle));
    CHECK_EQUAL(EXTERNAL_MODULE, build->GetFileModule(externalHandle));

    std::string name;
    CHECK_EQUAL("ProjectA\\main.cpp",  build->ResolveName(dict, mainHandle, name));
    CHECK_EQUAL("ProjectA\\App.cpp",   build->ResolveName(dict, appHandle, name));
    CHECK_EQUAL("ProjectA\\App.h",     build->ResolveName(dict, appHeaderHandle, name));
    CHECK_EQUAL("ProjectA\\defines.h", build->ResolveName(dict, definesHanlde, name));
    CHECK_EQUAL("ProjectB\\Message.h", build->ResolveName(dict, messageHeaderHandle, name));
    CHECK_EQUAL("ProjectB\\Message.cpp", build->ResolveName(dict, messageHandle, name));
    CHECK_EQUAL("EXTERNAL_PATH\\include\\sal.h", build->ResolveName(dict, externalHandle, name));

    GroupedDependencyGraph groupedGraph(32);
    DependencyView::Build(dependencies, dict, DependencyView::e_FilesCount, groupedGraph);

    ProjectDependencyGraph projectDependency(23);
    ProjectDependencyView::Build(groupedGraph, build, projectDependency);

    const ModuleDepends module[3] =
    {
      { 
        0, 3,
        { 
          { 0, 3,{mainHandle, appHandle, appHeaderHandle, definesHanlde} }, 
          { 1, 1, {messageHeaderHandle} },
          { EXTERNAL_MODULE, 1, {externalHandle} },
        }
      },
      {
        1, 1,
        {
          {1, 2, {messageHeaderHandle, messageHandle}}
        }
      },
      {
        EXTERNAL_MODULE, 1,
        {
          {EXTERNAL_MODULE, 1, {externalHandle}}
        }
      }
    };
    static const int MODULES_COUNT = sizeof(module) / sizeof(module[0]);

    ProjectDependencyGraph::Node* head = projectDependency.GetHead();
    CHECK_EQUAL(MODULES_COUNT, head->GetChildren().GetCount());

    ModuleDepends parsedModules[MODULES_COUNT];

    int moduleIdx = 0;
    for (size_t nodeIdx = 0; nodeIdx < MODULES_COUNT; ++nodeIdx)
    {
      ProjectDependencyGraph::Node* project = head->GetChildren().GetNode(nodeIdx);
      parsedModules[moduleIdx].moduleIndex = project->GetData().moduleIndex;
      size_t& dependCount = parsedModules[moduleIdx].dependenciesCount;
      dependCount = 0;
      for (size_t childIdx = 0; childIdx < project->GetChildren().GetCount(); childIdx++)
      {
        auto linkMap = project->GetChildren().GetLink(childIdx).fileDependencies;
        for (auto linkIt = linkMap.begin(); linkIt != linkMap.end(); ++linkIt)
        {
          ModuleDepends::Dependency& depend = parsedModules[moduleIdx].dependency[dependCount];
          dependCount += 1;

          depend.fileCount = 0;
          depend.moduleIdx = linkIt->first;
          
          for (auto fileIt = linkIt->second.begin(); fileIt != linkIt->second.end(); ++fileIt)
          {
            depend.files[ depend.fileCount++] = *fileIt;
          }

        }
      }

      moduleIdx += 1;
    }

    std::sort(parsedModules, parsedModules + MODULES_COUNT, SortDepends);
    for (size_t i=0; i < MODULES_COUNT; ++i)
    {
      for (size_t k=0; k < parsedModules[i].dependenciesCount; ++k)
      {
        std::sort(parsedModules[i].dependency[k].files, 
                  parsedModules[i].dependency[k].files + parsedModules[i].dependency[k].fileCount);
      }
    }
    
    for (size_t moduleIdx=0; moduleIdx < MODULES_COUNT; ++moduleIdx)
    {
      const ModuleDepends& moduleRef = module[moduleIdx];
      const ModuleDepends& moduleParsed = parsedModules[moduleIdx];

      CHECK_EQUAL(moduleRef.moduleIndex, moduleParsed.moduleIndex);
      CHECK_EQUAL(moduleRef.dependenciesCount, moduleParsed.dependenciesCount);
      for (size_t dependIdx=0; dependIdx < moduleRef.dependenciesCount; ++dependIdx)
      {
        const ModuleDepends::Dependency& dependRef = moduleRef.dependency[dependIdx];
        const ModuleDepends::Dependency& dependParsed = moduleParsed.dependency[dependIdx];

        CHECK_EQUAL(dependRef.fileCount, dependParsed.fileCount);
        for(size_t fileIdx = 0; fileIdx < dependRef.fileCount; ++fileIdx)
        {
          CHECK_EQUAL(dependRef.files[fileIdx], dependParsed.files[fileIdx]);
        }
      }
    }
    
    delete build;
  }
}
