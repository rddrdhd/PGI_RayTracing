#ifndef TUTORIALS_H_
#define TUTORIALS_H_

void error_handler( void * user_ptr, const RTCError code, const char * str = nullptr );

int tutorial_1( const char * config = "threads=0,verbose=3" );
int tutorial_2();
int raytrace_loop( const std::string object_file_name, const std::string background_file_name, const char * config = "threads=0,verbose=0" );

#endif
