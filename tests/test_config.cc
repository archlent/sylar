#include "sylar/config.h"
#include <yaml-cpp/yaml.h>

struct Person {
    std::string name;
    int age = 0;
    bool sex = 0;
    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name = " << name << ", age = " << age << ", sex = " << sex << "]";
        return ss.str();
    }
    bool operator==(const Person& other) const {
        return name == other.name && age == other.age && sex == other.sex;
    }
};

namespace sylar {
    template<>
    struct LexicalCast<std::string, Person> {
        decltype(auto) operator() (const std::string& v) {
            auto node = YAML::Load(v);
            std::stringstream ss;
            Person p;
            p.name = node["name"].as<std::string>();
            p.age = node["age"].as<int>();
            p.sex = node["sex"].as<bool>();
            return p;
        }
    };

    template<>
    struct LexicalCast<Person, std::string> {
        decltype(auto) operator() (const Person& p) {
            YAML::Node node;
            node["name"] = p.name;
            node["age"] = p.age;
            node["sex"] = p.sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
}

auto g_person = sylar::Config::Lookup("class.person", Person{"yyds", 1, false}, "system person");
auto g_person_map = sylar::Config::Lookup("class.map", std::map<std::string, Person>(), "person map");
auto g_person_vec_map = sylar::Config::Lookup("class.map_vec", std::map<std::string, std::vector<Person>>(), "person map vector");

void test_class() {
#define ZZ(g_var, prefix) \
    for (auto& [str, p] : g_var->getValue()) { \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << prefix << ": {" << str << " - " << p.toString() << "}"; \
    }
#define FF(g_var, prefix) \
    for (auto& [str, vec] : g_var->getValue()) { \
        SYLAR_LOG_FMT_INFO(SYLAR_LOG_ROOT(), "%s : { %s - ", prefix, str.c_str()); \
        for (auto& v : vec) { \
            std::cout << v.toString() << " "; \
        } \
        std::cout << "}\n"; \
    }

    g_person->addListener([](const Person& old_value, const Person& new_value) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "old_value = " << old_value.toString() << ", new_value = " << new_value.toString();
    });

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
    ZZ(g_person_map, "class.map before");
    FF(g_person_vec_map, "class.map_vecor before");
    YAML::Node root = YAML::LoadFile("../txt/test.yaml");
    sylar::Config::LoadFromYaml(root);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    ZZ(g_person_map, "class.map after");
    FF(g_person_vec_map, "class.map_vector after");
    //SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_person_vec_map->toString(); 
}

auto g_int_value_config = sylar::Config::Lookup("system.port", (int)8080, "system.port");
//auto g_intx_value_config = sylar::Config::Lookup("system.port", (float)8080, "system.port");
auto g_float_value_config = sylar::Config::Lookup("system.value", (float)10.2f, "system value");
auto g_vec_value_config = sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2, 3, 4, 5}, "vector int");
auto g_list_value_config = sylar::Config::Lookup("system.int_list", std::list<int>{6, 7, 8, 9}, "list int");
auto g_set_value_config = sylar::Config::Lookup("system.int_set", std::set<int>{10, 11, 12}, "set int");
auto g_unordered_set_value_config = sylar::Config::Lookup("system.int_unordered_set", std::unordered_set<int>{13, 14}, "unordered_set int");
auto g_map_value_config = sylar::Config::Lookup("system.str_int_map", std::map<std::string ,int>{{"k", 10}}, "map string int");
auto g_unordered_map_value_config = sylar::Config::Lookup("system.str_int_unordered_map", std::unordered_map<std::string ,int>{{"j", 34}}, "unordered_map string int");

void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(4 * level, ' ') << node.Scalar() << " - " <<  node.Type() << " - " << level;
    } else if (node.IsNull()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(4 * level, ' ') << "Null - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(4 * level, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(4 * level, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("../txt/test.yaml");
    print_yaml(root, 0);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root.Scalar();
}

void test_config() {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_float_value_config->toString();
    
#define XX(g_var, name, prefix) \
    for (auto& i : g_var->getValue()) { \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix << " " << #name << ": " << i; \
    } 
    //SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix << " " << #name << " yaml: " << g_var->toString(); 

#define YY(g_var, name, prefix) \
    for (auto& [x, y] : g_var->getValue()) { \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix << " " << #name << ": {" << x << " - " << y << "}"; \
    }

    XX(g_vec_value_config, int_vec, before);
    XX(g_list_value_config, int_list, before);
    XX(g_set_value_config, int_set, before);
    XX(g_unordered_set_value_config, int_unordered_set, before);
    
    YY(g_map_value_config, str_int_map, before);
    YY(g_unordered_map_value_config, str_int_unordered_map, before);

    YAML::Node root = YAML::LoadFile("../txt/test.yaml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_vec_value_config, int_vec, after);
    XX(g_list_value_config, int_list, after);
    XX(g_set_value_config, int_set, after);
    XX(g_unordered_set_value_config, int_unordered_set, after);

    YY(g_map_value_config, str_int_map, after);
    YY(g_unordered_map_value_config, str_int_unordered_map, after);

}

void test_log() {
    static ptr<sylar::Logger> system_log = SYLAR_LOG_NAME("system");
    SYLAR_LOG_INFO(system_log) << "hello system";
    std::cout << "----------------------------------------------------------------------------\n";
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("../txt/log.yaml");
    sylar::Config::LoadFromYaml(root);
    std::cout << "============================================================================" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    SYLAR_LOG_INFO(system_log) << "hello system.txt";
    system_log->setFormatter("%d - %m%n");
    SYLAR_LOG_INFO(system_log) << "hello world";
}

int main() {
    //test_config();

    // test_yaml();
    //test_class();

    //test_log();

    sylar::Config::Visit([](ptr<sylar::ConfigVarBase> var) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "name = [" << var->getName() << "],\tdescription = \"" << var->getDescription() 
         << "\"\ttypename => " << var->getTypeName() << "\tvalue = " << var->toString();
    });

    return 0;
}