#include "config.h"
namespace sylar {
    ptr<ConfigVarBase> Config::LookupBase(const std::string& name) {
        ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        return it == GetDatas().end() ? nullptr : it->second;
    }

    void Config::Visit(std::function<void(ptr<ConfigVarBase>)> cb) {
        ReadLock lock(GetMutex());
        ConfigVarMap& m = GetDatas();
        for (auto& [key, ptr] : m) {
            cb(ptr);
        }
    }

    static void ListAllMember(const std::string& prefix, const YAML::Node& node, std::list<std::pair<std::string, const YAML::Node>>& output) {
        if (prefix.find_first_not_of("abcdefghijklmnopqrstuivwxyz._012345678") != prefix.npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
            return;
        }
        output.emplace_back(prefix, node);
        if (node.IsMap()) {
            for (auto it = node.begin(); it != node.end(); ++it) {
                ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
            }
        }
    }

    void Config::LoadFromYaml(const YAML::Node& root) {
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        ListAllMember("", root, all_nodes);
        for (auto& [key, node] : all_nodes) {
            if (key.empty()) {
                continue;
            }
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            auto var = LookupBase(key);
            if (var) {
                if (node.IsScalar()) {
                    var->fromString(node.Scalar());
                } else {
                    std::stringstream ss;
                    ss << node;
                    var->fromString(ss.str());    
                }
            }
        }
    }
}