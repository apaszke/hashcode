#include <stdio.h>
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <limits>
#include <string>
#include "main.h"

using std::vector;
using std::cout;
using std::endl;
using std::min;
using std::max;

const int MAX_COLLECTIONS = 12000;


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

bool canShootPhoto(int current_time, int satellite_idx, photo_request photo) {
    const auto& stats = STATS[satellite_idx];
    const auto& sat = POSITIONS[satellite_idx];
    int time_since_last_photo = current_time - sat.last_photo;
    int max_movement = time_since_last_photo * stats.max_delta;
    bool lat_ok = false;
    bool lon_ok = false;

    int lat_min = sat.pos.lat + max(sat.last_photo_offset.lat - max_movement, -stats.max_value);
    int lat_max = sat.pos.lat + min(sat.last_photo_offset.lat + max_movement, stats.max_value);
    lat_min = max(lat_min, MIN_LAT);
    lat_max = min(lat_max, MAX_LAT);
    int lon_min = sat.pos.lon + max(sat.last_photo_offset.lon - max_movement, -stats.max_value);
    int lon_max = sat.pos.lon + min(sat.last_photo_offset.lon + max_movement, stats.max_value);

    position tmp(0, lon_min);
    position tmp2(0, lon_max);
    wrapPosition(tmp);
    wrapPosition(tmp2);
    lon_min = tmp.lon;
    lon_max = tmp2.lon;

    assert(lon_min >= MIN_LON);
    assert(lon_max <= MAX_LON);

    lat_ok = lat_min <= photo.pos.lat && photo.pos.lat <= lat_max;
    if (lon_min < lon_max) {
        lon_ok = lon_min <= photo.pos.lon && photo.pos.lon <= lon_max;
    } else {
        lon_ok = lon_max >= photo.pos.lon || photo.pos.lon >= lon_min;
    }

    return lat_ok && lon_ok;
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
        COLLECTIONS.emplace_back(i, value);
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

double MIN_DOUBLE = -std::numeric_limits<double>::infinity();

double COLLECTION_SCORE[MAX_COLLECTIONS];

double simple_score(collection_info &coll) {
    return double(coll.value) / coll.locations.size();
}

struct by_score {
    bool operator()(int x, int y) {
        return COLLECTION_SCORE[x] < COLLECTION_SCORE[y];
    }
};

void choose(vector<photo_request> &result_images, vector<int> &result_collections, int cut_off_num, int cut_off_value = MIN_DOUBLE) {
    for (int i = 0; i < NUM_COLLECTIONS; ++i) {
        if (COLLECTIONS[i].eliminated) {
            COLLECTION_SCORE[i] = MIN_DOUBLE;
        } else {
            COLLECTION_SCORE[i] = simple_score(COLLECTIONS[i]);
        }
    }
    int collections_order[NUM_COLLECTIONS];
    std::iota(collections_order, collections_order + NUM_COLLECTIONS, 0);
    std::sort(collections_order, collections_order + NUM_COLLECTIONS, by_score());
    for (int i = 0; i < NUM_COLLECTIONS - cut_off_num; ++i) {
        int which = collections_order[i];
        collection_info &coll = COLLECTIONS[which];
        if (COLLECTION_SCORE[which] > cut_off_value) {
            for (int j= 0; j < coll.locations.size() - cut_off_num; ++j) {
                result_images.push_back(coll.request(j));
            }
            result_collections.push_back(i);
        } else {
            coll.eliminated = true;
        }
    }
}

position get_offset(photo_made photo, int sat) {
    position photo_pos = photo.pos;
    position sat_pos = POSITIONS[sat].pos;
    return position(photo_pos.lat - sat_pos.lat, photo_pos.lon - sat_pos.lon);
}

void run(vector<photo_request> &images, vector<photo_made> &photos_made) {
    for (int cur_time = 0; cur_time < MAX_TIME; ++cur_time) {
        for (int sat = 0; sat < NUM_SATELLITES; ++sat) {
            for (auto &image: images) {
                if (image.finished)
                    continue;
                bool possible = canShootPhoto(cur_time, sat, image);
                if (possible) {
                    position pos = image.pos;
                    // we make photo at time cur_time and position pos
                    image.finished = true;
                    photo_made made;
                    made.pos = pos;
                    made.time = cur_time;
                    made.sat = sat;
                    POSITIONS[sat].last_photo = cur_time;
                    POSITIONS[sat].last_photo_offset = get_offset(made, sat);
                    photos_made.push_back(made);
                    break;
                }
            }
        }
        moveForward(1); // because we can shot at 0
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "USAGE %s <cutoff_value>", argv[0]);
        return 1;
    }
    double initial_cutoff = std::stod(argv[1]);
    loadData();
    vector<photo_request> images;
    vector<int> collection_ids;
    choose(images, collection_ids, 0, initial_cutoff);
    fprintf(stderr, "CHOSEN SET");
    for (auto i: images) {
        fprintf(stderr, "IMAGE %d %d (from %d)\n", i.pos.lat, i.pos.lon, i.from);
    }
    vector<photo_made> result;
    run(images, result);
    printf("%lu\n", result.size());
    for (auto r: result) {
        printf("%d %d %d %d\n", r.pos.lat, r.pos.lon, r.time, r.sat);
    }
    return 0;
}
