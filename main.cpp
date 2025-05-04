#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include <map>
#include <windows.h>
#include <filesystem>



using namespace std;
using json = nlohmann::json;
namespace fs = filesystem;


const string help_command = "--help";

const string pwd = "pwd";

const string run_mine = "run-mine";

const string add_mine = "add-mine";

const string list_mine = "list-mine";

const string run_project = "run-project";
const string delete_project = "delete-project";
const string init_project = "init-project";
const string list_project = "list-project";

fs::path getExecutableFolder() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    fs::path exePath = buffer;
    return exePath.parent_path(); // путь к папке, где exe
}

struct Build {
    std::string mods;
    std::string version;
};





struct Line {
    std::string content;     
    std::string key;         
    bool isComment = false;  
    bool isProperty = false; 
};

std::vector<Line> readPropertiesFile(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Line> lines;
    std::string line;

    while (std::getline(file, line)) {
        Line l;
        l.content = line;

        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == '!') {
            l.isComment = true;
        } else {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                l.key = line.substr(0, eq);
                l.key.erase(l.key.find_last_not_of(" \t\r\n") + 1);
                l.isProperty = true;
            }
        }

        lines.push_back(l);
    }

    return lines;
}

void setProperty(std::vector<Line>& lines, const std::string& key, const std::string& value) {
    for (auto& line : lines) {
        if (line.isProperty && line.key == key) {
            line.content = key + "=" + value;
            return;
        }
    }
    lines.push_back(Line{ key + "=" + value, key, false, true });
}

void writePropertiesFile(const std::string& filename, const std::vector<Line>& lines) {
    std::ofstream file(filename);
    for (const auto& line : lines) {
        file << line.content << "\n";
    }
}

void copy_clean(const fs::path& source, const fs::path& target) {
    if (fs::exists(target)) {
        std::error_code ec;
        fs::remove_all(target, ec);
        if (ec) {
               std::cerr << "Ошибка удаления: " << ec.message() << std::endl;
        }

    }

    fs::create_directories(target);

    for (const auto& entry : fs::directory_iterator(source)) {
        const auto& path = entry.path();
        fs::copy(path, target / path.filename(), fs::copy_options::recursive);
    }
}




int main(int argc, char* argv[]){
    string our_path = getExecutableFolder().string();
    fs::path our_path_p = getExecutableFolder();
    string path_tlauncher;
    string path_minecarft;


    string path_properties;
    string path_mods;
    map<string,Build> builds_list;

    int CounterId = 0;
    
    ifstream file(our_path_p / "conf.json");

    if (!file) {
        std::cerr << "Failed to open file!" << std::endl;
        return 1;
    }


    json j;
    file >> j;

    path_tlauncher = j["tlauncher_path"];
    path_minecarft = j["minecraft_path"];
    path_properties = path_tlauncher + "\\tlauncher-2.0.properties";
    path_mods = path_minecarft + "\\mods";


    for(const auto& [key,value] : j["builds"].items()){
        CounterId++;
        Build b;
        b.mods = value["mods"];
        b.version = value["version"];
        builds_list[key] = b;
    }



    file.close();


   if(argc < 2){
     cout << "Incorrect input try (  --help  )  " << endl;
     return 1;
   }

   string command = argv[1];

   if(command == help_command){
    cout << " run-mine  <id_builds> -> start minecraft game with mods and builds \n" 
    << " add-mine <version> <name> ->  add new build  \n" 
    << " run-mine  ->  start minecraft \n"
    << " list-mine -> get all and version builds \n"
    << " run-project  <id_projects>  -> start selected project \n" 
    << " init-project  -> init new project  \n"
    << " del-project <id_project>  ->  delete selected project"
    << " list-project   ->   get all project  \n" 
    << endl;
   }
   else if(command == pwd){
    cout << our_path << endl;
   }
   else if(command == run_mine){
      if(argc == 3){
        string id_build = argv[2];
        string game_version = builds_list[id_build].version;
        auto lines = readPropertiesFile(path_properties);
        setProperty(lines,"login.version.game",game_version);
        writePropertiesFile(path_properties,lines);

        string game_mods = (our_path_p / "builds" / builds_list[id_build].mods).string();
        
        copy_clean(game_mods , path_mods);
      }
      string path = path_minecarft + "\\Tlauncher.exe";
      system(path.c_str());
   }
   else if(command == add_mine){
      if(argc == 4){
        string name = argv[2];
        string name_mods = argv[3];
        std::ifstream in(our_path_p / "conf.json");
json config;
in >> config;
in.close();
cout << CounterId << endl;
config["builds"][to_string(CounterId)] = {
    {"mods", name_mods},
    {"version", name}
};

std::ofstream out(our_path_p / "conf.json");
out << config.dump(4);  
string path = (our_path_p / "builds" / name_mods).string();
fs::path target = our_path_p / "builds" / name_mods;
fs::path working_dir = fs::current_path();
fs::create_directories(path);
for (const auto& file : fs::directory_iterator(working_dir)) {
    if (fs::is_regular_file(file)) {
        fs::copy_file(file.path(), target / file.path().filename(),
                      fs::copy_options::overwrite_existing);
    }
}

out.close();

      }else{
        return 1;
      }
   }
   else if(command == list_mine){
      for(auto& build : builds_list){
         cout << build.first << " -m " << build.second.mods << " -v " << build.second.version << endl;
      }
   }
   else if(command == run_project){

   }
   else if(command == init_project){

   }
   else if(command == delete_project){

   }
   else{
    cout << "Incorrect input try ( --help )" << endl;
    return 1;
   }

    
   return 0;
}