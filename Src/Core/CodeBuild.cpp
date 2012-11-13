#include "Precompile.h"
#include "Core\CodeBuild.h"
#include "MSVSCodeBuild.h"

CodeBuild::~CodeBuild()
{

}

CodeBuild* CodeBuild::Create(CodeBuild::CodeBuildType codeBuildType, const char* path)
{
  switch(codeBuildType)
  {
  case e_MSVSSolution: return CreateMSVSCodeBuild(path);
  default:
    return NULL;
  }
}