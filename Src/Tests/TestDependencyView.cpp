#include "Precompile.h"
#include "Core/DependencyView.h"
#include "Utils/FileDictionary.h"

SUITE(TestDependencyView)
{
  TEST(testGroupByCompiledFilesNumber)
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

    DependencyGraph depends;
    NodeData data;
    data.fileHandle = 0x1;
    DependencyGraph::Node* node1 = depends.CreateNode(data);
    data.fileHandle = 0x2;
    DependencyGraph::Node* node2 = depends.CreateNode(data);
    data.fileHandle = 0x3;
    DependencyGraph::Node* node3 = depends.CreateNode(data);
    data.fileHandle = 0xA;
    DependencyGraph::Node* nodeA = depends.CreateNode(data);
    data.fileHandle = 0xB;
    DependencyGraph::Node* nodeB = depends.CreateNode(data);
    data.fileHandle = 0xC;
    DependencyGraph::Node* nodeC = depends.CreateNode(data);
    data.fileHandle = 0xD;
    DependencyGraph::Node* nodeD = depends.CreateNode(data);
    data.fileHandle = 0xE;
    DependencyGraph::Node* nodeE = depends.CreateNode(data);
    data.fileHandle = 0xA;
    DependencyGraph::Node* nodeA2 = depends.CreateNode(data);
    data.fileHandle = 0xA;
    DependencyGraph::Node* nodeA3 = depends.CreateNode(data);
    data.fileHandle = 0xC;
    DependencyGraph::Node* nodeC2 = depends.CreateNode(data);
    
    depends.AddDependency(depends.GetHead(), node1);
    depends.AddDependency(node1, nodeA);
    depends.AddDependency(nodeA, nodeB);
    depends.AddDependency(nodeA, nodeC);
    depends.AddDependency(node1, nodeD);
    depends.AddDependency(depends.GetHead(), node2);
    depends.AddDependency(node2, nodeB);
    depends.AddDependency(node2, nodeA2);
    depends.AddDependency(nodeA2, nodeC);
    depends.AddDependency(depends.GetHead(), node3);
    depends.AddDependency(node3, nodeA3);
    depends.AddDependency(nodeA3, nodeC2);
    depends.AddDependency(nodeC2, nodeE);

    GroupedDependencyGraph sortedDepends(depends.GetNodeCount());
    FileDictionary dict(FileDictionary::e_Compare_NoCase);
    DependencyView::Build(depends, dict, DependencyView::e_FilesCount, sortedDepends);
    DependencyView::Sort(sortedDepends);
    /*
      Result
          1(5)     2(4)     3(4)
        __|_      _|_       |
       |    |    |   |      A(3)
       A(3) D    B   A(2)   |
     __|__           |      C(2)
    |     |          C      |
    B     C                 E


      1 2 3                                        A(3/3)  A   2       1     1 
      |_|_|                                        |        \ /        |     |
        A(3/8)       1(1/5)     2(1/4)     3(1/4)  C(3/4)    B(2/2)    D(1)  E(1)
      __|__        __|__      __|__        |       |
     |     |      |     |    |     |       |       |
(1/1)B     C(3/4) A(1/3)D(1) A(1/2)B(1)    A(1/3)  E(1)
    */


    GroupedDependencyGraph::Node* head = sortedDepends.GetHead();
    CHECK_EQUAL(8u, head->GetChildren().GetCount());
    // sorting
    CHECK_EQUAL(0xA, head->GetChildren().GetNode(0u)->GetData().fileHandle);
    CHECK_EQUAL(0x1, head->GetChildren().GetNode(1u)->GetData().fileHandle);
    CHECK_EQUAL(0x2, head->GetChildren().GetNode(2u)->GetData().fileHandle);
    CHECK_EQUAL(0x3, head->GetChildren().GetNode(3u)->GetData().fileHandle);
    CHECK_EQUAL(0xC, head->GetChildren().GetNode(4u)->GetData().fileHandle);
    CHECK_EQUAL(0xB, head->GetChildren().GetNode(5u)->GetData().fileHandle);
    CHECK_EQUAL(0xD, head->GetChildren().GetNode(6u)->GetData().fileHandle);
    CHECK_EQUAL(0xE, head->GetChildren().GetNode(7u)->GetData().fileHandle);
    
    // A
    CHECK_EQUAL(8.0, head->GetChildren().GetNode(0u)->GetData().totalWeight);
    CHECK_EQUAL(3.0, head->GetChildren().GetNode(0u)->GetData().myTotalWeight);
    CHECK_EQUAL(3,   head->GetChildren().GetLink(0u).count);
    {
      GroupedDependencyGraph::Node* nodeA = head->GetChildren().GetNode(0u);
      // Ref by
      CHECK_EQUAL(3u, nodeA->GetRefBy().GetCount());

      CHECK_EQUAL(0x1, nodeA->GetRefBy().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(1, nodeA->GetRefBy().GetLink(0u).count);

      CHECK_EQUAL(0x2, nodeA->GetRefBy().GetNode(1u)->GetData().fileHandle);
      CHECK_EQUAL(1, nodeA->GetRefBy().GetLink(1u).count);

      CHECK_EQUAL(0x3, nodeA->GetRefBy().GetNode(2u)->GetData().fileHandle);
      CHECK_EQUAL(1, nodeA->GetRefBy().GetLink(2u).count);
      // Children
      CHECK_EQUAL(2u, nodeA->GetChildren().GetCount());

      CHECK_EQUAL(0xC, nodeA->GetChildren().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(4.0, nodeA->GetChildren().GetNode(0u)->GetData().totalWeight);
      CHECK_EQUAL(3.0, nodeA->GetChildren().GetNode(0u)->GetData().myTotalWeight);
      CHECK_EQUAL(3,   nodeA->GetChildren().GetLink(0u).count);

      CHECK_EQUAL(0xB, nodeA->GetChildren().GetNode(1u)->GetData().fileHandle);
      CHECK_EQUAL(1.0, nodeA->GetChildren().GetNode(1u)->GetData().totalWeight);
      CHECK_EQUAL(1.0, nodeA->GetChildren().GetNode(1u)->GetData().myTotalWeight);
      CHECK_EQUAL(1,   nodeA->GetChildren().GetLink(1u).count);
    }

    // 1
    CHECK_EQUAL(0x1, head->GetChildren().GetNode(1u)->GetData().fileHandle);
    CHECK_EQUAL(5.0, head->GetChildren().GetNode(1u)->GetData().totalWeight);
    CHECK_EQUAL(1.0, head->GetChildren().GetNode(1u)->GetData().myTotalWeight);
    {
      GroupedDependencyGraph::Node* node1 = head->GetChildren().GetNode(1u);
      CHECK_EQUAL(2u, node1->GetChildren().GetCount());

      CHECK_EQUAL(0xA, node1->GetChildren().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(3.0, node1->GetChildren().GetNode(0u)->GetData().totalWeight);
      CHECK_EQUAL(1.0, node1->GetChildren().GetNode(0u)->GetData().myTotalWeight);
      CHECK_EQUAL(1,   node1->GetChildren().GetLink(0u).count);

      CHECK_EQUAL(0xD, node1->GetChildren().GetNode(1u)->GetData().fileHandle);
      CHECK_EQUAL(1.0, node1->GetChildren().GetNode(1u)->GetData().totalWeight);
      CHECK_EQUAL(1.0, node1->GetChildren().GetNode(1u)->GetData().myTotalWeight);
      CHECK_EQUAL(1,   node1->GetChildren().GetLink(1u).count);
    }
    // 2
    CHECK_EQUAL(0x2, head->GetChildren().GetNode(2u)->GetData().fileHandle);
    CHECK_EQUAL(4.0, head->GetChildren().GetNode(2u)->GetData().totalWeight);
    CHECK_EQUAL(1.0, head->GetChildren().GetNode(2u)->GetData().myTotalWeight);
    {
      GroupedDependencyGraph::Node* node2 = head->GetChildren().GetNode(2u);
      CHECK_EQUAL(2u, node2->GetChildren().GetCount());

      CHECK_EQUAL(0xA, node2->GetChildren().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(2.0, node2->GetChildren().GetNode(0u)->GetData().totalWeight);
      CHECK_EQUAL(1.0, node2->GetChildren().GetNode(0u)->GetData().myTotalWeight);
      CHECK_EQUAL(1,   node2->GetChildren().GetLink(0u).count);

      CHECK_EQUAL(0xB, node2->GetChildren().GetNode(1u)->GetData().fileHandle);
      CHECK_EQUAL(1.0, node2->GetChildren().GetNode(1u)->GetData().totalWeight);
      CHECK_EQUAL(1.0, node2->GetChildren().GetNode(1u)->GetData().myTotalWeight);
      CHECK_EQUAL(1,   node2->GetChildren().GetLink(1u).count);
    }

    // 3
    CHECK_EQUAL(0x3, head->GetChildren().GetNode(3u)->GetData().fileHandle);
    CHECK_EQUAL(4.0, head->GetChildren().GetNode(3u)->GetData().totalWeight);
    CHECK_EQUAL(1.0, head->GetChildren().GetNode(3u)->GetData().myTotalWeight);

    {
      GroupedDependencyGraph::Node* node3 = head->GetChildren().GetNode(3u);
      CHECK_EQUAL(1u, node3->GetChildren().GetCount());

      CHECK_EQUAL(0xA, node3->GetChildren().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(3.0, node3->GetChildren().GetNode(0u)->GetData().totalWeight);
      CHECK_EQUAL(1.0, node3->GetChildren().GetNode(0u)->GetData().myTotalWeight);
      CHECK_EQUAL(1,   node3->GetChildren().GetLink(0u).count);
    }
    // C
    CHECK_EQUAL(4.0, head->GetChildren().GetNode(4u)->GetData().totalWeight);
    CHECK_EQUAL(3.0, head->GetChildren().GetNode(4u)->GetData().myTotalWeight);
    {
      GroupedDependencyGraph::Node* nodeC = head->GetChildren().GetNode(4u);
      // Ref by
      CHECK_EQUAL(1u, nodeC->GetRefBy().GetCount());

      CHECK_EQUAL(0xA, nodeC->GetRefBy().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(3,   nodeC->GetRefBy().GetLink(0u).count);
      // Children
      CHECK_EQUAL(1u, nodeC->GetChildren().GetCount());

      CHECK_EQUAL(0xE, nodeC->GetChildren().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(1.0, nodeC->GetChildren().GetNode(0u)->GetData().totalWeight);
      CHECK_EQUAL(1.0, nodeC->GetChildren().GetNode(0u)->GetData().myTotalWeight);
      CHECK_EQUAL(1,   nodeC->GetChildren().GetLink(0u).count);
    }
    // B
    CHECK_EQUAL(2.0, head->GetChildren().GetNode(5u)->GetData().totalWeight);
    CHECK_EQUAL(2.0, head->GetChildren().GetNode(5u)->GetData().myTotalWeight);
    {
      GroupedDependencyGraph::Node* nodeB = head->GetChildren().GetNode(5u);
      // Ref by
      CHECK_EQUAL(2u, nodeB->GetRefBy().GetCount());

      CHECK_EQUAL(0x2, nodeB->GetRefBy().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(1,   nodeB->GetRefBy().GetLink(0u).count);

      CHECK_EQUAL(0xA, nodeB->GetRefBy().GetNode(1u)->GetData().fileHandle);
      CHECK_EQUAL(1,   nodeB->GetRefBy().GetLink(1u).count);
      // Children
      CHECK_EQUAL(0u, nodeB->GetChildren().GetCount());
    }
    // D
    CHECK_EQUAL(1.0, head->GetChildren().GetNode(6u)->GetData().totalWeight);
    CHECK_EQUAL(1.0, head->GetChildren().GetNode(6u)->GetData().myTotalWeight);
    {
      GroupedDependencyGraph::Node* nodeD = head->GetChildren().GetNode(6u);
      // Ref by
      CHECK_EQUAL(1u, nodeD->GetRefBy().GetCount());

      CHECK_EQUAL(0x1, nodeD->GetRefBy().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(1,   nodeD->GetRefBy().GetLink(0u).count);
      // Children
      CHECK_EQUAL(0u, nodeD->GetChildren().GetCount());
    }
    // E
    CHECK_EQUAL(1.0, head->GetChildren().GetNode(7u)->GetData().totalWeight);
    CHECK_EQUAL(1.0, head->GetChildren().GetNode(7u)->GetData().myTotalWeight);
    {
      GroupedDependencyGraph::Node* nodeE = head->GetChildren().GetNode(7u);
      // Ref by
      CHECK_EQUAL(1u, nodeE->GetRefBy().GetCount());

      CHECK_EQUAL(0xC, nodeE->GetRefBy().GetNode(0u)->GetData().fileHandle);
      CHECK_EQUAL(1,   nodeE->GetRefBy().GetLink(0u).count);
      // Children
      CHECK_EQUAL(0u, nodeE->GetChildren().GetCount());
    }
  }
}
