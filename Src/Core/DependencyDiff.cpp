#include "Precompile.h"
#include "Core/DependencyDiff.h"
#include "Core/DependencyView.h"

class SortByName : public GroupedDependencyGraph::SortCriteria
{
public:
  bool IsLess(const GroupedDependencyGraph::Node* node1, const LinkWeight& link1, const GroupedDependencyGraph::Node* node2, const LinkWeight& link2)
  {
    return node1->GetData().fileHandle < node2->GetData().fileHandle;
  }
};

class FindNodeByFileHandle : public GroupedDependencyGraph::MatchNode
{
public:
  FindNodeByFileHandle(unsigned int fileHandle) : m_FileHandle(fileHandle) {}
  bool Match(const GroupedDependencyGraph::Node* node)
  {
    return node->GetData().fileHandle == m_FileHandle; 
  }
private:
  unsigned int m_FileHandle;
};

void DiffNode(GroupedDependencyGraph::Node* newNode, GroupedDependencyGraph::Node* oldNode, bool onlyDirectLink, bool diffSelfWeight, bool diffChildren, Diff& diff)
{
  if (newNode == NULL || oldNode == NULL)
  {
    return;
  }
  SortByName sorter;
  newNode->GetChildren().Sort(&sorter);
  oldNode->GetChildren().Sort(&sorter);
  unsigned int newChildId = 0;
  unsigned int oldChildId = 0;
  unsigned int newSize = newNode->GetChildren().GetCount();
  unsigned int oldSize = oldNode->GetChildren().GetCount();
  while (newChildId < newSize || oldChildId < oldSize)
  {
    if (newChildId == newSize)
    {
      const GroupedDependencyGraph::Node* oldChild = oldNode->GetChildren().GetNode(oldChildId);
      FindNodeByFileHandle match(oldNode->GetData().fileHandle);
      unsigned int dummy;
      if (!onlyDirectLink || oldChild->GetRefBy().FindNode(&match, dummy))
      {
        DiffEntry entry(oldNode->GetData().fileHandle, oldChild->GetData().fileHandle, oldNode->GetChildren().GetLink(oldChildId).count, oldChild->GetData().totalWeight);
        diff.m_Removed.push_back( entry );
      }
      oldChildId += 1;
    }
    else if (oldChildId == oldSize)
    {
      const GroupedDependencyGraph::Node* newChild = newNode->GetChildren().GetNode(newChildId);
      FindNodeByFileHandle match(newNode->GetData().fileHandle);
      unsigned int dummy;
      if (!onlyDirectLink || newChild->GetRefBy().FindNode(&match, dummy))
      {
        DiffEntry entry(newNode->GetData().fileHandle, newChild->GetData().fileHandle, newNode->GetChildren().GetLink(newChildId).count, newChild->GetData().totalWeight);
        diff.m_Added.push_back(entry);
      }
      newChildId += 1;
    }
    else
    {
      GroupedDependencyGraph::Node* newChild = newNode->GetChildren().GetNode(newChildId);
      GroupedDependencyGraph::Node* oldChild = oldNode->GetChildren().GetNode(oldChildId);
      if (newChild->GetData().fileHandle == oldChild->GetData().fileHandle)
      {
        if (diffSelfWeight)
        {
          if (newChild->GetData().myWeight > oldChild->GetData().myWeight)
          {
            DiffEntry entry(newChild->GetData().fileHandle,newChild->GetData().fileHandle, 0, newChild->GetData().myWeight - oldChild->GetData().myWeight);
            diff.m_Increased.push_back(entry);
          }
          else if (newChild->GetData().myWeight < oldChild->GetData().myWeight)
          {
            DiffEntry entry(newChild->GetData().fileHandle,newChild->GetData().fileHandle, 0, oldChild->GetData().myWeight - newChild->GetData().myWeight);
            diff.m_Decreased.push_back(entry);
          }
        }
        if (diffChildren)
        {
          DiffNode(newChild, oldChild, false, false, false, diff);
        }
        newChildId += 1;
        oldChildId += 1;
      }
      else if (newChild->GetData().fileHandle > oldChild->GetData().fileHandle)
      {
        FindNodeByFileHandle match(oldNode->GetData().fileHandle);
        unsigned int dummy;
        if (!onlyDirectLink || oldChild->GetRefBy().FindNode(&match, dummy))
        {
          DiffEntry entry(oldNode->GetData().fileHandle, oldChild->GetData().fileHandle, oldNode->GetChildren().GetLink(oldChildId).count, oldChild->GetData().totalWeight);
          diff.m_Removed.push_back( entry );
        }
        oldChildId += 1;
      }
      else 
      {
        FindNodeByFileHandle match(newNode->GetData().fileHandle);
        unsigned int dummy;
        if (!onlyDirectLink || newChild->GetRefBy().FindNode(&match, dummy))
        {
          DiffEntry entry(newNode->GetData().fileHandle, newChild->GetData().fileHandle, newNode->GetChildren().GetLink(newChildId).count, newChild->GetData().totalWeight);
          diff.m_Added.push_back(entry);
        }
        newChildId += 1;
      }
    }
  }
}

bool SortDiffByWeight(const DiffEntry& a, const DiffEntry& b)
{
  return a.weight > b.weight;
}

void DependencyDiff::MakeDiff(GroupedDependencyGraph& newDependencies, GroupedDependencyGraph& oldDependencies, Diff& diff)
{
  DiffNode(newDependencies.GetHead(), oldDependencies.GetHead(), true, true, true, diff);
  std::sort(diff.m_Added.begin(), diff.m_Added.end(), SortDiffByWeight);
  std::sort(diff.m_Decreased.begin(), diff.m_Decreased.end(), SortDiffByWeight);
  std::sort(diff.m_Increased.begin(), diff.m_Increased.end(), SortDiffByWeight);
  std::sort(diff.m_Removed.begin(), diff.m_Removed.end(), SortDiffByWeight);
}