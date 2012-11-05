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
