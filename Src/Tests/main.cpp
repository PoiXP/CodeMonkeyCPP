#include "Precompile.h"
#include "Utils/Log.h"

int RunTests()
{
  try
  {
    return UnitTest::RunAllTests();
  }
  catch( ... )
  {
    return -1;
  }
}

int main()
{
  Log::Initialize("Tests", Log::e_LogWarning);
  int result = RunTests();
  Log::Shutdown();
  return result;
}