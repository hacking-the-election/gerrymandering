#include <iostream>
#include <vector>
#include "../include/util.hpp"
#include <dirent.h>
#include <stdio.h>

void create_json(vector<string> args, string write_path) {
    string file = "{\"type\":\"FeatureCollection\", \"features\": [\n";

    for (string path : args) {
        cout << path << endl;
        file += readf(path) + ",\n";
    }

    file = file.substr(0,file.size() - 2);
    file += "\n]}";

    ofstream f(write_path);
    f << file;
    f.close();
}

int main(int argc, char* argv[]) {

    const char* PATH = argv[1];
    DIR *dir = opendir(PATH);
    struct dirent *entry = readdir(dir);

    string prefix = "";
    vector<string> args = {};

    while (entry != NULL) {
        if (entry->d_type == DT_DIR) {
            if (prefix != split(string(entry->d_name), "-")[0]) {

                if (prefix != "") {
                    create_json(args, string(PATH) + string("/") + prefix + ".geojson");
                }

                prefix = split(string(entry->d_name), "-")[0];
                cout << "new state " << prefix << endl;
                args = {};
            }

            args.push_back(string(PATH) + string("/") + string(entry->d_name) + "/shape.geojson");
        }
        entry = readdir(dir);
    }

    closedir(dir);

    return 0;
}
