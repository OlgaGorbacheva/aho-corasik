#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>

struct vertex {
   bool leaf; //// -1, если не лист, иначе - номер строки для которой вершиная является листом
   std::weak_ptr<vertex> parent;
   std::weak_ptr<vertex> root;
   char path; //буква по которой мы пришли из предка
   std::unordered_map <char, std::weak_ptr<vertex>> children; // буква - переход из вершины
   std::weak_ptr<vertex> link;
   std::weak_ptr<vertex> up;
   int sample_number;

   std::weak_ptr<vertex> get_link() {
       if (link.lock().get() == NULL) {
           if (parent.lock().get() == NULL || parent.lock()->parent.lock().get() == NULL) {//root или ребенок root
               link = root;
           }
           else {
               char c = path;
               link = parent.lock()->get_link();
               link = link.lock()->go(c);
           }
       }
       return link;
    }

    std::weak_ptr<vertex> go (char c) {
        std::weak_ptr<vertex> next;
        auto itr = children.find(c);
        if (itr != children.end()) {
            next = itr->second;
        } else {
            if (parent.lock().get() == NULL) {
                    next = root;
                }
                else next = get_link().lock()->go(c);
        }
        return next;
    }

   std::weak_ptr<vertex> get_up() {
       if (up.lock().get() == NULL) {
           if (get_link().lock()->leaf) {
               up = get_link();
           } else
               if ((get_link().lock())->parent.lock().get() != NULL)
                    up = get_link().lock()->get_up();
       }
       return up;
   }
};

struct Bohr {
    std::weak_ptr<vertex> root;
    std::vector<std::shared_ptr<vertex>> vertexes;
    std::vector<std::string> samples;
};

std::istream & make_bohr(std::istream &cin, Bohr &bohr) {
    std::weak_ptr<vertex> link(static_cast<std::shared_ptr<vertex>>(NULL));
    int n; // кол-во строк;
    char current;
    std::string sample;
    cin >> n;
    std::shared_ptr<vertex> new_ver(new vertex);
    std::weak_ptr<vertex> ver(new_ver);
    std::weak_ptr<vertex> par(static_cast<std::shared_ptr<vertex>>(NULL));
    ver.lock()->parent = par;
    ver.lock()->path = '\0';
    ver.lock()->link = link;
    ver.lock()->up = link;
    ver.lock()->root = ver;
    std::weak_ptr<vertex> root = ver;
    bohr.root = ver;
    bohr.vertexes.push_back(new_ver);
    for (int i = 0; i < n; i++) {
        while ((current = cin.get()) == '\n') {
        }
        sample = "";
        par = bohr.root;
        while (current != '\n') {
            auto itr = par.lock()->children.find(current);
            sample.push_back(current);
            if (itr != par.lock()->children.end()) {
                par = itr->second;
                current = cin.get();
            } else {
                std::shared_ptr<vertex> sh_new_ver(new vertex);
                std::weak_ptr<vertex> new_ver(sh_new_ver);
                bohr.vertexes.push_back(sh_new_ver);
                ver = new_ver;
                ver.lock()->parent = par;
                ver.lock()->path = current;
                ver.lock()->link = link;
                ver.lock()->up = link;
                ver.lock()->parent.lock()->children.insert(std::make_pair(current, ver));
                ver.lock()->root = root;
                ver.lock()->leaf = false;
                par = ver;
                current = cin.get();
            }
        }
        par.lock()->leaf = true;
        bohr.samples.push_back(sample);
        par.lock()->sample_number = i;
    }
    return cin;
}

std::istream & find(std::istream &cin, Bohr &bohr, std::vector<std::vector<int>> &positions) {
    positions.resize(bohr.samples.size());
    char current = cin.get();
    int i = 0; // длина текста
    std::weak_ptr<vertex> ver = bohr.root;
    while (!cin.eof()) {
        ver = ver.lock()->go(current);
        if (ver.lock()->leaf) {
            positions[ver.lock()->sample_number].push_back(i - bohr.samples[ver.lock()->sample_number].length() + 1);
        }
        std::weak_ptr<vertex> up = ver.lock()->get_up();
        while(up.lock().get() != NULL) {
            if (up.lock()->leaf) {
                positions[up.lock()->sample_number].push_back(i - bohr.samples[up.lock()->sample_number].length() + 1);
            }
            up = up.lock()->get_up();
        }
        current = cin.get();
        i++;
    }
    return cin;
}

int main() {
    std::ifstream fin("input.txt");
    if (!fin.is_open()) {
        std::cerr << "Файла нет" << std::endl;
        return 1;
    }
    Bohr bohr;
    make_bohr(fin, bohr);
    std::vector<std::vector<int>> positions;
    find(fin, bohr, positions);
    for (unsigned int i = 0; i < bohr.samples.size(); i++) {
        std::cout << bohr.samples[i] << std::endl;
        for (unsigned int j = 0; j < positions[i].size(); j++)
            std::cout << positions[i][j] << ' ';
        std::cout << std::endl;
    }
    return 0;
}
