#pragma once
#include "const.h"
struct SectionInfo{
	SectionInfo() {}
	~SectionInfo() { _section_datas.clear(); }

	SectionInfo(const SectionInfo& other) {
		_section_datas = other._section_datas;
	}

	SectionInfo& operator=(const SectionInfo& other) {
		if (this == &other) {
			return *this;
		}
		this->_section_datas = other._section_datas;
		return *this;
	}

	std::map<std::string, std::string> _section_datas;
	std::string operator[](const std::string&key) {
		if(_section_datas.find(key) == _section_datas.end()) {
			return "";
		}
		return _section_datas[key];
	}

	std::string GetValue(const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			return "";
		}
		// 这里可以添加一些边界检查  
		return _section_datas[key];
	}
};
class ConfigMgr
{
public:

	~ConfigMgr() {
		_config_map.clear();
	}

	static ConfigMgr& Inst() {
		static ConfigMgr cfg_mgr;
		return cfg_mgr;
	}

	SectionInfo operator[](const std::string& section) {
		if(_config_map.find(section) == _config_map.end()) {
			return SectionInfo();
		}
		return _config_map[section];
	}

	ConfigMgr(const ConfigMgr& other) {
		_config_map = other._config_map;
	}

	ConfigMgr& operator=(const ConfigMgr& other) {
		if (this == &other) {
			return *this;
		}
		this->_config_map = other._config_map;
	}
	std::string GetValue(const std::string& section, const std::string& key);

private:
	ConfigMgr();
	std::map<std::string, SectionInfo> _config_map;
};

