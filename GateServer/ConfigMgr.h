#pragma once
#include "const.h"

struct SectionInfo{
    SectionInfo(){}

    // 拷贝构造
    SectionInfo(const SectionInfo& src){
        _section_datas = src._section_datas;
    }
    // 赋值构造
    SectionInfo& operator = (const SectionInfo& src){
        if(&src == this){
            return *this;
        }

        this->_section_datas = src._section_datas;
    }

    ~SectionInfo(){
        _section_datas.clear();
    }

    std::map<std::string, std::string> _section_datas;

    std::string operator[](const std::string& key){
        if(_section_datas.find(key) == _section_datas.end()){
            return "";
        }
        // 没进行边界检查
        return _section_datas[key];
    }

};

class ConfigMgr{
public:
    ~ConfigMgr(){
        _config_map.clear();
    }

    SectionInfo operator[](const std::string& section){
        if(_config_map.find(section) == _config_map.end()){
            return SectionInfo();
        }
        return _config_map[section];
    }

    ConfigMgr& operator=(const ConfigMgr& src) {
        if (&src == this) {
            return *this;
        }

        this->_config_map = src._config_map;
    };

    ConfigMgr(const ConfigMgr& src) {
        this->_config_map = src._config_map;
    }

    ConfigMgr();
private:
    // key 是section名称， value 是sectioninfo存的键值对
    std::map<std::string, SectionInfo> _config_map;
};
