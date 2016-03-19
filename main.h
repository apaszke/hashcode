#include <vector>

using std::vector;

struct sat_stats {
    sat_stats(short max_delta, short max_val): max_orient_change(max_delta), max_orient_value(max_val) {};
    short max_orient_change;
    short max_orient_value;
};

struct position {
    position(int lat, int lon): lat(lat), lon(lon) {};
    int lat;
    int lon;
};

struct sat_position {
    sat_position(int lat, int lon, int vel): pos(lat, lon), vel(vel), last_photo(0) {};
    position pos;
    int vel;
    int last_photo; // when last photo has been taken
};

struct time_range {
    time_range(int start, int end): start(start), end(end) {};
    int start;
    int end;
};

struct collection_info {
    collection_info(int val, int num_loc, int num_ran): value(val) {};
    int value;
    vector<position> locations;
    vector<time_range> ranges;
};
