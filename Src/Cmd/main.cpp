#include "Core\MSVSDependencyParser.h"
#include "Core\DependencyGraph.h"
#include "Core\DependencyView.h"
#include "Core\DependencyDiff.h"
#include "Utils\FileDictionary.h"
#include "Utils\Log.h"
#include <boost/program_options.hpp>
#include <fstream>
#include <cstdarg>

namespace po = boost::program_options;

enum Error
{
  e_Success_Has_Diff = 1,
  e_Success = 0,
  e_Error_InvalidParameters = -1,
  e_Error_Parse_Failed = -2,
};

class Output
{
public:
  virtual ~Output() {}
  virtual void Print(const char* format, ...) = 0;
};

class LogOutput : public Output
{
public:
  void Print(const char* format, ...)
  {
    va_list args;
    va_start(args, format);
    Log::LogMessage_va(Log::e_LogInfo, format, args);
    va_end(args);
  }
};

class FileOutput : public Output
{
public:
  FileOutput() {}
  ~FileOutput()
  { 
    fclose(m_File);
  }
  bool Open(const char* filename)
  {
    fopen_s(&m_File, filename, "w");
    return m_File != 0;
  }
  void Print(const char* format, ...)
  {
    va_list args;
    va_start(args, format);
    vfprintf_s(m_File, format, args);
    va_end(args);
    fprintf(m_File, "\n");
  }
private:
  FILE* m_File;
};

static Output* g_Output = NULL;
#define OUTPUT(...) g_Output->Print(##__VA_ARGS__);

void PrintDiff(const Diff& diff, FileDictionary& dict)
{
  OUTPUT("");
  OUTPUT("New dependencies : ");
  for (auto i = diff.m_Added.begin(); i != diff.m_Added.end(); ++i)
  {
    std::string fromFile, toFile;
    OUTPUT("\t%5.0lf(%u) %s -> %s \n", i->weight, i->count, dict.GetFileName(i->fromFile, fromFile).c_str(), dict.GetFileName(i->toFile, toFile).c_str());
  }
  OUTPUT("");
  OUTPUT("Removed dependencies : ");
  for (auto i = diff.m_Removed.begin(); i != diff.m_Removed.end(); ++i)
  {
    std::string fromFile, toFile;
    OUTPUT("\t%5.0lf(%u) %s -> %s\n",  i->weight, i->count, dict.GetFileName(i->fromFile, fromFile).c_str(), dict.GetFileName(i->toFile, toFile).c_str());
  }
  OUTPUT("");
  OUTPUT("Increased weight : ");
  for (auto i = diff.m_Increased.begin(); i != diff.m_Increased.end(); ++i)
  {
    std::string fromFile;
    LOG_INFO("\t%5.0lf %s\n",  i->weight, dict.GetFileName(i->fromFile, fromFile).c_str());
  }
  OUTPUT("");
  OUTPUT("Decreased weight : ");
  for (auto i = diff.m_Decreased.end(); i != diff.m_Decreased.end(); ++i)
  {
    std::string fromFile;
    OUTPUT("\t%5.0lf %s\n",  i->weight, dict.GetFileName(i->fromFile, fromFile).c_str());
  }
  OUTPUT("");
}

void PrintNode(const DependencyGraph::Node* node, const FileDictionary& dict, unsigned int indent)
{
  char format[64];
  sprintf_s(format, sizeof(format), "%%%ds%%s", 2*indent);
  std::string s;
  OUTPUT(format, "", dict.GetFileName(node->GetData().fileHandle, s).c_str());

  for (unsigned int i=0u; i < node->GetChildren().GetCount(); i++)
  {
    PrintNode( node->GetChildren().GetNode(i), dict, indent+1);
  }
}

int Run(int ArgCount, char** Args)
{
  po::positional_options_description positionOptions;
  positionOptions.add("command", 1);
  positionOptions.add("in_file", 1);
  positionOptions.add("prev_file", 1);

  po::options_description options("Allowed options");
  options.add_options()
    ("help", "print help message")
    ("command", po::value<std::string>(), "Command name {print, diff}. [Positional]")
    ("in_file", po::value<std::string>(), "A dependency file to analyze. [Positional]")
    ("prev_file", po::value<std::string>(), "A dependency file to diff with. [Positional]")
    ("solution", po::value<std::string>(), "Visual Studio 2010 solution file")
    ("out", po::value<std::string>(), "redirect output into the file");

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(ArgCount, Args). options(options).positional(positionOptions).run(), vm);
    po::notify(vm);
  }
  catch(po::error& genericError)
  {
    LOG_ERROR("Exception: %s", genericError.what());
    return e_Error_InvalidParameters;
  }

  if (vm.count("command"))
  {
    if (vm.count("out"))
    {
      FileOutput* output = new FileOutput();
      if (output->Open(vm["out"].as<std::string>().c_str()))
      {
        g_Output = output;
      }
      else
      {
        delete output;
        LOG_ERROR("Can't open open file: %s", vm["out"].as<std::string>())
        return e_Error_InvalidParameters;
      }
    }
    else
    {
      g_Output = new LogOutput;
    }

    std::string commandName = vm["command"].as<std::string>();
    if (commandName == "print")
    {
      FileDictionary dictonary(FileDictionary::e_Compare_NoCase, 512);
      std::string fileName = vm["in_file"].as<std::string>();
      DependencyGraph dependencies;
      MSVSDependencyParser parser;
      int parseValue = parser.ParseDenendencies(fileName.c_str(), dependencies, dictonary);
      if (parseValue != MSVSDependencyParser::e_OK)
      {
        LOG_ERROR("Not able to parse: %s error code: %d", fileName.c_str(), parseValue);
        return e_Error_Parse_Failed;
      }
      if (vm.count("solution"))
      {

      }
      else
      {
        PrintNode(dependencies.GetHead(), dictonary, 0u);
      }
      return e_Success;
    }
    else if (commandName == "diff")
    {
      std::string fileName = vm["in_file"].as<std::string>();
      std::string prevFileName = vm["prev_file"].as<std::string>();

      DependencyGraph newDependencies, oldDependencies;
      FileDictionary dictonary(FileDictionary::e_Compare_NoCase, 512);
      MSVSDependencyParser parser;
      int parseValue = parser.ParseDenendencies(fileName.c_str(), newDependencies, dictonary);
      if (parseValue != MSVSDependencyParser::e_OK)
      {
        LOG_ERROR("Not able to parse: %s error code: %d", fileName.c_str(), parseValue);
        return e_Error_Parse_Failed;
      }
      if (parser.ParseDenendencies(prevFileName.c_str(), oldDependencies, dictonary) != MSVSDependencyParser::e_OK)
      {
        LOG_ERROR("Not able to parse: %s error code: %d", prevFileName.c_str(), parseValue);
        return e_Error_Parse_Failed;
      }

      GroupedDependencyGraph newGroupedGraph(newDependencies.GetNodeCount()), oldGroupedGraph(oldDependencies.GetNodeCount());
      DependencyView::Build(newDependencies, dictonary, DependencyView::e_FilesCount, newGroupedGraph);
      DependencyView::Build(oldDependencies, dictonary, DependencyView::e_FilesCount, oldGroupedGraph);
      Diff diff;
      DependencyDiff::MakeDiff(newGroupedGraph, oldGroupedGraph, diff);

      if (vm.count("solution"))
      {

      }
      else
      {
        PrintDiff(diff, dictonary);
      }

      return (diff.m_Added.empty() && diff.m_Removed.empty()) ? e_Success : e_Success_Has_Diff;
    }
    LOG_ERROR("Unkown command: %s", commandName.c_str());
  }
  std::ostringstream s;
  options.print(s);
  LOG_INFO( s.str().c_str() );
  return vm.count("help") ? e_Success : e_Error_InvalidParameters;
}

int main(int ArgCount, char** Args)
{
  Log::Initialize("CodeMonkeyCmd", Log::e_LogAll);
  LOG_INFO("Application started");
  int returnValue = Run(ArgCount, Args);
  Log::Shutdown();
  delete g_Output;

  return returnValue;
}