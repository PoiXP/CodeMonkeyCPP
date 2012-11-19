#ifndef CORE_PROJECT_DEPENDENCY_VIEW_H
#define CORE_PROJECT_DEPENDENCY_VIEW_H

#include "Utils/Graph.h"

struct ProjectLinkData
{
  std::map<size_t, std::set<size_t> > fileDependencies;
};

struct ProjectNodeData
{
  size_t moduleIndex;
};

class ProjectDependencyGraph : public Graph<ProjectNodeData, ProjectLinkData>
{
public:
  ProjectDependencyGraph(unsigned int modulesCount) : Graph(modulesCount) {}
protected:
  void ApplyDependency(ProjectLinkData& data) { }
};

class GroupedDependencyGraph;
class CodeBuild;

class ProjectDependencyView
{
public:
  static void Build(const GroupedDependencyGraph& dependencies, CodeBuild* codeBuild, const ProjectDependencyGraph& result );
};

#endif //CORE_DEPENDENCY_VIEW_H