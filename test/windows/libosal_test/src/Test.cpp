#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"
#include "mutex_test.h"
#include "semaphore_test.h"

bool runAllTests( int argc, char const *argv[] )
{
  cute::xml_file_opener                     xmlfile( argc, argv );
  cute::xml_listener<cute::ide_listener<> > lis( xmlfile.out );
  auto                                      runner = cute::makeRunner( lis, argc, argv );

  bool                                      success;

  cute::suite                               libosal_mutex_suite = make_suite_mutex_test();
  success &= runner( libosal_mutex_suite, "libosal_mutex_suite" );

  cute::suite                               libosal_semaphore_suite = make_suite_semaphore_test();
  success &= runner( libosal_semaphore_suite, "libosal_semaphore_suite" );

  return ( success );
}

int main( int argc, char const *argv[] )
{
  return ( runAllTests( argc, argv ) ? EXIT_SUCCESS : EXIT_FAILURE );
}