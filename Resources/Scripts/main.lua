-- main.lua
package.path = package.path .. App:GetResourcesDir() .. "/Scripts/?.lua;"
require "AppSettings"



local mySettings = {
    sUserName = "user name",
    sPassWord = ""
}

local bShowPassword = false
local function draw_demo_settings()
    mySettings.sUserName = ImGui.InputText("User Name",mySettings.sUserName)
    local passFlags = ImGuiInputTextFlags.Password
    if bShowPassword then
        passFlags = 0
    end

    mySettings.sPassWord = ImGui.InputText("Password",mySettings.sPassWord,passFlags)
    ImGui.SameLine()
    bShowPassword = ImGui.Checkbox("Show",bShowPassword)
end

---@param Ar json 
local function save_demo_settigns(Ar)
    local MyAr = Ar.demo_settings
    MyAr.sUserName = mySettings.sUserName
    MyAr.sPassWord = mySettings.sPassWord
end

---@param Ar json
local function load_demo_settigns(Ar)
    if not Ar:contains("demo_settings")then
        return false
    end
    ---@type json
     local MyAr = Ar.demo_settings
        mySettings.sUserName = MyAr.sUserName:as_string()
        mySettings.sPassWord = MyAr.sPassWord:as_string()
    return true
end

AppSettings:Register("Test","demo settings",draw_demo_settings)
App.OnAppSave:Bind(1001,save_demo_settigns)
App.OnAppLoad:Bind(1001,load_demo_settigns)

App.OnAppTick:Bind(1001,function (val)
    log.info(val)
end)