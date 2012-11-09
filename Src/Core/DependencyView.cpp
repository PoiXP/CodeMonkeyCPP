#include "Precompile.h"
#include "Core/DependencyView.h"
#include "Utils/FileDictionary.h"
#include "Utils/FileUtils.h"

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

class GroupView
{
public:
  typedef std::map<unsigned int,  unsigned int > FileToNodeMap;
  typedef std::pair<unsigned int,  unsigned int > FileToMapEntry;
  GroupView(const DependencyGraph& dependencies, const FileDictionary& fileDictionary, DependencyView::WeightType weightType, GroupedDependencyGraph& result)
    : m_Dependencies(dependencies)
    , m_FileDict(fileDictionary)
    , m_WeightType(weightType)
    , m_GroupedGraph(result)
  {

  }

  void Group()
  {
    const DependencyGraph::Node* head = m_Dependencies.GetHead();
    m_GroupedGraph.GetHead()->GetData().fileHandle = head->GetData().fileHandle;
    double totalWeight = 0.0;
    for(unsigned int i=0; i < head->GetChildren().GetCount(); i++)
    {
      totalWeight += Visit(head->GetChildren().GetNode(i), m_GroupedGraph.GetHead(), 1u);
    }
    m_GroupedGraph.GetHead()->GetData().myTotalWeight = 0;
    m_GroupedGraph.GetHead()->GetData().totalWeight = totalWeight;
  }
private:
  // Visits a node returns total weight
  double Visit(const DependencyGraph::Node* dependencyNode, GroupedDependencyGraph::Node* refByNode, unsigned int times)
  {
    unsigned int fileHandle = dependencyNode->GetData().fileHandle;
    FileToNodeMap::iterator it = m_FileMap.find(fileHandle);
    GroupedDependencyGraph::Node* groupNode = NULL;
    if (it != m_FileMap.end())
    {
      groupNode = m_GroupedGraph.GetHead()->GetChildren().GetNode(it->second);
      m_GroupedGraph.GetHead()->GetChildren().GetLink(it->second).count += times;
    }
    else
    {
      NodeWeight weight;
      weight.fileHandle = fileHandle;
      weight.myWeight = GetFileWeight(fileHandle);
      weight.myTotalWeight = 0.0;
      weight.totalWeight = 0.0;
      groupNode = m_GroupedGraph.CreateNode(weight);

      LinkWeight linkData;
      linkData.count = 1;
      unsigned int nodeId = m_GroupedGraph.GetHead()->GetChildren().AddLink(groupNode, linkData);
      m_FileMap.insert( FileToMapEntry(fileHandle, nodeId) );
    }

    unsigned int refById;
    FindNodeByFileHandle matchFunc(refByNode->GetData().fileHandle);
    if (groupNode->GetRefBy().FindNode(&matchFunc, refById))
    {
      groupNode->GetRefBy().GetLink(refById).count += times;
    }
    else
    {
      NodeWeight nodeDescr;
      nodeDescr.fileHandle = refByNode->GetData().fileHandle;
      nodeDescr.myTotalWeight = 0;
      nodeDescr.totalWeight = 0;
      GroupedDependencyGraph::Node* newNode = m_GroupedGraph.CreateNode(nodeDescr);
      LinkWeight linkData;
      linkData.count = times;
      groupNode->GetRefBy().AddLink(newNode, linkData);
    }

    double totalWeight = 0.0;
    for(unsigned int i=0; i < dependencyNode->GetChildren().GetCount(); i++)
    {
      const DependencyGraph::Node* child = dependencyNode->GetChildren().GetNode(i);
      double w = Visit(child, groupNode, dependencyNode->GetChildren().GetLink(i).count);

      unsigned int linkId;
      FindNodeByFileHandle matchFunc(child->GetData().fileHandle);
      if (groupNode->GetChildren().FindNode(&matchFunc, linkId))
      {
        GroupedDependencyGraph::Node* groupDependency = groupNode->GetChildren().GetNode(linkId);
        groupDependency->GetData().myTotalWeight += GetFileWeight(child->GetData().fileHandle);
        groupDependency->GetData().totalWeight += w;
        groupNode->GetChildren().GetLink(linkId).count += 1;
      }
      else
      {
        NodeWeight nodeDescr;
        nodeDescr.fileHandle = child->GetData().fileHandle;
        nodeDescr.myTotalWeight = GetFileWeight(child->GetData().fileHandle);
        nodeDescr.totalWeight = w;
        GroupedDependencyGraph::Node* newNode = m_GroupedGraph.CreateNode(nodeDescr);
        LinkWeight linkData;
        linkData.count = 1;
        groupNode->GetChildren().AddLink(newNode, linkData);
      }
      totalWeight += w;
    }

    double fileWeight = GetFileWeight(fileHandle);
    groupNode->GetData().myTotalWeight += fileWeight;
    totalWeight += fileWeight;
    groupNode->GetData().totalWeight += totalWeight;

    return totalWeight;

  }
  double GetFileWeight(unsigned int fileHandle)
  {
    switch(m_WeightType)
    {
    case DependencyView::e_FilesCount: return 1.0; break;
    case DependencyView::e_CompiledFilesCount:
      {
        auto it = m_FileComplexityCache.find(fileHandle);
        if (it != m_FileComplexityCache.end())
        {
          return it->second;
        }
        else
        {
          std::string filename;
          m_FileDict.GetFileName(fileHandle, filename);
          unsigned int tokens =  FileUtils::GetFileSize(filename);
          m_FileComplexityCache[fileHandle] = tokens;
          return tokens;
        }
      }
      break;
    default:
      return 0.0;
    }
  }
  FileToNodeMap m_FileMap;
  DependencyView::WeightType m_WeightType;
  const FileDictionary& m_FileDict;
  const DependencyGraph& m_Dependencies;
  GroupedDependencyGraph& m_GroupedGraph;
  std::map<unsigned int, unsigned int> m_FileComplexityCache;
};


class SortByTotalWeight : public GroupedDependencyGraph::SortCriteria
{
public:
  bool IsLess(const GroupedDependencyGraph::Node* node1, const LinkWeight& link1, const GroupedDependencyGraph::Node* node2, const LinkWeight& link2)
  {
    if (node1->GetData().totalWeight != node2->GetData().totalWeight)
    {
      return node1->GetData().totalWeight > node2->GetData().totalWeight;
    }
    return node1->GetData().fileHandle < node2->GetData().fileHandle;
  }
};

class SortByLinkCount : public GroupedDependencyGraph::SortCriteria
{
public:
  bool IsLess(const GroupedDependencyGraph::Node* node1, const LinkWeight& link1, const GroupedDependencyGraph::Node* node2, const LinkWeight& link2)
  {
    if (link1.count != link2.count)
    {
      return link1.count > link2.count;
    }
    return node1->GetData().fileHandle < node2->GetData().fileHandle;
  }
};


void SortNode(GroupedDependencyGraph::Node* node)
{
  SortByTotalWeight byWeight;
  node->GetChildren().Sort(&byWeight);
  SortByLinkCount byRefCount;
  node->GetRefBy().Sort(&byRefCount);
  for (unsigned int i = 0; i < node->GetChildren().GetCount(); i++)
  {
    SortNode(node->GetChildren().GetNode(i));
  }
}

void DependencyView::Build(const DependencyGraph& dependencies, const FileDictionary& fileDictionary, WeightType criteria, GroupedDependencyGraph& result )
{
  // Group all nodes with unique file name O(Entries)
  GroupView groupView(dependencies, fileDictionary, criteria, result);
  groupView.Group();
}

void DependencyView::Sort(GroupedDependencyGraph& result)
{
  SortNode( result.GetHead() );
}
