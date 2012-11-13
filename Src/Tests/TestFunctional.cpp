#include "Precompile.h"
#include "Core/MSVSDependencyParser.h"
#include "Core/DependencyView.h"
#include "Utils/FileDictionary.h"
#include "Core/DependencyDiff.h"

SUITE(FunctionalTest)
{
  TEST(FunctionalDiffTest)
  {
    /*
          1        2        3
        __|_      _|_       |
       |    |    |   |      A
       A    D    B   A      |
     __|__           |      C
    |     |          C      |
    B     C                 E
    */

    const char beforeFilename [] = "before.functional.test";
    {
      std::fstream file;
      file.open(beforeFilename, std::ios_base::out);
      file << "1>.cpp .h useless stuff" << std::endl;
      file << "1>------ Rebuild All started: Project: A, Configuration: Any Configuration " << std::endl;
      file << "1>InitializeBuildStatus:" << std::endl;
      file << "1>  1" << std::endl;
      file << "1>  Note: including file: A" << std::endl;
      file << "1>  Note: including file:  B" << std::endl;
      file << "1>  Note: including file:  C" << std::endl;
      file << "1>  Note: including file: D" << std::endl;
      file << "1>  2" << std::endl;
      file << "1>  Note: including file: B" << std::endl;
      file << "1>  Note: including file: A" << std::endl;
      file << "1>  Note: including file:  C" << std::endl;
      file << "1>  Generating Code..." << std::endl;
      file << "2>------ Rebuild All started: Project: B, Configuration: Any Configuration " << std::endl;
      file << "1>FinalizeBuildStatus:" << std::endl;
      file << "2>.cpp .h useless stuff" << std::endl;
      file << "2>InitializeBuildStatus:" << std::endl;
      file << "2>  3" << std::endl;
      file << "2>  Note: including file: A" << std::endl;
      file << "2>  Note: including file:  C" << std::endl;
      file << "2>  Note: including file:   E" << std::endl;
      file << "2>FinalizeBuildStatus:" << std::endl;
      file << "2>.cpp .h useless stuff" << std::endl;
      file.close();
    }
    /*
          1       3  4
        __|_      |  |
       |  | |     A  0
       A  0 D     |
     __|__        C
    |     |      
    B     C      
    */
    const char afterFilename [] = "after.functional.test";
    {
      std::fstream file;
      file.open(afterFilename, std::ios_base::out);
      file << "1>.cpp .h useless stuff" << std::endl;
      file << "1>------ Rebuild All started: Project: A, Configuration: Any Configuration " << std::endl;
      file << "1>InitializeBuildStatus:" << std::endl;
      file << "1>  1" << std::endl;
      file << "1>  Note: including file: A" << std::endl;
      file << "1>  Note: including file:  B" << std::endl;
      file << "1>  Note: including file:  C" << std::endl;
      file << "1>  Note: including file: 0" << std::endl;
      file << "1>  Note: including file: D" << std::endl;
      file << "2>------ Rebuild All started: Project: B, Configuration: Any Configuration " << std::endl;
      file << "1>FinalizeBuildStatus:" << std::endl;
      file << "2>.cpp .h useless stuff" << std::endl;
      file << "2>InitializeBuildStatus:" << std::endl;
      file << "2>  3" << std::endl;
      file << "2>  Note: including file: A" << std::endl;
      file << "2>  Note: including file:  C" << std::endl;
      file << "2>  4" << std::endl;
      file << "2>  Note: including file: 0" << std::endl;
      file << "2>PostBuildEvent:";
      file << "2>  Post_build_step" << std::endl;
      file << "2>FinalizeBuildStatus:" << std::endl;
      file << "2>  Finalize_build_step" << std::endl;
      file.close();
    }

    MSVSDependencyParser parser;
    DependencyGraph dependsBefore, dependsAfter;
    FileDictionary dict(FileDictionary::e_Compare_NoCase, 32u);
    parser.ParseDenendencies(beforeFilename, dependsBefore, dict);
    parser.ParseDenendencies(afterFilename, dependsAfter, dict);
    GroupedDependencyGraph viewBefore(16), viewAfter(16);
    DependencyView::Build(dependsBefore, dict, DependencyView::e_FilesCount, viewBefore);
    DependencyView::Build(dependsAfter, dict, DependencyView::e_FilesCount, viewAfter);
    Diff diff;
    DependencyDiff::MakeDiff(viewAfter, viewBefore, diff);
#define CHECK_NODE_NAME(name, handle) \
    { \
      std::string s; \
      CHECK_EQUAL(name, dict.GetFileName(handle, s)); \
    } \

    std::string filename;
    CHECK_EQUAL(2u, diff.m_Added.size());
    CHECK_NODE_NAME("LIBRARY\\B", diff.m_Added[0].fromFile);
    CHECK_NODE_NAME("LIBRARY\\B\\4", diff.m_Added[0].toFile);
    CHECK_EQUAL(2.0, diff.m_Added[0].weight);
    CHECK_EQUAL(1u, diff.m_Added[0].count);
    CHECK_NODE_NAME("LIBRARY\\A\\1", diff.m_Added[1].fromFile);
    CHECK_NODE_NAME("0", diff.m_Added[1].toFile);
    CHECK_EQUAL(1.0, diff.m_Added[1].weight);
    CHECK_EQUAL(1u, diff.m_Added[1].count);

    CHECK_EQUAL(2u, diff.m_Removed.size());
    CHECK_NODE_NAME("LIBRARY\\A", diff.m_Removed[0].fromFile);
    CHECK_NODE_NAME("LIBRARY\\A\\2", diff.m_Removed[0].toFile);
    CHECK_EQUAL(4.0, diff.m_Removed[0].weight);
    CHECK_EQUAL(1, diff.m_Removed[0].count);
    CHECK_NODE_NAME("C", diff.m_Removed[1].fromFile);
    CHECK_NODE_NAME("E", diff.m_Removed[1].toFile);
    CHECK_EQUAL(1.0, diff.m_Removed[1].weight);
    CHECK_EQUAL(1, diff.m_Removed[1].count);
  }
}
