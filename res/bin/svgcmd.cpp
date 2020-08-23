#include <stdlib.h>
#include <iostream>
#include <string>
#include <regex>

#define LINE_MAX 1024

using std::cout;
using std::endl;

int GetCommand(const std::string &path, int &new_index, char &command)
{
        std::regex rx_command("[[:space:]]*([MZLHVCSQTA])", std::regex::icase);
        std::smatch command_match;

        if( std::regex_search( path, command_match, rx_command) && command_match.size() > 1 )
        {
                // Find the first match only
                std::cout << command_match.str(1) << endl;
                command = command_match.str(1)[0];
                new_index = command_match.position(1) + command_match.length(1);
                return true;
        }
        return false;
}

int GetCoords(const std::string &path, int &new_index, double &x, double &y)
{
        /* std::regex rx_coords("[[:space:]]*((\\+|-)?[[:digit:]]+(\\.(([[:digit:]]+)?))?((e|E)((\\+|-)?)[[:digit:]]+)?)[[:space:]]*,?[[:space:]]*((\\+|-)?[[:digit:]]+(\\.(([[:digit:]]+)?))?((e|E)((\\+|-)?)[[:digit:]]+)?)"); */
        std::regex rx_coords("[[:space:]]*([[:digit:]]+)");
        std::smatch coords_match;

        cout << "String: " << path << endl;
        if( std::regex_match( path, coords_match, rx_coords) )
        {
                // Find the first match only
                //x = std::stod( coords_match[0].str() );
                //cout << x << endl;
                cout << "String: " << path << endl;
                for( size_t i = 1; i < coords_match.size(); i++ ) {
                        std::ssub_match sm = coords_match[i];
                        cout << "Submatch " << i << " : " << sm.str() << endl;
                }
                /* cout << "Match Position: " << coords_match.position() << endl; */
                /* cout << "Match Length: " << coords_match.length() << endl; */
                /* cout << "Match String: " << coords_match[0].str() << endl; */
                new_index = coords_match[0].length() + 1;
                return true;
        } else {
                cout << "No coordinate match found" << endl;
        }
        return false;
}

void AddLine(double start_x, double start_y, double end_x, double end_y)
{
        cout << "Addline" << endl;
        //std::cout << start_x << ", " << start_y << ", ";
        //std::cout << end_x << ", " << end_y << std::endl;
}

int main(int argc, char *argv[])
{
        // Read a line from stdin
        char line[LINE_MAX];
        std::istream *result;
        while ((result = &std::cin.getline(line, LINE_MAX))) {
                int index = 0, new_index, result;
                double x, y, last_x, last_y;
                char command;

                // Find the first m and coordinate pair
                result = GetCommand(std::string((char*)&line), new_index, command);
                if (command != 'M' && command != 'm') {
                        cout << "Command is not M or m" << endl;
                        return -1;
                }
                if (!(result = GetCoords(std::string((char *)&line + new_index), new_index, last_x,
                                         last_y))) {
                        cout << "Failed to get coordinates" << endl;
                        return -2;
                }
                cout << "Start Coordinates: " << last_x << ", " << last_y << endl;

                while ((result = GetCoords((char *)&line+new_index, new_index, x, y))) {
                        AddLine(last_x, last_y, x, y);
                        last_x = x;
                        last_y = y;
                        index += new_index;
                }


                /* switch( command ) { */
                /*      case ' */
                //
                // Iterate through any more coordinate pairs as lines
                //
                // Do:  Get the next character and coordinates
                //
                //      Iterate through any more coordinate pairs
                //
                // While there are any geometries left
        }

        return 0;
}
