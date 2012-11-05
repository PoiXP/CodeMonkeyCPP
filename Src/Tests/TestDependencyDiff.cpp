#include <UnitTest++\src\UnitTest++.h>
#include <Core/DependencyDiff.h>
#include <Core/DependencyView.h>

SUITE(TestDependencyDiff)
{
  TEST(TestDiff)
  {
#define MAKE_NODE(handle, weight, total, compiled, name, graph) \
  GroupedDependencyGraph::Node* name = NULL; \
  { \
    NodeWeight w; \
    w.fileHandle = handle; \
    w.myTotalWeight = weight; \
    w.totalWeight = total; \
    w.myWeight = 1.0; \
    name = graph.CreateNode(w); \
    LinkWeight link; \
    link.count = compiled; \
    graph.GetHead()->GetChildren().AddLink(name, link); \
  } \

#define ADD_CHILD(handle, weight, total, compiled, parent, graph) \
  { \
    GroupedDependencyGraph::Node* child = NULL; \
    NodeWeight w; \
    w.fileHandle = handle; \
    w.myTotalWeight = weight; \
    w.totalWeight = total; \
    child = graph.CreateNode(w); \
    LinkWeight link; \
    link.count = compiled; \
    parent->GetChildren().AddLink(child, link); \
  } \

#define ADD_REFBY(handle, refBy, parent, graph) \
  { \
    GroupedDependencyGraph::Node* ref = NULL; \
    NodeWeight w; \
    w.fileHandle = handle; \
    w.myTotalWeight = 0; \
    w.totalWeight = 0; \
    ref = graph.CreateNode(w); \
    LinkWeight link; \
    link.count = refBy; \
    parent->GetRefBy().AddLink(ref, link); \
  } \

    GroupedDependencyGraph dependsBefore(16);
    {
      /*
      Before:
            1        2        3
          __|_      _|_       |
         |    |    |   |      A
         A    D    B   A      |
       __|__           |      C
      |     |          C      |
      B     C                 E
      */
      dependsBefore.GetHead()->GetData().fileHandle = 0xDAD;
      MAKE_NODE(0x1, 1.0, 5.0, 1u, node1, dependsBefore);
        ADD_CHILD(0xA, 1.0, 3.0, 1u, node1, dependsBefore);
        ADD_CHILD(0xD, 1.0, 1.0, 1u, node1, dependsBefore);
        ADD_REFBY(0xDAD, 1u, node1, dependsBefore);
      MAKE_NODE(0x2, 1.0, 4.0, 1u, node2, dependsBefore);
        ADD_CHILD(0xB, 1.0, 1.0, 1u, node2, dependsBefore);
        ADD_CHILD(0xA, 1.0, 2.0, 1u, node2, dependsBefore);
        ADD_REFBY(0xDAD, 1u, node2, dependsBefore);
      MAKE_NODE(0x3, 1.0, 4.0, 1u, node3, dependsBefore);
        ADD_CHILD(0xA, 1.0, 2.0, 1u, node3, dependsBefore);
        ADD_REFBY(0xDAD, 1u, node3, dependsBefore);
      MAKE_NODE(0xA, 3.0, 8.0, 3u, nodeA, dependsBefore);
        ADD_CHILD(0xB, 1.0, 1.0, 1u, nodeA, dependsBefore);
        ADD_CHILD(0xC, 3.0, 4.0, 3u, nodeA, dependsBefore);
        ADD_REFBY(0x1, 1u, nodeA, dependsBefore);
        ADD_REFBY(0x2, 1u, nodeA, dependsBefore);
        ADD_REFBY(0x3, 1u, nodeA, dependsBefore);
      MAKE_NODE(0xB, 2.0, 2.0, 2u, nodeB, dependsBefore);
        ADD_REFBY(0xA, 1u, nodeB, dependsBefore);
        ADD_REFBY(0x2, 1u, nodeB, dependsBefore);
      MAKE_NODE(0xC, 3.0, 4.0, 3u, nodeC, dependsBefore);
        ADD_CHILD(0xE, 1.0, 1.0, 1u, nodeC, dependsBefore);
        ADD_REFBY(0xA, 3u, nodeC, dependsBefore);
      MAKE_NODE(0xD, 1.0, 1.0, 1u, nodeD, dependsBefore);
        ADD_REFBY(0x1, 1u, nodeD, dependsBefore);
      MAKE_NODE(0xE, 1.0, 1.0, 1u, nodeE, dependsBefore);
        ADD_REFBY(0xC, 1u, nodeE, dependsBefore);
    }
    GroupedDependencyGraph dependsAfter(16);
    {
      /*
      After:
            1        2           4
          __|_      _|_          |
         |    |    |   |         A
         A    D    F   A        _|_
       __|__          _|_      | | |
      |  |  |        |   |     F 0 C
      F  0  C        0   C   
      */
      dependsAfter.GetHead()->GetData().fileHandle = 0xDAD;
      MAKE_NODE(0x1, 1.0, 6.0, 1u, node1, dependsAfter);
        ADD_CHILD(0xA, 1.0, 4.0, 1u, node1, dependsAfter);
        ADD_CHILD(0xD, 1.0, 1.0, 1u, node1, dependsAfter);
        ADD_REFBY(0xDAD, 1u, node1, dependsAfter);
      MAKE_NODE(0x2, 1.0, 5.0, 1u, node2, dependsAfter);
        ADD_CHILD(0xF, 1.0, 1.0, 1u, node2, dependsAfter);
        ADD_CHILD(0xA, 1.0, 3.0, 1u, node2, dependsAfter);
        ADD_REFBY(0xDAD, 1u, node2, dependsAfter);
      MAKE_NODE(0x4, 1.0, 5.0, 1u, node4, dependsAfter);
        ADD_CHILD(0xA, 1.0, 4.0, 1u, node4, dependsAfter);
        ADD_REFBY(0xDAD, 1u, node4, dependsAfter);
      MAKE_NODE(0xA, 3.0, 11.0, 3u, nodeA, dependsAfter);
        ADD_CHILD(0xF, 2.0, 2.0, 2u, nodeA, dependsAfter);
        ADD_CHILD(0x0, 3.0, 3.0, 3u, nodeA, dependsAfter);
        ADD_CHILD(0xC, 3.0, 3.0, 3u, nodeA, dependsAfter);
        ADD_REFBY(0x1, 1u, nodeA, dependsAfter);
        ADD_REFBY(0x2, 1u, nodeA, dependsAfter);
        ADD_REFBY(0x4, 1u, nodeA, dependsAfter);
      MAKE_NODE(0xC, 3.0, 3.0, 3u, nodeC, dependsAfter);
        ADD_REFBY(0xA, 3u, nodeC, dependsAfter);
      MAKE_NODE(0xD, 1.0, 1.0, 1u, nodeD, dependsAfter);
        ADD_REFBY(0x1, 1u, nodeD, dependsAfter);
      MAKE_NODE(0xF, 3.0, 3.0, 3u, nodeF, dependsAfter);
        ADD_REFBY(0xA, 2u, nodeF, dependsAfter);
        ADD_REFBY(0x2, 1u, nodeF, dependsAfter);
      MAKE_NODE(0x0, 3.0, 3.0, 3u, node0, dependsAfter);
        ADD_REFBY(0xA, 3u, node0, dependsAfter);
    }
    /*
    Diff:
    DAD - 3
    2 - B
    A - B
    C - E

    DAD + 4
    A + 0
    A + F
    2 + F
    */

    Diff diff;
    DependencyDiff::MakeDiff(dependsAfter, dependsBefore, diff);
    CHECK_EQUAL(diff.m_Added.size(), 4u);
    CHECK_EQUAL(diff.m_Added[0].fromFile, 0xDAD);
    CHECK_EQUAL(diff.m_Added[0].toFile, 0x4);
    CHECK_EQUAL(diff.m_Added[0].weight, 5.0);
    CHECK_EQUAL(diff.m_Added[1].fromFile, 0xA);
    CHECK_EQUAL(diff.m_Added[1].toFile, 0x0);
    CHECK_EQUAL(diff.m_Added[1].weight, 3.0);
    CHECK_EQUAL(diff.m_Added[2].fromFile, 0xA);
    CHECK_EQUAL(diff.m_Added[2].toFile, 0xF);
    CHECK_EQUAL(diff.m_Added[2].weight, 2.0);
    CHECK_EQUAL(diff.m_Added[3].fromFile, 0x2);
    CHECK_EQUAL(diff.m_Added[3].toFile, 0xF);
    CHECK_EQUAL(diff.m_Added[3].weight, 1.0);
    CHECK_EQUAL(diff.m_Removed.size(), 4u);
    CHECK_EQUAL(diff.m_Removed[0].fromFile, 0xDAD);
    CHECK_EQUAL(diff.m_Removed[0].toFile, 0x3);
    CHECK_EQUAL(diff.m_Removed[0].weight, 4.0);
    CHECK_EQUAL(diff.m_Removed[1].fromFile, 0x2);
    CHECK_EQUAL(diff.m_Removed[1].toFile, 0xB);
    CHECK_EQUAL(diff.m_Removed[1].weight, 1.0);
    CHECK_EQUAL(diff.m_Removed[2].fromFile, 0xA);
    CHECK_EQUAL(diff.m_Removed[2].toFile, 0xB);
    CHECK_EQUAL(diff.m_Removed[2].weight, 1.0);
    CHECK_EQUAL(diff.m_Removed[3].fromFile, 0xC);
    CHECK_EQUAL(diff.m_Removed[3].toFile, 0xE);
    CHECK_EQUAL(diff.m_Removed[3].weight, 1.0);
  }
}
