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
vector<sat_position> BACKUP_POSITIONS;
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

const int BONUS_COMPLETE = 2;

double simple_score(collection_info &coll) {
    double basic_score = double(coll.value) / coll.locations.size();
    if (coll.complete) {
        return basic_score * BONUS_COMPLETE;
    } else {
        return basic_score;
    }
}

struct by_score {
    bool operator()(int x, int y) {
        return COLLECTION_SCORE[x] > COLLECTION_SCORE[y];
    }
};

void choose(vector<photo_request> &result_images, vector<int> &result_collections, int cut_off_num) {
    int eliminated_before_cnt = 0;
    for (int i = 0; i < NUM_COLLECTIONS; ++i) {
        if (!COLLECTIONS[i].eliminated) {
            COLLECTION_SCORE[i] = simple_score(COLLECTIONS[i]);
        } else {
            ++eliminated_before_cnt;
        }
    }
    int collections_order[NUM_COLLECTIONS];
    int active_cnt = 0;
    for (int i = 0; i < NUM_COLLECTIONS; ++i) {
        if (!COLLECTIONS[i].eliminated) {
            collections_order[active_cnt++] = i;
        }
    }
    fprintf(stderr, "ACTIVE_CNT %d\n", active_cnt);
    int cnt_eliminated_now = 0;
    std::sort(collections_order, collections_order + active_cnt, by_score());
    int taken_cnt = active_cnt - cut_off_num;
    for (int i = 0; i < active_cnt; ++i) {
        int which = collections_order[i];
        collection_info &coll = COLLECTIONS[which];
        assert(!coll.eliminated);
        if (i < taken_cnt) {
            for (int j= 0; j < coll.locations.size(); ++j) {
                result_images.push_back(coll.request(j));
            }
            result_collections.push_back(i);
        } else {
            coll.eliminated = true;
            ++cnt_eliminated_now;
        }
    }
    fprintf(stderr, "ELIMINATED NOW %d\nELIMINATED BEFORE %d\n", cnt_eliminated_now, eliminated_before_cnt);
}

position get_offset(photo_made photo, int sat) {
    position photo_pos = photo.pos;
    position sat_pos = POSITIONS[sat].pos;
    return position(photo_pos.lat - sat_pos.lat, photo_pos.lon - sat_pos.lon);
}

void run(vector<photo_request> &images, vector<photo_made> &photos_made) {
    POSITIONS = BACKUP_POSITIONS;
    int taken[MAX_COLLECTIONS];
    for (int coll_id = 0; coll_id < MAX_COLLECTIONS; ++coll_id) {
        taken[coll_id] = 0;
    }
    int info_step = MAX_TIME / 5;
    for (int cur_time = 0; cur_time < MAX_TIME; ++cur_time) {
        if (cur_time % info_step == 0) {
            fprintf(stderr, "TIME %d/%d\n", cur_time, MAX_TIME);
        }
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
                    ++taken[image.from];
                    break;
                }
            }
        }
        moveForward(1); // because we can shot at 0
    }
    for (int coll_id = 0; coll_id < MAX_COLLECTIONS; ++coll_id) {
        if (taken[coll_id] == COLLECTIONS[coll_id].locations.size()) {
            COLLECTIONS[coll_id].complete = true;
        }
    }
}

const int MAX_LEFT = 2;
const double LEFT_DROPPED_PART = 0.25;

int main(int argc, char **argv) {
    loadData();
    BACKUP_POSITIONS = POSITIONS;

    int left = 0;
    vector<photo_made> result;
    int iteration_nr = 0;
    vector<photo_made> best_result;
    int best_result_score = 0;
    while (iteration_nr == 0 || left > MAX_LEFT) {
        ++iteration_nr;
        fprintf(stderr, "ITERATION %d\n", iteration_nr);
        result = {};
        vector<photo_request> images;
        vector<int> collection_ids;
        int drop_cnt = left * LEFT_DROPPED_PART;
        if (drop_cnt == 0 && iteration_nr > 1) {
            drop_cnt = 1;
        }
        fprintf(stderr, "DROPPING %d\n", drop_cnt);
        choose(images, collection_ids, drop_cnt);
        fprintf(stderr, "CHOSEN IMAGES %lu\nCHOSEN COLLECTIONS %lu\n", images.size(), collection_ids.size());
        for (int i = 0; i < NUM_COLLECTIONS; ++i) {
            COLLECTIONS[i].complete = false;
        }
        run(images, result);
        int score = 0;
        left = collection_ids.size();
        for (int i = 0; i < NUM_COLLECTIONS; ++i) {
            if (COLLECTIONS[i].complete) {
                --left;
                score += COLLECTIONS[i].value;
            }
        }
        fprintf(stderr, "LEFT: %d\n", left);
        fprintf(stderr, "SCORE: %d\n", score);
        if (score > best_result_score) {
            fprintf(stderr, "BEST (before %d)\n", best_result_score);
            best_result_score = score;
            best_result = result;
        }
    }
    printf("%lu\n", best_result.size());
    for (auto r: best_result) {
        printf("%d %d %d %d\n", r.pos.lat, r.pos.lon, r.time, r.sat);
    }
    return 0;
}
