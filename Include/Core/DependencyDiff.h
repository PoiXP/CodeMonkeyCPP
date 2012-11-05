#pragma once

#include <vector>

class GroupedDependencyGraph;

struct DiffEntry
{
  DiffEntry() {}
  DiffEntry(unsigned int from, unsigned int to, unsigned linkCount, double diffWeight)
    : fromFile(from), toFile(to), count(linkCount), weight(diffWeight) {}
  unsigned int fromFile;
  unsigned int toFile;
  unsigned int count;
  double weight;
};

struct Diff
{
  std::vector<DiffEntry> m_Added;
  std::vector<DiffEntry> m_Increased;
  std::vector<DiffEntry> m_Decreased;
  std::vector<DiffEntry> m_Removed;
};

class DependencyDiff
{
public:
  static void MakeDiff(GroupedDependencyGraph& newDependencies, GroupedDependencyGraph& oldDependencies, Diff& diff);
};
