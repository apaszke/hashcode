#include <stdio.h>
#include <vector>
#include <iostream>
#include <cassert>

using std::vector;
using std::cout;
using std::endl;

struct sat_stats {
    sat_stats(short max_delta, short max_val): max_orient_change(max_delta), max_orient_value(max_val) {};
    short max_orient_change;
    short max_orient_value;
};

struct sat_position {
    sat_position(int lat, int lon, int vel): lat(lat), lon(lon), vel(vel), last_photo(0) {};
    int lat;
    int lon;
    int vel;
    int last_photo; // when last photo has been taken
};

struct time_range {
    time_range(int start, int end): start(start), end(end) {};
    int start;
    int end;
};

struct position {
    position(int lat, int lon): lat(lat), lon(lon) {};
    int lat;
    int lon;
};

struct collection_info {
    collection_info(int val, int num_loc, int num_ran): value(val) {};
    int value;
    vector<position> locations;
    vector<time_range> ranges;
};

int MAX_TIME;
int CURRENT_TIME;
vector<sat_stats> STATS;
vector<sat_position> POSITIONS;
vector<collection_info> COLLECTIONS;

void moveForward(int time) {
    assert(time > 0);
}

void loadData() {
    CURRENT_TIME = 0;
    scanf("%d", &MAX_TIME);

    int num_satellites;
    scanf("%d", &num_satellites);
    STATS.reserve(num_satellites);
    POSITIONS.reserve(num_satellites);

    int lat, lon, vel, max_delta, max_val;
    for (int i = 0; i < num_satellites; ++i) {
        scanf("%d %d %d %d %d", &lat, &lon, &vel, &max_delta, &max_val);
        POSITIONS.emplace_back(lat, lon, vel);
        STATS.emplace_back(max_delta, max_val);
    }

    int num_collections;
    scanf("%d", &num_collections);
    COLLECTIONS.reserve(num_collections);
    for (int i = 0; i < num_collections; ++i) {
        int value, num_locations, num_ranges;
        scanf("%d %d %d", &value, &num_locations, &num_ranges);
        COLLECTIONS.emplace_back(value, num_locations, num_ranges);
        auto& collection = COLLECTIONS.back();
        for (int k = 0; k < num_locations; ++k) {
            int lat, lon;
            scanf("%d %d", &lat, &lon);
            collection.locations.emplace_back(lat, lon);
        }

        for (int k = 0; k < num_ranges; ++k) {
            int start, end;
            scanf("%d %d", &start, &end);
            collection.ranges.emplace_back(start, end);
        }
    }
}

void debugData() {
    for (auto& x: COLLECTIONS) {
        cout << "=======" << endl;
        cout << x.value << endl;
        for (auto& m: x.locations) {
            cout << m.lat << " " << m.lon << endl;
        }
        for (auto& m: x.ranges) {
            cout << m.start << " " << m.end << endl;
        }
    }
}

int main() {
    loadData();
    for (; CURRENT_TIME < MAX_TIME; ++CURRENT_TIME) {
        moveForward(1);
        // TODO: Barzo
    }
}
