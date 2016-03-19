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

int MAX_TIME;
int CURRENT_TIME;
int NUM_SATELLITES;
int NUM_COLLECTIONS;
vector<sat_stats> STATS;
vector<sat_position> POSITIONS;
vector<collection_info> COLLECTIONS;

const int MAX_LON =  647999;
const int MIN_LON = -648000;
const int MAX_LAT =  324000;
const int MIN_LAT = -324000;
const int LON_CHANGE = -15;

void wrapPosition(position& p) {
    if (p.lat < MIN_LAT) {
        p.lat = 2 * MIN_LAT - p.lat;
        p.lon = MIN_LON + p.lon;
    } else if (p.lat > MAX_LAT) {
        p.lat = 2 * MAX_LAT - p.lat;
        p.lon = MIN_LON + p.lon;
    }

    if (p.lon < MIN_LON) {
        p.lon = (p.lon - MIN_LON) + MAX_LON + 1;
    } else if (p.lon > MAX_LON) {
        p.lon = (p.lon - MAX_LON) + MIN_LON - 1;
    }

}

bool shouldWrapSpeed(position& p) {
    return (p.lat < MIN_LAT) || (p.lat > MAX_LAT);
}

void moveForward(int time) {
    assert(time > 0);
    for (auto& sat: POSITIONS) {
        sat.pos.lon += LON_CHANGE * time;
        sat.pos.lat += sat.vel * time;
        if (shouldWrapSpeed(sat.pos)) {
            sat.vel = -sat.vel;
        }
        wrapPosition(sat.pos);
    }
}

void loadData() {
    CURRENT_TIME = 0;
    scanf("%d", &MAX_TIME);

    int num_satellites;
    scanf("%d", &num_satellites);
    NUM_SATELLITES = num_satellites;
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
    NUM_COLLECTIONS = num_collections;
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
    }
}
