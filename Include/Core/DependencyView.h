#ifndef CORE_DEPENDENCY_VIEW_H
#define CORE_DEPENDENCY_VIEW_H

#include "Core/DependencyGraph.h"

struct LinkWeight : public LinkData
{
//  double linkWeight;
};

struct NodeWeight : public NodeData
{
  double totalWeight;
  double myTotalWeight;
  double myWeight;
};

class GroupedDependencyGraph : public Graph<NodeWeight, LinkWeight>
{
public:
  GroupedDependencyGraph(unsigned int nodesAmount) : Graph(nodesAmount) {}
protected:
  void ApplyDependency(LinkWeight& data) { }
};

class FileDictionary;

class DependencyView
{
public:
  enum WeightType
  {
    e_CompiledFilesCount,
    e_FilesCount,
  };

  static void Build(const DependencyGraph& dependencies, const FileDictionary& fileDictionary, WeightType criteria, GroupedDependencyGraph& result );
  static void Sort(GroupedDependencyGraph& result);
};

#endif //CORE_DEPENDENCY_VIEW_H