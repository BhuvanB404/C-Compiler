#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "main.h"

using namespace std;

// Abstract base class for code generation targets
class TargetAPI
{
public:
    virtual ~TargetAPI() = default;
    
    
    virtual string gcode(const vector<inst>& ir) = 0;
    
    
    virtual string asm_ext() const = 0;
    
    
    virtual string asm_cmd(const string& asm_file, const string& obj_file) const = 0;
    
    
    virtual string ld_cmd(const string& obj_file, const string& exe_file) const = 0;
    
    
    virtual string name() const = 0;
    
    
    virtual bool avail() const = 0;
};


struct Target
{
    string codegen_name;
    unique_ptr<TargetAPI> api;
    
    Target(string name, unique_ptr<TargetAPI> target_api)
        : codegen_name(move(name)), api(move(target_api))
    {
    }
};

// Target registry class
class TargetRegistry
{
public:
    static TargetRegistry& instance();
    
    void register_target(string name, unique_ptr<TargetAPI> api);
    TargetAPI* get_target(const string& name);
    vector<string> list_targets() const;
    void register_all_targets();
    
private:
    unordered_map<string, unique_ptr<TargetAPI>> m_targets;
    bool m_initialized = false;
};