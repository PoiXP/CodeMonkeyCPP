#include <UnitTest++\src\UnitTest++.h>
#include <Core/MSVCDependencyParser.h>
#include <Core/DependencyGraph.h>
#include <Utils/FileDictionary.h>
#include <fstream>

SUITE(MSVCDependencyParserTest)
{
  TEST(testParsing_1)
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

    const char filename [] = "log.test";
    std::fstream file;
    file.open(filename, std::ios_base::out);
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
    file << "2>FinalizeBuildStatus:" << std::endl;
    file << "2>.cpp .h useless stuff" << std::endl;
    file << "2>------ Rebuild All started: Project: B, Configuration: Any Configuration " << std::endl;
    file << "2>InitializeBuildStatus:" << std::endl;
    file << "2>  3" << std::endl;
    file << "2>  Note: including file: A" << std::endl;
    file << "2>  Note: including file:  C" << std::endl;
    file << "2>  Note: including file:   E" << std::endl;
    file << "2>PostBuildEvent:" << std::endl;
    file << "2>  Post_build_step" << std::endl;
    file << "2>FinalizeBuildStatus:" << std::endl;
    file << "2>  Finalize_build_step" << std::endl;
    file.close();

    MSVCDependencyParser parser;
    DependencyGraph depends;
    FileDictionary dict(FileDictionary::e_Compare_NoCase, 32u);
    int code = parser.ParseDenendencies(filename, depends, dict);
    CHECK_EQUAL(MSVCDependencyParser::e_OK, code);

    CHECK_EQUAL(2u, depends.GetHead()->GetChildren().GetCount());
    CHECK_EQUAL(0u, depends.GetHead()->GetRefBy().GetCount());

#define CHECK_NODE(nodePtr, node_name, childrenNo) \
  { \
    std::string s; \
    CHECK_EQUAL(node_name, dict.GetFileName(nodePtr->GetData().fileHandle, s)); \
    CHECK_EQUAL(childrenNo, nodePtr->GetChildren().GetCount()); \
  }\

    const DependencyGraph::Node* libA = depends.GetHead()->GetChildren().GetNode(0u);
    CHECK_NODE(libA, "LIBRARY\\A", 2)
    {
      const DependencyGraph::Node* node1 = libA->GetChildren().GetNode(0u);
      CHECK_NODE(node1, "1", 2);
      {
        const DependencyGraph::Node* nodeA = node1->GetChildren().GetNode(0u);
        CHECK_NODE(nodeA, "A", 2);
        const DependencyGraph::Node* nodeB = nodeA->GetChildren().GetNode(0u);
        CHECK_NODE(nodeB, "B", 0);
        const DependencyGraph::Node* nodeC = nodeA->GetChildren().GetNode(1u);
        CHECK_NODE(nodeC, "C", 0);
        const DependencyGraph::Node* nodeD = node1->GetChildren().GetNode(1u);
        CHECK_NODE(nodeD, "D", 0);
      }
      const DependencyGraph::Node* node2 = libA->GetChildren().GetNode(1u);
      CHECK_NODE(node2, "2", 2);
      {
        const DependencyGraph::Node* nodeB = node2->GetChildren().GetNode(0u);
        CHECK_NODE(nodeB, "B", 0);
        const DependencyGraph::Node* nodeA = node2->GetChildren().GetNode(1u);
        CHECK_NODE(nodeA, "A", 1);
        const DependencyGraph::Node* nodeC = nodeA->GetChildren().GetNode(0u);
        CHECK_NODE(nodeC, "C", 0);
      }
    }
    const DependencyGraph::Node* libB = depends.GetHead()->GetChildren().GetNode(1u);
    CHECK_NODE(libB, "LIBRARY\\B", 1)
    {
      const DependencyGraph::Node* node3 = libB->GetChildren().GetNode(0u);;
      CHECK_NODE(node3, "3", 1);
      {
        const DependencyGraph::Node* nodeA = node3->GetChildren().GetNode(0u);
        CHECK_NODE(nodeA, "A", 1);
        const DependencyGraph::Node* nodeC = nodeA->GetChildren().GetNode(0u);
        CHECK_NODE(nodeC, "C", 1);
        const DependencyGraph::Node* nodeE = nodeC->GetChildren().GetNode(0u);
        CHECK_NODE(nodeE, "E", 0);
      }
    }
  }
}
