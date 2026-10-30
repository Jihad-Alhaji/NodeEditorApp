-- main.lua
package.path = package.path .. App:GetResourcesDir() .. "/Scripts/?.lua;"
local Settings = require "AppSettings"

---@class mySettings : SettingEntry
local mySettings = Settings.SettingEntry:new("demo","General")
mySettings.bShowPassword = false
mySettings.sUserName = ""
mySettings.sPassWord = ""
 
function mySettings:Draw()
    mySettings.sUserName = ImGui.InputText("User Name",mySettings.sUserName)
    local passFlags = ImGuiInputTextFlags.Password
    if mySettings.bShowPassword then
        passFlags = 0
    end

    mySettings.sPassWord = ImGui.InputText("Password",mySettings.sPassWord,passFlags)
    ImGui.SameLine()
    mySettings.bShowPassword = ImGui.Checkbox("Show",mySettings.bShowPassword)
end

---@param Ar json 
function mySettings:Save (Ar)
    Ar.sUserName = mySettings.sUserName
    Ar.sPassWord = mySettings.sPassWord
end

---@param Ar json
function mySettings:Load(Ar)
        mySettings.sUserName = Ar.sUserName:as_string()
        mySettings.sPassWord = Ar.sPassWord:as_string()
end

Settings.AppSettings:Register(mySettings)


