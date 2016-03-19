#include <stdio.h>
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <limits>
#include <string>

using std::vector;
using std::cout;
using std::endl;

const int MAX_COLLECTIONS = 12000;

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
    position(): lat(0), lon(0) {}
    position(int lat, int lon): lat(lat), lon(lon) {};
    int lat;
    int lon;
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

int MAX_TIME;
int CURRENT_TIME;
int NUM_SATELLITES;
int NUM_COLLECTIONS;
vector<sat_stats> STATS;
vector<sat_position> POSITIONS;
vector<collection_info> COLLECTIONS;

void moveForward(int time) {
    assert(time > 0);
    //for (int i = 0; i <
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

void run(vector<photo_request> &images, vector<photo_made> &photos_made) {
    for (int sat = 0; sat < NUM_SATELLITES; ++sat) {
        // INIT SAT
    }
    for (int cur_time = 0; cur_time < MAX_TIME; ++cur_time) {
        for (int sat = 0; sat < NUM_SATELLITES; ++sat) {
            for (auto &image: images) {
                if (image.finished)
                    continue;
                bool possible = false;
                if (possible) {
                    position pos = image.pos;
                    // we make photo at time cur_time and position pos
                    image.finished = false;
                    photo_made made;
                    made.pos = pos;
                    made.time = cur_time;
                    made.sat = sat;
                    photos_made.push_back(made);
                    break;
                }
            }
        }
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
