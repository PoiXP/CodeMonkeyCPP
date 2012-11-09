#include "Precompile.h"
#include "Core/MSVCDependencyParser.h"
#include "Core/DependencyGraph.h"
#include "Utils/File.h"
#include "Utils/FileDictionary.h"
#include "Utils/Log.h"

MSVCDependencyParser::MSVCDependencyParser()
  : m_CurrentLineIndex(0)
{

}

MSVCDependencyParser::ParseInfo::ParseInfo()
  : parsed(false)
  , error(false)
  , depthLevel(0)
  , filename("")
{

}

int MSVCDependencyParser::ParseDenendencies(const char* filename, DependencyGraph& builtGraph, FileDictionary& fileDictionary)
{
  LOG_INFO("Parsing file: %s", filename);
  RemoveThreadingInformationIfNeeded(filename);
  File fileReader;

  m_CompilationStage = e_CompilationNew;
  m_CurrentLine = "";
  m_CurrentLineIndex = 0;
  builtGraph.GetHead()->GetData().fileHandle = fileDictionary.MakeHandle("<HEAD>");
  if (!fileReader.Open(filename))
  {
    return e_FileNotFound;
  }
  std::string line;
  std::string projectName;
  DependencyGraph::Node* libraryNode = NULL;
  unsigned int projectFound = 0;
  while (!fileReader.EndOfFile())
  {
    std::string currentLine = GetCurrentLine(fileReader);
    switch (m_CompilationStage)
    {
    case e_CompilationNew:
      GoToNextLine(fileReader);
      if (IsProjectBuildStarted(currentLine, projectName))
      {
        LOG_INFO("Parsing project: %s", projectName.c_str());
        projectFound += 1;
        NodeData data;
        data.fileHandle = fileDictionary.MakeHandle("LIBRARY\\"+projectName);
        libraryNode = builtGraph.CreateNode(data);
        if (libraryNode != NULL)
        {
          builtGraph.AddDependency(builtGraph.GetHead(), libraryNode);
        }
        m_CompilationStage = e_CompilationInitialization;
      }
      break;
    case e_CompilationInitialization:
      GoToNextLine(fileReader);
      if (IsCompilationStageStarted(currentLine))
      {
        m_CompilationStage = e_CompilationProgress;
      }
      break;
    case e_CompilationProgress:
      if (IsCompilationStageOver(currentLine))
      {
        m_CompilationStage = e_CompilationNew;
      }
      else
      {
        DependencyGraph::Node* node = ReadNode(builtGraph, fileReader, fileDictionary, 0u);
        if (node != NULL)
        {
          builtGraph.AddDependency(libraryNode, node);
        }
      }
      break;
    }
  }
  LOG_INFO("The file been parsed. %d total project(s) found.", projectFound);
  return e_OK;
}

const std::string& MSVCDependencyParser::GetCurrentLine(File& fileReader)
{
  if (m_CurrentLine.empty())
  {
    fileReader.ReadLine(m_CurrentLine);
    m_CurrentLineIndex += 1;
  }
  return m_CurrentLine;
}
void MSVCDependencyParser::GoToNextLine(File& fileReader)
{
  m_CurrentLine = "";
  m_ParsedLine.parsed = false;
}

const MSVCDependencyParser::ParseInfo& MSVCDependencyParser::ParseCurrentLine()
{
  if (m_ParsedLine.parsed)
  {
    return m_ParsedLine;
  }

  static boost::regex reCompilationUnit = boost::regex("^  ([^ ]*)$");
  boost::match_results<std::string::const_iterator> results;
  if (boost::regex_match(m_CurrentLine, results, reCompilationUnit))
  {
    m_ParsedLine.parsed = true;
    m_ParsedLine.error = false;
    m_ParsedLine.depthLevel = 0u;
    m_ParsedLine.filename = std::string(results[1].first, results[1].second);
    return m_ParsedLine;
  }

  static boost::regex reInclude = boost::regex("^  Note: including file:([ ]+)(.*)$");
  if (boost::regex_match(m_CurrentLine, results, reInclude))
  {
    m_ParsedLine.parsed = true;
    m_ParsedLine.error = false;
    m_ParsedLine.depthLevel = std::distance(results[1].first, results[1].second);
    m_ParsedLine.filename = std::string(results[2].first, results[2].second);
    return m_ParsedLine;
  }

  m_ParsedLine.parsed = true;
  m_ParsedLine.error = true;
  return m_ParsedLine;
}


DependencyGraph::Node* MSVCDependencyParser::ReadNode(DependencyGraph& graph, File& fileReader, FileDictionary& fileDictionary, unsigned int dependencyLevel)
{
  while (!fileReader.EndOfFile())
  {
    if (IsCompilationStageOver(GetCurrentLine(fileReader)))
    {
      return NULL;
    }
    if (!ShouldSkipLine(GetCurrentLine(fileReader)))
    {
      break;
    }
    GoToNextLine(fileReader);
  }

  const std::string& currentLine = GetCurrentLine(fileReader);
  const ParseInfo& parsedData = ParseCurrentLine();
  if (!parsedData.error)
  {
    if (parsedData.depthLevel == dependencyLevel)
    {
      NodeData data;
      data.fileHandle = fileDictionary.MakeHandle(parsedData.filename);
      GoToNextLine(fileReader);

      return MakeNode(data, graph, fileReader, fileDictionary, dependencyLevel);
    }
    else if (parsedData.depthLevel < dependencyLevel)
    {
      return NULL;
    }
    else if (parsedData.depthLevel > dependencyLevel)
    {
      LOG_ERROR("Error when parsing line(%d):%s. The expected depth %d got: %d", m_CurrentLineIndex, currentLine.c_str(), dependencyLevel, parsedData.depthLevel);
      GoToNextLine(fileReader);
      return NULL;
    }
  }
  else
  {
    GoToNextLine(fileReader);
    return NULL;
  }
  return NULL;
}

DependencyGraph::Node* MSVCDependencyParser::MakeNode(const NodeData& data, DependencyGraph& graph, File& fileReader, FileDictionary& fileDictionary, unsigned int dependencyLevel)
{
  DependencyGraph::Node* node = graph.CreateNode(data);
  while(true)
  {
    DependencyGraph::Node* child = ReadNode(graph, fileReader, fileDictionary, dependencyLevel + 1u);
    if (child == NULL)
    {
      break;
    }
    graph.AddDependency(node, child);
  }
  return node;
}

bool MSVCDependencyParser::IsProjectBuildStarted(const std::string& line, std::string& projectName)
{
  // ------ <Build action> started: Project: <Project_Name>, Configuration: <Configuration> <Platform>
  static boost::regex reProject = boost::regex("^-*.*started: Project:[\\s]*([^,\\s]*), Configuration:.*$");

  boost::match_results<std::string::const_iterator> results;
  if (boost::regex_match(line, results, reProject))
  {
    projectName = std::string(results[1].first, results[1].second);
    return true;
  }
  return false;
}

bool MSVCDependencyParser::IsCompilationStageStarted(const std::string& line)
{
  return line == "InitializeBuildStatus:";
}

bool MSVCDependencyParser::IsCompilationStageOver(const std::string& line)
{
  return line == "FinalizeBuildStatus:" || line == "PostBuildEvent:";
}

bool MSVCDependencyParser::ShouldSkipLine(const std::string& line)
{
  return line == "  Generating Code..." || line == "  Compiling...";
}

namespace
{
  std::string::const_iterator FindThreadingPadding(std::string::const_iterator begin, std::string::const_iterator end, unsigned int& threadId)
  {
    std::string::const_iterator it = begin;
    while(it != end)
    {
      if (*it == '>' )
      {
        threadId = atoi(std::string(begin, it).c_str());
        return ++it;
      }
      if (!std::isdigit(*it)) return end;
      ++it;
    }
    return end;
  }
}

void MSVCDependencyParser::RemoveThreadingInformationIfNeeded(const char* filename)
{
  File reader;
  if (!reader.Open(filename))
  {
    return;
  }

  std::string line;
  unsigned int threadId;
  std::vector<std::fstream> m_Writer;
  m_Writer.reserve(1000);
  bool threadedVersion = false;
  int linesSkipped = 0;

  while(!reader.EndOfFile()) 
  {
    line.clear();
    reader.ReadLine(line);
    std::string::const_iterator begin = line.begin();
    std::string::const_iterator end = line.end();
    std::string::const_iterator dataPos = FindThreadingPadding(begin, end, threadId);
    if ( dataPos != line.end())
    {
      threadedVersion = true;
      if (m_Writer.size() < threadId)
      {
        char number[16];
        sprintf_s(number, sizeof(number), "%u.temp", threadId);
        m_Writer.push_back( std::fstream() );
        m_Writer[threadId - 1].open(number, std::ios_base::out | std::ios_base::ate);
      }
    }
    else
    {
      if (!threadedVersion && linesSkipped++ > 100)
        break;
      continue;
    }

    m_Writer[threadId-1].write(line.c_str() + std::distance(begin, dataPos), std::distance(dataPos, end));
    m_Writer[threadId-1] << std::endl;
  }

  reader.Close();
  if (!threadedVersion)
    return;

  LOG_INFO("Converting log into to the plain format.");

  boost::filesystem::path filePath = filename;
  std::string backFilename = filePath.root_directory().string() + std::string("backup\\");
  boost::filesystem::create_directory(backFilename);
  backFilename += filePath.filename().string();
  boost::filesystem::copy_file(filename, backFilename, boost::filesystem::copy_option::overwrite_if_exists);

  LOG_INFO("Backup file is created in: %s", backFilename.c_str());

  std::fstream newFile;
  newFile.open(filename, std::ios_base::out);
  char buffer[4096];
  for (unsigned int t = 1; t <= m_Writer.size(); ++t)
  {
    m_Writer[t-1].close();
    std::fstream f;
    char number[16];
    sprintf_s(number, sizeof(number), "%u.temp", t);
    f.open(number, std::ios_base::in);
    while (!f.eof())
    {
      f.read(buffer, sizeof(buffer));
      newFile.write(buffer, f.gcount());
    }
    f.close();
    boost::filesystem::remove(number);
  }
  newFile.close();
  m_Writer.clear();
  LOG_INFO("The log is converted");
}

