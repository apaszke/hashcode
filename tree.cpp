#include "main.h"
#include <cassert>
#include <iostream>
using namespace std;

struct tree {
    // <sx; ex>
    int sx, ex;
    int sy, ey;
    int number_photos;
    tree* _subtree[2][2];

    vector<photo_request*> content;

    bool is_leaf() {
        return (sx == ex && sy == ey);
    }

    // last of subtree[0]
    int mx() {
        return sx + (ex - sx) / 2;
    }
    int my() {
        return sy + (ey - sy) / 2;
    }

    tree(int sx, int ex, int sy, int ey): sx(sx), ex(ex), sy(sy), ey(ey){
//        cout<<"TREE CREATED"<<sx<<" "<<ex<<" "<<sy<<" "<<ey<<endl;
        assert(sx <= ex && sy <= ey);
        number_photos = 0;
        _subtree[0][0] = 0;
        _subtree[0][1] = 0;
        _subtree[1][0] = 0;
        _subtree[1][1] = 0;
    }

    tree* get_subtree(int x, int y) {
        //cout << "mx" << mx() << " my " << my() << endl;
        if(x <= mx()) {
            if(y <= my()) {
                if(_subtree[0][0] == 0)
                    _subtree[0][0] = new tree(sx, mx(), sy, my());
                return _subtree[0][0];
            }else {
                if(_subtree[0][1] == 0)
                    _subtree[0][1] = new tree(sx, mx(), my()+1, ey);
                return _subtree[0][1];
            }
        } else {
            if(y <= my()) {
                if(_subtree[1][0] == 0)
                    _subtree[1][0] = new tree(mx() + 1, ex, sy, my());
                return _subtree[1][0];
            }else {
                if(_subtree[1][1] == 0)
                    _subtree[1][1] = new tree(mx() + 1, ex, my()+1, ey);
                return _subtree[1][1];
            }
        }
    }

    void add(photo_request *photo, int x, int y) {
        //cout<<"WTF pos "<<x<<" "<<y<<endl;
        //cout<<"X "<<sx<<" "<<ex<<endl;
        //cout<<"Y "<<sy<<" "<<ey<<endl;
        assert(sx <= x && x <= ex && sy <= y && y <= ey);
        number_photos += 1;
        if(is_leaf()) {
            assert(sx == ex && sy == ey);
            content.push_back(photo);
        } else {
            assert(sx != ex || sy != ey);
            get_subtree(x, y)->add(photo, x, y);
        }
    }

    int remove(photo_request *photo, int x, int y) {
        assert(sx <= x && x <= ex && sy <= y && y <= ey);
        if(is_leaf()) {
            assert(sx == ex && sy == ey);
            for(auto it = content.begin(); it != content.end(); it++) {
                if(*it == photo) {
                    content.erase(it);
                    number_photos -= 1;
                    return 1;
                }
            }
            return 0;
        } else {
            assert(sx != ex || sy != ey);
            int rnum = get_subtree(x, y)->remove(photo, x, y);
            number_photos -= rnum;
            return rnum;
        }
    }

    void add_to_vector(vector<photo_request*> &vec, int psx, int pex, int psy, int pey) {
        if(number_photos == 0)
            return;
        if(ex < psx || pex < sx || ey < psy || pey < sy)
            return;
        if(is_leaf()) {
            assert(psx <= sx && ex <= pex && psy <= sy && ey <= pey);
            for(auto it = content.begin(); it != content.end(); it++) {
                vec.push_back(*it);
            }
        } else {
            if(_subtree[0][0])
                _subtree[0][0]->add_to_vector(vec, psx, pex, psy, pey);
            if(_subtree[0][1])
                _subtree[0][1]->add_to_vector(vec, psx, pex, psy, pey);
            if(_subtree[1][0])
                _subtree[1][0]->add_to_vector(vec, psx, pex, psy, pey);
            if(_subtree[1][1])
                _subtree[1][1]->add_to_vector(vec, psx, pex, psy, pey);

        }
    }

    vector<photo_request*> get_area(int psx, int pex, int psy, int pey) {
        vector<photo_request*> result;
        add_to_vector(result, psx, pex, psy, pey);
        return result;
    }
};
