---@class AppSettings : ScriptModule
---@field bOpen boolean
---@field Log log.Logger
local AppSettings = ScriptModule.new("AppSettings")
AppSettings.Log = log.NewLogger(AppSettings.Name)
AppSettings.bOpen = false


---@class SettingEntry
---@field Name string
---@field Category string
---@field Save fun(self: SettingEntry, Ar: json)
---@field Load fun(self: SettingEntry, Ar: json)
---@field Draw fun(self: SettingEntry)
local SettingEntry = {}
SettingEntry.__index = SettingEntry

---@param name string
---@param category string
---@return SettingEntry
function SettingEntry:new(name, category)
    return setmetatable({
        Name = name or "",
        Category = category or "",
        Draw = function ()  end,
        Load = function (Ar)  end,
        Save = function (Ar)  end
    }, self)
end

-- Registry: category -> list of setting groups
AppSettings.registry = {}
AppSettings.selectedCategory = nil

--- Register a new settings group
---@param entry SettingEntry
function AppSettings:Register(entry)
    if not self.registry[entry.Category] then
        self.registry[entry.Category] = {}
    end
    table.insert(self.registry[entry.Category], entry)
    AppSettings.Log:info("registered: ",entry.Category .. "." .. entry.Name)
end

--- Unregister a settings group
---@param category string
---@param name string
function AppSettings:Unregister(category, name)
    local cat = self.registry[category]
    if not cat then return end
    for i, entry in ipairs(cat) do
        if entry.Name == name then
            table.remove(cat, i)
            break
        end
    end
    if #cat == 0 then
        self.registry[category] = nil
        if self.selectedCategory == category then
            self.selectedCategory = nil
        end
    end
end

--- Draw Settings Window 
function AppSettings:Draw(DeltaTime)
    if not self.bOpen then return end

    ImGui.SetNextWindowSize(720, 480, ImGuiCond.FirstUseEver)
    self.bOpen = ImGui.Begin("Settings", self.bOpen, ImGuiWindowFlags.MenuBar)
    if not self.bOpen then return end

    -- Split UI into two columns: categories (left), settings (right)
    ImGui.Columns(2, "settings_columns", true)

    -- Left: Categories
    for category, _ in pairs(self.registry) do
        local isSelected = (self.selectedCategory == category)
        if ImGui.Selectable(category, isSelected) then
            self.selectedCategory = category
        end
    end

    ImGui.NextColumn()

    -- Right: Settings for selected category
    if self.selectedCategory and self.registry[self.selectedCategory] then
        for _, entry in ipairs(self.registry[self.selectedCategory]) do
            if ImGui.CollapsingHeader(entry.Name, ImGuiTreeNodeFlags.DefaultOpen) then
                entry:Draw()
            end
        end
    else
        ImGui.Text("...")
    end

    ImGui.Columns(1)
    ImGui.End()
end

function AppSettings:BuildMenus()
    if ImGui.BeginMenu("Edit") then
        if ImGui.MenuItem("Settings") then
            self.bOpen = not self.bOpen
        end
        ImGui.EndMenu()
    end
end



---@param Ar json
function AppSettings:Save(Ar)
    for cat, entries in pairs(self.registry) do
        for _, entry in ipairs(entries) do
            self.Log:warn("Saving: ", entry.Category, "->", entry.Name)
            entry:Save(Ar[cat][entry.Name]);
        end
    end
end


---@param Ar json
function AppSettings:Load(Ar)
     for cat, entries in pairs(self.registry) do
        for _, entry in ipairs(entries) do
             self.Log:warn("Loading: ", entry.Category, "->", entry.Name)
            entry:Load(Ar[cat][entry.Name]);
        end
    end
    return true;
end

App:AttachModule(AppSettings)

local m = {}
m.AppSettings = AppSettings
m.SettingEntry = SettingEntry

return m
