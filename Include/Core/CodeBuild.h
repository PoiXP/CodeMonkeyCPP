#ifndef CORE_CODE_BUILD_H
#define CORE_CODE_BUILD_H
#pragma once

class FileDictionary;

class CodeBuild
{
public:
  enum CodeBuildType
  {
    e_MSVSSolution,
  };

	static CodeBuild* Create(CodeBuildType codeBuildType, const char* path);

  virtual ~CodeBuild() = 0;
  virtual void                CreateMapping(const FileDictionary& dictionary) = 0;
  virtual const std::string&  ResolveName(const FileDictionary& dictionary, size_t handle, std::string& name) const = 0;
  virtual unsigned int        GetFileModule(size_t handle) const = 0;
  virtual const std::string&  GetModuleName(size_t moduleIndex) const = 0;
};

#endif //CORE_CODE_BUILD_H