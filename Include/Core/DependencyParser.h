#ifndef CORE_DEPENDENCY_PARSER
#define CORE_DEPENDENCY_PARSER
#pragma once

class DependencyGraph;
class FileDictionary;

class DependencyParser
{
public:
  DependencyParser() {}
  virtual ~DependencyParser() {}
  virtual int ParseDenendencies(const char* filename, DependencyGraph& builtGraph, FileDictionary& fileDictionary) = 0;
};

#endif // CORE_DEPENDENCY_PARSER