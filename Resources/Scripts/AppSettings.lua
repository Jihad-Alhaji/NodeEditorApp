---@class AppSettings : ScriptModule
---@field bOpen boolean
---@field Log log.Logger
AppSettings = ScriptModule.new("AppSettings")
AppSettings.Log = log.NewLogger(AppSettings.Name)
AppSettings.bOpen = false

-- Registry: category -> list of setting groups
AppSettings.registry = {}
AppSettings.selectedCategory = nil

--- Register a new settings group
---@param category string
---@param name string
---@param drawer fun() function to draw the settings UI
function AppSettings:Register(category, name, drawer)
    if not self.registry[category] then
        self.registry[category] = {}
    end
    table.insert(self.registry[category], { name = name, drawer = drawer })
    AppSettings.Log:info("registered: ",category .. "." .. name)
end

--- Unregister a settings group
---@param category string
---@param name string
function AppSettings:Unregister(category, name)
    local cat = self.registry[category]
    if not cat then return end
    for i, entry in ipairs(cat) do
        if entry.name == name then
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
            if ImGui.CollapsingHeader(entry.name, ImGuiTreeNodeFlags.DefaultOpen) then
                entry.drawer()
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

App:AttachModule(AppSettings)
return AppSettings
