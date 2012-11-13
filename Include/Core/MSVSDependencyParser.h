#ifndef CORE_MSVS_DEPENDENCY_PARSER_H
#define CORE_MSVS_DEPENDENCY_PARSER_H
#pragma once

#include <string>
#include "DependencyParser.h"
#include "DependencyGraph.h"

class File;
class DependencyGraph;

class MSVSDependencyParser : public DependencyParser
{
public:
  enum ReturnCode
  {
    e_OK,
    e_FileNotFound,
  };
  MSVSDependencyParser();
  int ParseDenendencies(const char* filename, DependencyGraph& builtGraph, FileDictionary& fileDictionary);
private:
  struct ParseInfo
  {
    ParseInfo();
    bool          parsed;
    bool          error;
    unsigned int  depthLevel;
    std::string   filename;
  };
  
  void RemoveThreadingInformationIfNeeded(const char* filename);

  const std::string& GetCurrentLine(File& fileReader);
  const ParseInfo&  ParseCurrentLine();
  void  GoToNextLine(File& fileReader);

  DependencyGraph::Node* ReadNode(DependencyGraph& graph, File& fileReader, FileDictionary& fileDictionary, const std::string& projectName, unsigned int dependencyLevel);
  DependencyGraph::Node* MakeNode(const NodeData& data, DependencyGraph& graph, File& fileReader, FileDictionary& fileDictionary, const std::string& projectName, unsigned int dependencyLevel);
  bool IsProjectBuildStarted(const std::string& line, std::string& projectName);
  bool IsCompilationStageStarted(const std::string& line);
  bool IsCompilationStageOver(const std::string& line);
  bool ShouldSkipLine(const std::string& line);

  std::string  m_CurrentLine;
  unsigned int m_CurrentLineIndex;
  ParseInfo    m_ParsedLine;
  enum CompilationStageEnum
  {
    e_CompilationNew,
    e_CompilationInitialization,
    e_CompilationProgress,
  };
  CompilationStageEnum m_CompilationStage;
};

#endif // CORE_MSVS_DEPENDENCY_PARSER_H