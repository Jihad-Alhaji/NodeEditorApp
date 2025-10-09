#pragma once
#include<sol/sol.hpp>
// Base class for anything scriptable
class ScriptableBase {
protected:
	sol::table lua_self;

public:
	virtual ~ScriptableBase() = default;

	sol::table& GetLuaSelf() { return lua_self; }
	void SetLuaSelf(const sol::table& t) { lua_self = t; }


	bool HasLuaFunction(const std::string& name) const {
		return lua_self.valid() && lua_self[name].valid() && lua_self[name].get_type() == sol::type::function;
	}

	template <typename... Args>
	bool CallIfExists(const std::string& name, Args... args) const {
		if (HasLuaFunction(name)) {
			sol::function f = lua_self[name];
			f(lua_self, std::forward<Args>(args)...);
			return true;
		}
		return false;
	}

	template <typename Ret, typename... Args>
	sol::optional<Ret> CallWithReturnIfExists(const std::string& name, Args... args) const {
		if (HasLuaFunction(name)) {
			sol::protected_function_result result = lua_self[name](lua_self, std::forward<Args>(args)...);
			if (result.valid()) return result;
		}
		return sol::nullopt;
	}
};

template<class Scriptable_t>
sol::object Scriptable_Lua_Index(Scriptable_t& self, const std::string& key, sol::this_state s)
{
	static_assert(std::is_base_of_v<ScriptableBase, Scriptable_t>, "only classes that inherit from ScriptableBase are allowed");
	sol::state_view L(s);

	// Check dynamic lua_state first
	if (self.GetLuaSelf().valid()) {
		sol::object val = self.GetLuaSelf().raw_get<sol::object>(key);
		if (val.valid()) return val;
	}

	// Then check C++ usertype
	sol::object cpp_obj = sol::make_object(L, &self);
	return cpp_obj.as<sol::table>()[key];
}

template<class Scriptable_t>
void Scriptable_Lua_NewIndex(Scriptable_t& self, const std::string& key, sol::object value)
{
	static_assert(std::is_base_of_v<ScriptableBase, Scriptable_t>, "only classes that inherit from ScriptableBase are allowed");
	
	if (self.GetLuaSelf().valid()) {
		self.GetLuaSelf().raw_set(key, value);
	}
}

