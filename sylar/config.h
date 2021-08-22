#pragma once

#include "log.h"
#include "mutex.h"
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <boost/type_index.hpp>

using boost::typeindex::type_id;

namespace sylar {
    class ConfigVarBase {
    public:
        ConfigVarBase(const std::string& name, const std::string& description) : 
            m_name(name), m_description(description) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase() {}
        const std::string& getName() const { return m_name; }
        const std::string& getDescription() const { return m_description; }

        virtual std::string toString() = 0;
        virtual bool fromString(const std::string& val) = 0;
        virtual decltype(type_id<int>()) getTypeName() const = 0;
    protected:
        std::string m_name;
        std::string m_description;
    };

    template <typename F, typename T>
    struct LexicalCast {
        T operator() (const F& v) {
            return boost::lexical_cast<T>(v);
        }
    };

    // vector
    template <typename T>
    struct LexicalCast<std::string, std::vector<T>> {
        decltype(auto) operator() (const std::string& v) {
            auto node = YAML::Load(v);
            std::stringstream ss;
            std::vector<T> vec;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.emplace_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template <typename T>
    struct LexicalCast<std::vector<T>, std::string> {
        decltype(auto) operator() (const std::vector<T>& vec) {
            YAML::Node node;
            for (auto& v : vec) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(v)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // list
    template <typename T>
    struct LexicalCast<std::string, std::list<T>> {
        decltype(auto) operator() (const std::string& v) {
            auto node = YAML::Load(v);
            std::stringstream ss;
            std::list<T> vec;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.emplace_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template <typename T>
    struct LexicalCast<std::list<T>, std::string> {
        decltype(auto) operator() (const std::list<T>& vec) {
            YAML::Node node;
            for (auto& v : vec) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(v)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // set
    template <typename T>
    struct LexicalCast<std::string, std::set<T>> {
        decltype(auto) operator() (const std::string& v) {
            auto node = YAML::Load(v);
            std::stringstream ss;
            std::set<T> vec;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.emplace(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template <typename T>
    struct LexicalCast<std::set<T>, std::string> {
        decltype(auto) operator() (const std::set<T>& vec) {
            YAML::Node node;
            for (auto& v : vec) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(v)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // unordered_set
    template <typename T>
    struct LexicalCast<std::string, std::unordered_set<T>> {
        decltype(auto) operator() (const std::string& v) {
            auto node = YAML::Load(v);
            std::stringstream ss;
            std::unordered_set<T> vec;
            for (size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.emplace(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template <typename T>
    struct LexicalCast<std::unordered_set<T>, std::string> {
        decltype(auto) operator() (const std::unordered_set<T>& vec) {
            YAML::Node node;
            for (auto& v : vec) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(v)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // map
    template <typename T>
    struct LexicalCast<std::string, std::map<std::string, T>> {
        decltype(auto) operator() (const std::string& v) {
            auto node = YAML::Load(v);
            std::stringstream ss;
            std::map<std::string, T> vec;
            for (auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.emplace(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template <typename T>
    struct LexicalCast<std::map<std::string, T>, std::string> {
        decltype(auto) operator() (const std::map<std::string, T>& vec) {
            YAML::Node node;
            for (const auto& [x, y] : vec) {
                node[x] = YAML::Load(LexicalCast<T, std::string>()(y));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // unordered_map
    template <typename T>
    struct LexicalCast<std::string, std::unordered_map<std::string, T>> {
        decltype(auto) operator() (const std::string& v) {
            auto node = YAML::Load(v);
            std::stringstream ss;
            std::unordered_map<std::string, T> vec;
            for (auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.emplace(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template <typename T>
    struct LexicalCast<std::unordered_map<std::string, T>, std::string> {
        decltype(auto) operator() (const std::unordered_map<std::string, T>& vec) {
            YAML::Node node;
            for (const auto& [x, y] : vec) {
                node[x] = YAML::Load(LexicalCast<T, std::string>()(y));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };


    template <typename T, typename FromStr = LexicalCast<std::string, T>, typename ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase {
    public:
        using RWmutexType = std::shared_mutex;
        using on_change_cb = std::function<void(const T& old_value, const T& new_value)>;

        ConfigVar(const std::string& name, const T& default_val, const std::string& description = "") 
            : ConfigVarBase(name, description), m_val(default_val) {
        }

        std::string toString() override {
            try {
                ReadLock lock(m_mutex);
                //return boost::lexical_cast<std::string>(m_val);
                return ToStr()(m_val);
            } catch(const std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Configval::toString exception" << e.what() << " convert:" 
                  << typeid(m_name).name() << "to String" << " " << m_name;
            }
            return "";
        }

        bool fromString(const std::string& val) override {
            try {
                //m_val = boost::lexical_cast<T>(val);
                setValue(FromStr()(val));
            } catch(const std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Configval::toString exception" << e.what() << " convert: string to " 
                  << type_id<decltype(m_name)>() << " " << m_name << " " << val;
            }
            return false;
        }

        decltype(type_id<int>()) getTypeName() const override {
            //return typeid(T).name();   // Linux下太简陋了，不够明确，建议使用boost的 type_id 或者 type_id_with_cur
            return type_id<T>();
        }

        const T& getValue() const { 
            ReadLock lock(m_mutex);
            return m_val; 
        }

        //void setValue(const T& value) { 
        //    upgrade_lock ReadLock(m_mutex);
        //    if (value == m_val) {
        //        return;
        //    }
        //    for (auto& [key, cb] : m_cbs) {
        //        cb(m_val, value);
        //    }
        //    upgrade_to_unique_lock WriteLock(ReadLock);
        //    m_val = value;
        //}

        void setValue(const T& value) { 
            {
                ReadLock lock(m_mutex);
                if (value == m_val) {
                    return;
                }
                for (auto& [key, cb] : m_cbs) {
                    cb(m_val, value);
                }
            }
            WriteLock lock(m_mutex);
            m_val = value; 
        } 

        uint64_t addListener(on_change_cb cb) {  
            static uint64_t key = 0;
            WriteLock lock(m_mutex);;
            ++key;
            m_cbs[key] = cb; 
            return key;
        }

        void delListener(uint64_t key) {  
            WriteLock lock(m_mutex);;
            m_cbs.erase(key); 
        }

        on_change_cb get_Listner(uint64_t key) const {  
            ReadLock lock(m_mutex);
            return m_cbs.find(key) == m_cbs.end() ? nullptr : m_cbs[key]; 
        }

        void clearListener() { 
            WriteLock lock(m_mutex);;
            m_cbs.clear(); 
        }
    private:
        T m_val;
        std::unordered_map<uint64_t, on_change_cb> m_cbs;  // 变更回调函数
        mutable RWmutexType m_mutex;
    };

    class Config {
    public:
        using ConfigVarMap = std::unordered_map<std::string, ptr<ConfigVarBase>>;
        using RWmutexType = std::shared_mutex;

        template <typename T>
        static ptr<ConfigVar<T>> Lookup(const std::string& name, const T& default_val, const std::string& description = "") {
            WriteLock lock(GetMutex());
            auto it = GetDatas().find(name);
            if (it != GetDatas().end()) {
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                if (tmp) {
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exisit";
                    return tmp;
                } else {
                    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exisit but type not " << type_id<T>() 
                     << ", real type is " << it->second->getTypeName() << ", value = " << it->second->toString();
                    return nullptr;
                }
            }
            if (name.find_first_not_of("abcdefghijklmnopqrstuivwxyz._012345678") != std::string::npos) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);      
            } 
            auto v = std::make_shared<ConfigVar<T>>(name, default_val, description);
            GetDatas()[name] = v;
            return v;
        }
        static void LoadFromYaml(const YAML::Node& root);
        static ptr<ConfigVarBase> LookupBase(const std::string& name);
        static void Visit(std::function<void(ptr<ConfigVarBase>)> cb);
    private:
        static ConfigVarMap& GetDatas() {
            static ConfigVarMap s_datas;
            return s_datas;
        }
        static RWmutexType& GetMutex() {
            static RWmutexType m_mutex;
            return m_mutex;
        }
    };
}  // namespace sylar