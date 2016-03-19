#include <vector>

using std::vector;

struct sat_stats {
    sat_stats(short max_delta, short max_val): max_delta(max_delta), max_value(max_val) {};
    int max_delta;
    int max_value;
};

struct position {
    position(): lat(0), lon(0) {}
    position(int lat, int lon): lat(lat), lon(lon) {};
    int lat;
    int lon;
};

struct sat_position {
    sat_position(int lat, int lon, int vel): pos(lat, lon), vel(vel), last_photo(0), last_photo_offset() {};
    position pos;
    int vel;
    int last_photo; // when last photo has been taken
    position last_photo_offset;
};

struct time_range {
    time_range(int start, int end): start(start), end(end) {};
    int start;
    int end;
};

struct photo_request {
    photo_request(int from, int from_nr, const position &pos, const vector<time_range> &ranges):
        from(from), from_nr(from_nr), pos(pos), ranges(ranges), finished(false) {}
    int from; // which collection
    int from_nr; // which position in collection
    const position &pos;
    const std::vector<time_range> &ranges;
    bool finished;
};

struct photo_made {
    position pos;
    int sat;
    int time;
};

struct collection_info {
    collection_info(int nr, int val): nr(nr), value(val), eliminated(false) {};
    int nr;
    int value;
    bool eliminated;
    vector<position> locations;
    vector<time_range> ranges;
    photo_request request(int which) {
        return photo_request(nr, which, locations[which], ranges);
    }
};
