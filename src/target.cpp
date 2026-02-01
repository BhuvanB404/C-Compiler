#include "target.h"
#include "generator.h"
#include "codegen/x86_64_generator.h"
#include "codegen/arm_generator.h"
#include "codegen/wasm_generator.h"
#include "codegen/wasmedge_generator.h"
#include <iostream>

using namespace std;


TargetRegistry& TargetRegistry::instance()
{
    static TargetRegistry registry;
    return registry;
}

void TargetRegistry::register_target(string name, unique_ptr<TargetAPI> api)
{
    m_targets[name] = move(api);
}

TargetAPI* TargetRegistry::get_target(const string& name)
{
    if (!m_initialized) {
        register_all_targets();
        m_initialized = true;
    }
    
    auto it = m_targets.find(name);
    if (it != m_targets.end()) {
        return it->second.get();
    }
    return nullptr;
}

vector<string> TargetRegistry::list_targets() const
{
    // Ensure targets are initialized
    if (!m_initialized) {
        const_cast<TargetRegistry*>(this)->register_all_targets();
        const_cast<TargetRegistry*>(this)->m_initialized = true;
    }
    
    vector<string> names;
    for (const auto& pair : m_targets) {
        names.push_back(pair.first);
    }
    return names;
}

// Forward declarations for target implementations
unique_ptr<TargetAPI> create_x86_64_target();
unique_ptr<TargetAPI> create_arm_target();
unique_ptr<TargetAPI> create_wasm_target();
unique_ptr<TargetAPI> create_wasmedge_target();

void TargetRegistry::register_all_targets()
{
    register_target("x86_64", create_x86_64_target());
    register_target("x86_64-linux", create_x86_64_target());
    register_target("aarch64", create_arm_target());
    register_target("aarch64-linux", create_arm_target());
    register_target("arm64", create_arm_target());
    register_target("wasm", create_wasm_target());
    register_target("wasm32", create_wasm_target());
    register_target("wasmtime", create_wasm_target());
    register_target("wasmedge", create_wasmedge_target());
}