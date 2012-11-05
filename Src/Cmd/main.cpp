#include "Core\MSVCDependencyParser.h"
#include "Core\DependencyGraph.h"
#include "Core\DependencyView.h"
#include "Core\DependencyDiff.h"
#include "Utils\FileDictionary.h"
#include "Utils\Log.h"

using namespace std;

enum Error
{
  e_Success_Has_Diff = 1,
  e_Success = 0,
  e_Error_InvalidParameters = -1,
  e_Error_Parse_Failed = -2,
};

void PrintDiff(const Diff& diff, FileDictionary& dict)
{
  LOG_INFO("");
  LOG_INFO("New dependencies : ");
  for (auto i = diff.m_Added.begin(); i != diff.m_Added.end(); ++i)
  {
    std::string fromFile, toFile;
    LOG_INFO("\t%5.0lf(%u) %s -> %s \n", i->weight, i->count, dict.GetFileName(i->fromFile, fromFile).c_str(), dict.GetFileName(i->toFile, toFile).c_str());
  }
  LOG_INFO("");
  LOG_INFO("Removed dependencies : ");
  for (auto i = diff.m_Removed.begin(); i != diff.m_Removed.end(); ++i)
  {
    std::string fromFile, toFile;
    LOG_INFO("\t%5.0lf(%u) %s -> %s\n",  i->weight, i->count, dict.GetFileName(i->fromFile, fromFile).c_str(), dict.GetFileName(i->toFile, toFile).c_str());
  }
  LOG_INFO("");
  LOG_INFO("Increased weight : ");
  for (auto i = diff.m_Increased.begin(); i != diff.m_Increased.end(); ++i)
  {
    std::string fromFile;
    LOG_INFO("\t%5.0lf %s\n",  i->weight, dict.GetFileName(i->fromFile, fromFile).c_str());
  }
  LOG_INFO("");
  LOG_INFO("Decreased weight : ");
  for (auto i = diff.m_Decreased.end(); i != diff.m_Decreased.end(); ++i)
  {
    std::string fromFile;
    LOG_INFO("\t%5.0lf %s\n",  i->weight, dict.GetFileName(i->fromFile, fromFile).c_str());
  }
  LOG_INFO("");
}

void PrintNode(const DependencyGraph::Node* node, const FileDictionary& dict, unsigned int indent)
{
  char format[64];
  sprintf_s(format, sizeof(format), "%%%ds%%s", 2*indent);
  std::string s;
  LOG_INFO(format, "", dict.GetFileName(node->GetData().fileHandle, s).c_str());

  for (unsigned int i=0u; i < node->GetChildren().GetCount(); i++)
  {
    PrintNode( node->GetChildren().GetNode(i), dict, indent+1);
  }
}

int Run(int ArgCount, char** Args)
{
  if (ArgCount < 3)
  {
    LOG_ERROR("Invalid arguments count: %d", ArgCount);
    return e_Error_InvalidParameters;
  }
  DependencyGraph newDependencies, oldDependencies;
  FileDictionary dictonary(FileDictionary::e_Compare_NoCase, 512);
  MSVCDependencyParser parser;
  int parseValue = parser.ParseDenendencies(Args[1], newDependencies, dictonary);
  if (parseValue != MSVCDependencyParser::e_OK)
  {
    LOG_ERROR("Not able to parse: %s error code: %d", Args[1], parseValue);
    return e_Error_Parse_Failed;
  }
  if (strcmp(Args[2],"print") ==0)
  {
    PrintNode(newDependencies.GetHead(), dictonary, 0u);
    return e_Success;
  }
  if (parser.ParseDenendencies(Args[2], oldDependencies, dictonary) != MSVCDependencyParser::e_OK)
  {
    LOG_ERROR("Not able to parse: %s error code: %d", Args[2], parseValue);
    return e_Error_Parse_Failed;
  }

  GroupedDependencyGraph newGroupedGraph(newDependencies.GetNodeCount()), oldGroupedGraph(oldDependencies.GetNodeCount());
  DependencyView::Build(newDependencies, dictonary, DependencyView::e_FilesCount, newGroupedGraph);
  DependencyView::Build(oldDependencies, dictonary, DependencyView::e_FilesCount, oldGroupedGraph);
  Diff diff;
  DependencyDiff::MakeDiff(newGroupedGraph, oldGroupedGraph, diff);

  PrintDiff(diff, dictonary);

  return (diff.m_Added.empty() && diff.m_Removed.empty()) ? e_Success : e_Success_Has_Diff;
}

int main(int ArgCount, char** Args)
{
  Log::Initialize("CodeMonkeyCmd", Log::e_LogAll);
  LOG_INFO("Application started");
  int returnValue = Run(ArgCount, Args);
  Log::Shutdown();

  return returnValue;
}