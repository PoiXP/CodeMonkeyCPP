#include "Precompile.h"

#include "MSVSCodeBuild.h"
#include "Core\CodeBuild.h"
#include "Utils\Log.h"
#include "Utils\FileDictionary.h"
#include "WhatIBuild\include\BuildParser.h"
#include "WhatIBuild\include\Build.h"
#include "WhatIBuild\include\Logging.h"

class MSVSCodeBuild : public CodeBuild
{
public:
  MSVSCodeBuild();
  ~MSVSCodeBuild();

  void                LoadSolution(const char* path);

  void                CreateMapping(const FileDictionary& dictionary);
  const std::string&  ResolveName(const FileDictionary& dictionary, size_t handle, std::string& name) const;
  unsigned int        GetFileModule(size_t handle) const;
  const std::string&  GetModuleName(size_t moduleIndex) const;
private:
  struct UnitIndex
  {
    size_t moduleIndex;
    size_t unitIndex; 
  };
  std::map<size_t, UnitIndex> m_Mapping;
  WhatIBuild::Build           m_Build;
  std::string                 m_SolutionPath;
};


namespace 
{
  static void LogInfo(const char* text)
  {
    LOG_INFO("WhatIBuild: %s", text);
  }
  static void LogWarning(const char* text)
  {
    LOG_WARNING("WhatIBuild: %s", text);
  }
  static void LogError(const char* text)
  {
    LOG_ERROR("WhatIBuild: %s", text);
  }
  static void InitializeWhatIBuild()
  {
    static bool initialized = false;
    if (!initialized)
    {
      static WhatIBuild::LoggingCallbacks callbacks;
      callbacks.logInfo = &LogInfo;
      callbacks.logWarning = &LogWarning;
      callbacks.logError = &LogError;

      WhatIBuild::LogManager::Inititalize(callbacks);
      initialized = true;
    }
  }
  static const size_t EXTERNAL_MODULE = 0xFFFFFFFF;
}


MSVSCodeBuild::MSVSCodeBuild()
{
}

MSVSCodeBuild::~MSVSCodeBuild()
{

}

void MSVSCodeBuild::LoadSolution(const char* path)
{
  if (path != NULL)
  {
    InitializeWhatIBuild();
    boost::filesystem::path solutionPath = boost::filesystem::absolute(path);
    m_SolutionPath = solutionPath.normalize().string();
    WhatIBuild::Parser::ParseVSSolution(m_SolutionPath.c_str(), m_Build);
  }
}

void MSVSCodeBuild::CreateMapping(const FileDictionary& dictionary)
{
  m_Mapping.clear();
  boost::filesystem::path solutionPath = boost::filesystem::path(m_SolutionPath).parent_path();
  std::string librarySection = "LIBRARY\\";
  for (size_t moduleIdx = 0; moduleIdx < m_Build.GetModulesCount(); ++moduleIdx)
  {
    const WhatIBuild::Module& module = m_Build.GetModule(moduleIdx);
    std::string translationUnitPath = librarySection + module.GetName() + "\\";
    for (size_t unitIdx = 0; unitIdx < module.GetUnitsCount(); ++unitIdx)
    {
      const WhatIBuild::Unit& unit = module.GetUnit(unitIdx);

      // First search for LIBRARY/<PROJECT>/file
      std::string shortPath = translationUnitPath + unit.GetFileName();
      size_t handle = dictionary.FindHandle(shortPath);
      if (handle == FileDictionary::NO_INDEX)
      {
        handle = dictionary.FindHandle(unit.GetPath());
      }

      if (handle != FileDictionary::NO_INDEX)
      {
        UnitIndex index;
        index.moduleIndex = moduleIdx;
        index.unitIndex = unitIdx;
        assert(m_Mapping.find(handle) == m_Mapping.end());
        m_Mapping[handle] = index;
      }
    }
  }
}

const std::string& MSVSCodeBuild::ResolveName(const FileDictionary& dictionary, size_t handle, std::string& name) const
{
  name.clear();
  auto it = m_Mapping.find(handle);
  if (it != m_Mapping.end())
  {
    const UnitIndex& index = it->second;
    const WhatIBuild::Module& module = m_Build.GetModule(index.moduleIndex);
    const WhatIBuild::Unit& unit = module.GetUnit(index.unitIndex);
    name += module.GetName();
    name += '\\';
    name += unit.GetFileName();
    return name;
  }
  return dictionary.GetFileName(handle, name);
}

unsigned int MSVSCodeBuild::GetFileModule(size_t handle) const
{
  auto it = m_Mapping.find(handle);
  if (it != m_Mapping.end())
  {
    const UnitIndex& index = it->second;
    return index.moduleIndex;
  }
  return EXTERNAL_MODULE;
}

const std::string& MSVSCodeBuild::GetModuleName(size_t moduleIndex) const
{
  if (moduleIndex != EXTERNAL_MODULE)
  {
    const WhatIBuild::Module& module = m_Build.GetModule(moduleIndex);
    return module.GetName();
  }

  static std::string external("EXTERNAL");
  return external;
}

CodeBuild* CreateMSVSCodeBuild(const char* path)
{
  MSVSCodeBuild* codeBuild = new MSVSCodeBuild();
  codeBuild->LoadSolution(path);
  return codeBuild;
}
