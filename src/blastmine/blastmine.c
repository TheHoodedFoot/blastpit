#include <syslog.h>

int main( int argc, char ** argv )
{
    setlogmask( LOG_UPTO( LOG_WARN ) );
    syslog( LOG_INFO, "You'll never see me!" );
    syslog( LOG_WARN, "Now you see me." );

    return 0;
}
