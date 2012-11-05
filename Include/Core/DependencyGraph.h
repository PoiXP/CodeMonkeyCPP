#pragma once;

#include <Utils/Graph.h>

struct LinkData
{
  int count;
};

struct NodeData
{
  unsigned int fileHandle;
};

class DependencyGraph : public Graph<NodeData, LinkData>
{
public:
  DependencyGraph(unsigned int nodesAmount = 0) : Graph(nodesAmount) {}
protected:
  virtual void ApplyDependency(LinkData& data)
  {
    data.count += 1;
  }
};
