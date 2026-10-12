local H = { rows = {}, pins = {}, fills = {}, queue = {}, page = 1, merge = {} }
XPHotspots = H
local frame = CreateFrame("Frame")
local selected, draft, hovered, panel, enabled, gm, announcements
local clockOffset = 0
local chunks, chunkNext, chunkCount
local snapshot, incoming
local function N(v) return tonumber(v) or 0 end
local function B(v) return v == true or v == "true" or v == 1 or v == "1" end
local function Notify(text) DEFAULT_CHAT_FRAME:AddMessage("|cffb080ffHotspots:|r " .. text) end
local function Send(text)
    if #text > 240 then Notify("Request too long."); return end
    if #H.queue >= 64 then Notify("Please wait for pending hotspot requests.");return end
    H.queue[#H.queue + 1] = text
end
local function Now() return time() + clockOffset end
local function View()
    local area = GetCurrentMapAreaID()
    if not C_WorldMap or not area then return end
    local map = C_WorldMap.GetMapIDByAreaID(area)
    local zone = C_WorldMap.GetZoneIDByAreaID(area) or 0
    local top, left = C_WorldMap.GetWorldPosition(area, 0, 0)
    local bottom, right = C_WorldMap.GetWorldPosition(area, 1, 1)
    if not map or not top or top == bottom or left == right then return end
    return map, zone, top, bottom, left, right
end
local function Position(x, y)
    local map, zone, top, bottom, left, right = View()
    if not map then return end
    return (y - left) / (right - left), (x - top) / (bottom - top)
end
local function AtCursor()
    local map, zone, top, bottom, left, right = View()
    if not map then return end
    local x, y = GetCursorPosition()
    local scale = WorldMapButton:GetEffectiveScale()
    x = (x / scale - WorldMapButton:GetLeft()) / WorldMapButton:GetWidth()
    y = (WorldMapButton:GetTop() - y / scale) / WorldMapButton:GetHeight()
    return top + y * (bottom-top), left + x * (right-left)
end
local function Inside(r, x, y)
    if r.kind == "union" then
        for _,part in ipairs(type(r.parts)=="table" and r.parts or {}) do
            if Inside(part,x,y) then return true end
        end
        return false
    end
    if r.kind == "circle" then return (x-N(r.x))^2 + (y-N(r.y))^2 <= N(r.radius)^2 end
    if r.kind == "zone" then
        if type(r.mask)~="table" or type(r.bounds)~="table" then return false end
        local top,bottom,left,right=N(r.bounds[1]),N(r.bounds[2]),N(r.bounds[3]),N(r.bounds[4])
        if top==bottom or left==right then return false end
        local row=math.floor((x-top)/(bottom-top)*64)+1
        local col=math.floor((y-left)/(right-left)*64)+1
        return row>=1 and row<=64 and col>=1 and col<=64 and
            type(r.mask[row])=="string" and r.mask[row]:sub(col,col)=="1"
    end
    local points = type(r.points) == "table" and r.points or {}
    local inside, j = false, #points
    for i=1,#points do
        local a,b=points[i],points[j]
        local ax,ay,bx,by=N(a.x),N(a.y),N(b.x),N(b.y)
        if (ay>y) ~= (by>y) and x < (bx-ax)*(y-ay)/(by-ay)+ax then inside=not inside end
        j=i
    end
    return inside
end
function H.Refresh(page)
    if not enabled or not WorldMapFrame:IsShown() then return end
    if H.refreshPending then
        H.refreshAgain=true
        if type(page)=="number" then H.nextPage=page end
        return
    end
    local map, zone = View()
    if not map then return end
    H.page=type(page)=="number" and math.max(1,page) or H.page or 1
    H.rows={}
    H.refreshPending=true
    H.refreshStarted=GetTime()
    H.viewMap,H.viewZone=map,zone
    if gm and H.catalog and H.allMaps then map,zone=4294967295,0 end
    Send((gm and H.catalog and "list " or "sync ") .. map .. " " .. zone .. " " .. H.page .. " " .. (H.search or ""))
end
local function Tooltip(r, owner)
    WorldMapTooltip:SetOwner(owner,"ANCHOR_RIGHT")
    WorldMapTooltip:ClearLines()
    WorldMapTooltip:AddLine(r.name or "Hotspot",1,1,1)
    WorldMapTooltip:AddLine("+" .. N(r.xp) .. "% monster-kill experience",0.7,1,0.4)
    WorldMapTooltip:AddLine("Any eligible creature kill while you are inside this area.",0.7,0.8,1,true)
    local remaining=math.max(0,N(r.ends)-Now())
    WorldMapTooltip:AddLine(remaining>0 and SecondsToTime(remaining) .. " remaining" or "Inactive",1,0.82,0)
    if r.kind == "zone" then WorldMapTooltip:AddLine("Entire zone",0.7,0.8,1) end
    if gm and r.geometrySource=="approximate-spawn-circle" then
        WorldMapTooltip:AddLine("Approximate spawn area; no usable quest polygon recovered.",1,0.7,0.3,true)
    end
    if B(r.outlineUnavailable) then WorldMapTooltip:AddLine("Zone boundary preview unavailable",1,0.7,0.3,true) end
    if gm then
        WorldMapTooltip:AddLine("ID " .. N(r.id) .. " | " .. (B(r.enabled) and "Enabled" or "Disabled"),0.8,0.8,0.8)
        if not B(r.allowed) and not B(r.draft) then
            WorldMapTooltip:AddLine("Unavailable: map restriction or unresolved geometry",1,0.4,0.4,true)
        end
        WorldMapTooltip:AddLine("Click to select and manage",0.7,0.8,1)
    end
    WorldMapTooltip:Show()
end
function H.Highlight(r)
    for _,t in ipairs(H.fills) do t:Hide() end
    if not r or not B(r.highlight) then return end
    local map, zone, top, bottom, left, right=View()
    if map~=N(r.map) then return end
    local w,h=WorldMapButton:GetWidth(),WorldMapButton:GetHeight()
    local count=0
    local function Fill(row,first,last)
        first,last=math.max(0,first),math.min(1,last)
        if last<=first then return end
        count=count+1
        local t=H.fills[count]
        if not t then t=WorldMapButton:CreateTexture(nil,"OVERLAY");H.fills[count]=t end
        t:SetTexture(0.35,0.45,1,0.25)
        t:ClearAllPoints()
        t:SetPoint("TOPLEFT",WorldMapButton,"TOPLEFT",first*w,-row*h/66)
        t:SetSize((last-first)*w,h/66+0.5);t:Show()
    end
    local function Interval(row,a,b)
        a,b=(a-left)/(right-left),(b-left)/(right-left)
        Fill(row,math.min(a,b),math.max(a,b))
    end
    local points=type(r.points)=="table" and r.points or {}
    for row=0,65 do
        local x=top+(row+0.5)/66*(bottom-top)
        if r.kind=="circle" then
            local square=N(r.radius)^2-(x-N(r.x))^2
            if square>0 then
                local half=math.sqrt(square);Interval(row,N(r.y)-half,N(r.y)+half)
            end
        elseif r.kind=="polygon" then
            local hits={}
            for i=1,#points do
                local a,b=points[i],points[i%#points+1]
                local ax,bx=N(a.x),N(b.x)
                if (ax>x)~=(bx>x) then hits[#hits+1]=N(a.y)+(x-ax)*(N(b.y)-N(a.y))/(bx-ax) end
            end
            table.sort(hits)
            for i=1,#hits-1,2 do Interval(row,hits[i],hits[i+1]) end
        else
            local first
            for col=0,100 do
                local hit=col<100 and Inside(r,x,left+(col+0.5)/100*(right-left))
                if hit and not first then first=col end
                if first and not hit then Fill(row,first/100,col/100);first=nil end
            end
        end
    end
end
function H.Draw()
    for _,p in ipairs(H.pins) do p:Hide() end
    for _,t in ipairs(H.fills) do t:Hide() end
    if not enabled or not WorldMapFrame:IsShown() then return end
    local map,zone=View()
    if not map then return end
    local rows={}
    for _,r in pairs(H.rows) do
        if not draft or N(draft.id)~=N(r.id) then rows[#rows+1]=r end
    end
    if draft then rows[#rows+1]=draft end
    local count=0
    for _,r in ipairs(rows) do
        if N(r.map)==map and (B(r.icon) or gm and H.catalog or B(r.draft)) then
            local x,y=Position(N(r.x),N(r.y))
            if x and x>=0 and x<=1 and y>=0 and y<=1 then
                count=count+1
                local p=H.pins[count]
                if not p then
                    p=CreateFrame("Button",nil,WorldMapButton)
                    p:SetSize(24,24)
                    p:SetFrameLevel(WorldMapButton:GetFrameLevel()+8)
                    p:SetNormalAtlas("hotspot-xp")
                    p:RegisterForClicks("LeftButtonUp","RightButtonUp")
                    p:SetScript("OnEnter",function(self)
                        hovered=self.row;H.hoverOwner=self;Tooltip(self.row,self);H.Highlight(self.row)
                        if self.row.kind=="zone" and not self.row.mask and not self.row.outlineRequested then
                            self.row.outlineRequested=true;Send("outline "..N(self.row.id))
                        end
                    end)
                    p:SetScript("OnLeave",function()
                        hovered=nil;H.hoverOwner=nil;WorldMapTooltip:Hide();H.Highlight(selected or draft)
                    end)
                    p:SetScript("OnClick",function(self)
                        H.Select(self.row)
                    end)
                    H.pins[count]=p
                end
                p.row=r
                local chosen=selected and N(selected.id)==N(r.id)
                p:SetSize(chosen and 32 or 24,chosen and 32 or 24)
                if not p.selection then
                    p.selection=p:CreateTexture(nil,"OVERLAY")
                    p.selection:SetAllPoints()
                    p.selection:SetTexture("Interface\\Buttons\\UI-ActionButton-Border")
                    p.selection:SetBlendMode("ADD")
                    p.selection:SetVertexColor(1,0.8,0.2,1)
                end
                if chosen then p.selection:Show() else p.selection:Hide() end
                p:ClearAllPoints()
                p:SetPoint("CENTER",WorldMapButton,"TOPLEFT",x*WorldMapButton:GetWidth(),-y*WorldMapButton:GetHeight())
                p:SetAlpha(N(r.ends)>Now() and 1 or 0.5)
                p:Show()
            end
        end
    end
    H.Highlight(hovered or selected or draft)
    if H.DrawBrowser then H.DrawBrowser() end
end
local function Button(parent,text,x,y,width,click)
    local b=CreateFrame("Button",nil,parent,"UIPanelButtonTemplate")
    b:SetSize(width or 130,22);b:SetPoint("TOPLEFT",x,y);b:SetText(text);b:SetScript("OnClick",click)
    b:SetBackdrop({bgFile="Interface\\Buttons\\WHITE8X8",
        edgeFile="Interface\\Buttons\\WHITE8X8",edgeSize=1})
    b:SetBackdropColor(0.03,0.11,0.17,1);b:SetBackdropBorderColor(0.78,0.60,0.27,1)
    b:SetNormalTexture("");b:SetPushedTexture("")
    local glow=b:CreateTexture(nil,"HIGHLIGHT");glow:SetAllPoints();glow:SetTexture(0.90,0.70,0.30,0.16)
    b:SetHighlightTexture(glow)
    b:SetNormalFontObject("GameFontNormalSmall");b:SetHighlightFontObject("GameFontHighlightSmall")
    return b
end
local fields={}
local browser
function H.DrawBrowser()
    if not browser then return end
    if not enabled or not gm or not H.catalog then browser:Hide();return end
    local rows={}
    for _,r in pairs(H.rows) do rows[#rows+1]=r end
    table.sort(rows,function(a,b) return N(a.id)<N(b.id) end)
    for i,b in ipairs(browser.rows) do
        local r=rows[i]
        b.row=r
        if r then
            b:SetText((N(r.ends)>Now() and "|cff80ff80" or B(r.enabled) and "|cffffffff" or "|cff888888") ..
                (selected and N(selected.id)==N(r.id) and "[SELECTED] " or "")..
                (H.merge[N(r.id)] and "[+] " or "")..N(r.id).." "..(r.name or "").."|r")
            b:Show()
        else b:Hide() end
    end
    browser.page:SetText("Page "..(H.page or 1).." / "..(H.pages or 1))
    browser:Show()
end
local function ShowBrowser()
    if not browser then
        browser=CreateFrame("Frame",nil,WorldMapFrame)
        browser:SetSize(260,469);browser:SetPoint("TOPLEFT",WorldMapFrame,"TOPLEFT",12,-65)
        browser:SetFrameStrata("DIALOG");browser:EnableMouse(true)
        browser:SetBackdrop({bgFile="Interface\\Buttons\\WHITE8X8",
            edgeFile="Interface\\Tooltips\\UI-Tooltip-Border",edgeSize=12})
        browser:SetBackdropColor(0.03,0.11,0.17,0.97)
        browser:SetBackdropBorderColor(0.78,0.60,0.27,1)
        local title=browser:CreateFontString(nil,"OVERLAY","GameFontNormal")
        title:SetPoint("TOPLEFT",12,-12);title:SetText("XP Hotspots")
        title:SetTextColor(0.95,0.76,0.39)
        browser.title=title
        local search=CreateFrame("EditBox",nil,browser)
        search:SetFontObject("GameFontHighlightSmall")
        search:SetTextInsets(6,6,2,2)
        search:SetBackdrop({bgFile="Interface\\Buttons\\WHITE8X8",
            edgeFile="Interface\\Buttons\\WHITE8X8",edgeSize=1})
        search:SetBackdropColor(0.02,0.03,0.06,1);search:SetBackdropBorderColor(0.65,0.53,0.3,1)
        search:SetPoint("TOPLEFT",12,-36);search:SetSize(190,20);search:SetAutoFocus(false);search:SetMaxLetters(100)
        H.searchBox=search
        local function Search() H.searchAt=nil;H.search=search:GetText();H.Refresh(1) end
        search:SetScript("OnTextChanged",function(self,user)
            if user then H.searchAt=GetTime()+0.4 end
        end)
        search:SetScript("OnEscapePressed",function(self) self:ClearFocus() end)
        search:SetScript("OnEnterPressed",function(self) self:ClearFocus();Search() end)
        Button(browser,"Find",205,-34,45,Search)
        browser.rows={}
        for i=1,12 do
            local b=Button(browser,"",10,-64-(i-1)*27,240,function(self)
                if IsShiftKeyDown() and self.row and self.row.kind~="zone" then
                    local id=N(self.row.id)
                    H.merge[id]=not H.merge[id] and N(self.row.revision) or nil
                    H.DrawBrowser();return
                end
                H.Select(self.row)
            end)
            b:SetScript("OnEnter",function(self)
                if not self.row then return end
                hovered=self.row;H.hoverOwner=self;Tooltip(self.row,self);H.Highlight(self.row)
                if self.row.kind=="zone" and not self.row.mask then Send("outline "..N(self.row.id)) end
            end)
            b:SetScript("OnLeave",function()
                hovered=nil;H.hoverOwner=nil;WorldMapTooltip:Hide();H.Highlight(selected or draft)
            end)
            browser.rows[i]=b
        end
        Button(browser,"Previous",10,-393,100,function() H.Refresh(math.max(1,(H.page or 1)-1)) end)
        Button(browser,"Next",150,-393,100,function() H.Refresh(math.min(H.pages or 1,(H.page or 1)+1)) end)
        browser.page=browser:CreateFontString(nil,"OVERLAY","GameFontNormalSmall")
        browser.page:SetPoint("TOP",0,-424)
        Button(browser,"This map / all locations",10,-440,240,function() H.allMaps=not H.allMaps;H.Refresh(1) end)
    end
    H.DrawBrowser()
end
local function Field(label,y,default)
    local text=panel:CreateFontString(nil,"OVERLAY","GameFontNormalSmall")
    text:SetPoint("TOPLEFT",10,y);text:SetText(label)
    local edit=CreateFrame("EditBox",nil,panel)
    edit:SetFontObject("GameFontHighlightSmall");edit:SetTextInsets(5,5,2,2)
    edit:SetBackdrop({bgFile="Interface\\Buttons\\WHITE8X8",
        edgeFile="Interface\\Buttons\\WHITE8X8",edgeSize=1})
    edit:SetBackdropColor(0.02,0.06,0.10,1);edit:SetBackdropBorderColor(0.78,0.60,0.27,1)
    edit:SetSize(160,20);edit:SetPoint("TOPLEFT",115,y+3);edit:SetAutoFocus(false)
    edit:SetText(default);edit:SetMaxLetters(100);fields[label]=edit
    return edit
end
local function SelectedCommand(action)
    if not selected then Notify("Select a hotspot marker first.");return end
    Send(action .. " " .. N(selected.id) .. " " .. N(selected.revision))
end
local function ApplyFields()
    if not draft or selected and N(selected.id)~=N(draft.id) then Notify("Click Edit selected first.");return false end
    Send("name "..fields.Name:GetText())
    Send("settings "..fields["XP bonus %"]:GetText().." "..N(fields.Minutes:GetText())*60 .." "..
        fields.Weight:GetText().." "..N(fields["Cooldown min"]:GetText())*60 .." "..
        (fields.icon:GetChecked() and "1" or "0").." "..(fields.fill:GetChecked() and "1" or "0"))
    if draft.kind=="circle" then Send("radius "..fields["Radius yards"]:GetText()) end
    return true
end
function H.Select(row)
    selected=row
    if gm then H.ShowPanel() end
    H.Draw()
end
function H.ShowPanel()
    if not gm then return end
    if not panel then
        panel=CreateFrame("Frame",nil,WorldMapFrame)
        panel:SetSize(300,530);panel:SetPoint("TOPRIGHT",WorldMapFrame,"TOPRIGHT",-12,-55)
        panel:SetFrameStrata("DIALOG");panel:EnableMouse(true)
        panel:SetBackdrop({bgFile="Interface\\Buttons\\WHITE8X8",
            edgeFile="Interface\\Tooltips\\UI-Tooltip-Border",edgeSize=12})
        panel:SetBackdropColor(0.03,0.11,0.17,0.97)
        panel:SetBackdropBorderColor(0.78,0.60,0.27,1)
        local title=panel:CreateFontString(nil,"OVERLAY","GameFontNormal")
        title:SetPoint("TOPLEFT",10,-12);title:SetText("XP Hotspot Manager")
        title:SetTextColor(0.95,0.76,0.39)
        panel.title=title
        Button(panel,"X",265,-5,25,function() panel:Hide();if browser then browser:Hide() end end)
        Field("Name",-45,"New Hotspot")
        Field("XP bonus %",-75,"150")
        Field("Minutes",-105,"60")
        Field("Weight",-135,"1")
        Field("Cooldown min",-165,"60")
        Field("Radius yards",-195,"150")
        local icon=CreateFrame("CheckButton",nil,panel,"UICheckButtonTemplate")
        icon:SetPoint("TOPLEFT",10,-445);icon:SetSize(24,24);icon:SetChecked(true)
        local iconText=panel:CreateFontString(nil,"OVERLAY","GameFontNormalSmall")
        iconText:SetPoint("LEFT",icon,"RIGHT",0,0);iconText:SetText("Show XP icon")
        local fill=CreateFrame("CheckButton",nil,panel,"UICheckButtonTemplate")
        fill:SetPoint("TOPLEFT",155,-445);fill:SetSize(24,24);fill:SetChecked(true)
        local fillText=panel:CreateFontString(nil,"OVERLAY","GameFontNormalSmall")
        fillText:SetPoint("LEFT",fill,"RIGHT",0,0);fillText:SetText("Show highlight")
        fields.icon,fields.fill=icon,fill
        Button(panel,"New circle",10,-225,90,function()
            H.newKind="circle";Notify("CTRL-click the map to place the center.")
        end)
        Button(panel,"Whole zone",105,-225,90,function()
            H.newKind="zone";Notify("CTRL-click the zone map to create its hotspot.")
        end)
        Button(panel,"Polygon",200,-225,90,function()
            H.newKind="polygon";Notify("CTRL-click the map to start the boundary, then add vertices.")
        end)
        Button(panel,"Edit selected",10,-255,130,function()
            H.vertex=nil;H.newKind=nil
            if selected then Send("edit "..N(selected.id)) end
        end)
        Button(panel,"Merge selection",155,-255,130,function()
            local ids={};for id in pairs(H.merge) do ids[#ids+1]=id end;table.sort(ids)
            if #ids<2 or #ids>8 then Notify("SHIFT-click 2 to 8 non-zone browser rows to merge.");return end
            local command="merge"
            for _,id in ipairs(ids) do command=command.." "..id.." "..H.merge[id] end
            Send(command);H.newKind=nil;H.vertex=nil
        end)
        Button(panel,"Apply settings",10,-285,130,ApplyFields)
        Button(panel,"Save draft",155,-285,130,function()
            if not draft or selected and N(selected.id)~=N(draft.id) then Notify("Click Edit selected first.");return end
            Send("save "..fields["XP bonus %"]:GetText().." "..N(fields.Minutes:GetText())*60 .." "..
                fields.Weight:GetText().." "..N(fields["Cooldown min"]:GetText())*60 .." "..
                (fields.icon:GetChecked() and "1" or "0").." "..(fields.fill:GetChecked() and "1" or "0")..
                " "..fields["Radius yards"]:GetText().." "..fields.Name:GetText())
        end)
        Button(panel,"Enable selected",10,-315,130,function() SelectedCommand("enable") end)
        Button(panel,"Disable selected",155,-315,130,function() SelectedCommand("disable") end)
        Button(panel,"Move to next",10,-345,130,function() SelectedCommand("next") end)
        Button(panel,"Stop selected",155,-345,130,function() SelectedCommand("stop") end)
        Button(panel,"Browse / active",10,-375,130,function()
            H.catalog=not H.catalog;ShowBrowser();H.Refresh(1)
        end)
        fields.announce=Button(panel,"Announcements",155,-375,130,function()
            Send("announce "..(announcements and "0" or "1"))
        end)
        Button(panel,"Cancel draft",10,-405,130,function()
            if selected then selected=H.rows[N(selected.id)] end
            draft=nil;H.newKind=nil;H.vertex=nil;H.merge={};Send("cancel");H.Draw();H.DrawBrowser()
        end)
        Button(panel,"Refresh",155,-405,130,H.Refresh)
        local hint=panel:CreateFontString(nil,"OVERLAY","GameFontHighlightSmall")
        hint:SetPoint("TOPLEFT",10,-488);hint:SetWidth(280)
        hint:SetText("CTRL-click map: place center/add vertex. ALT-click: select nearest vertex; "..
            "CTRL-click: move it. CTRL-right: undo. SHIFT-click browser rows to merge. Save, then Enable.")
    end
    local r=selected or draft
    panel.title:SetText(r and ("Selected Hotspot #"..N(r.id)) or "XP Hotspot Manager")
    if r then
        fields.Name:SetText(r.name or "")
        fields["XP bonus %"]:SetText(N(r.xp));fields.Minutes:SetText(N(r.duration)/60)
        fields.Weight:SetText(N(r.weight));fields["Cooldown min"]:SetText(N(r.cooldown)/60)
        fields["Radius yards"]:SetText(N(r.radius))
        fields.icon:SetChecked(B(r.icon));fields.fill:SetChecked(B(r.highlight))
    end
    panel:Show()
    fields.announce:SetText(announcements and "Announce: ON" or "Announce: OFF")
    ShowBrowser()
end
local function Receive(text)
    if text:sub(1,6)=="BRAND|" then return end
    if text=="OFF" then
        enabled=false;gm=false;H.rows={};H.queue={};H.refreshPending=false;H.refreshAgain=false
        selected=nil;draft=nil;hovered=nil;snapshot=nil;incoming=nil;chunks=nil
        if panel then panel:Hide() end
        if browser then browser:Hide() end
        WorldMapTooltip:Hide()
        H.Draw();return
    end
    if text:match("^CHANGED|") then enabled=true;H.Refresh();return end
    local g,isGM,a,now,page,pages=text:match("^B|(%d+)|(%d+)|(%d+)|(%d+)|(%d+)|(%d+)$")
    if g then
        enabled=true;gm=isGM=="1";announcements=a=="1";clockOffset=N(now)-time()
        if fields.announce then fields.announce:SetText(announcements and "Announce: ON" or "Announce: OFF") end
        snapshot={generation=N(g),page=N(page),pages=N(pages)};incoming={};return
    end
    local index,total,data=text:match("^R|(%d+)|(%d+)|(.*)$")
    if index then
        index,total=N(index),N(total)
        if total>256 or total<1 then return end
        if index==0 then chunks={};chunkNext=0;chunkCount=total end
        if not chunks or index~=chunkNext or total~=chunkCount then chunks=nil;return end
        chunks[#chunks+1]=data;chunkNext=chunkNext+1
        if chunkNext==chunkCount then
            local ok,r=pcall(C_Serialize.FromJSON,C_Serialize,table.concat(chunks));chunks=nil
            if ok and type(r)=="table" then
                if B(r.outline) then
                    if draft and N(draft.zone)==N(r.zone) then
                        draft.mask=r.mask;draft.bounds=r.bounds;draft.outlineUnavailable=r.outlineUnavailable
                    end
                    H.Draw()
                elseif B(r.draft) then
                    draft=r;selected=r
                    if r.kind=="union" and N(r.id)>0 then H.merge={} end
                    H.ShowPanel();H.Draw()
                elseif incoming then incoming[N(r.id)]=r end
            end
        end
        return
    end
    if text=="E" and snapshot then
        H.rows=incoming
        if selected and H.rows[N(selected.id)] then selected=H.rows[N(selected.id)] end
        if hovered and H.rows[N(hovered.id)] then hovered=H.rows[N(hovered.id)] end
        local s=snapshot;snapshot=nil;incoming=nil
        H.page,H.pages=s.page,s.pages
        H.Draw()
        H.refreshPending=false
        if H.refreshAgain then
            local nextPage=H.nextPage;H.nextPage=nil;H.refreshAgain=false;H.Refresh(nextPage)
        end
        return
    end
    if text:match("^ERROR|") or text:match("^OK|") then Notify(text:sub(text:find("|",1,true)+1)) end
end
frame:RegisterEvent("PLAYER_LOGIN");frame:RegisterEvent("CHAT_MSG_ADDON");frame:RegisterEvent("WORLD_MAP_UPDATE")
frame:SetScript("OnEvent",function(_,event,prefix,text,channel,sender)
    if event=="CHAT_MSG_ADDON" then
        if prefix=="XPHOT" and channel=="WHISPER" and sender==UnitName("player") then Receive(text) end
    elseif event=="PLAYER_LOGIN" then
        Send("sync")
        WorldMapFrame:HookScript("OnShow",H.Refresh)
        local filters=WorldMapButton.Filters
        if filters and filters.DropDown and not H.filterInstalled then
            local dropdown=filters.DropDown
            local initialize=dropdown.initialize
            if type(initialize)=="function" then
                UIDropDownMenu_Initialize(dropdown,function(self,level,menuList)
                    initialize(self,level,menuList)
                    if enabled and gm and filters:IsVisible() and (not level or level==1) then
                        local info=UIDropDownMenu_CreateInfo()
                        info.text="XP Hotspot Menu";info.notCheckable=true
                        info.func=function() if enabled and gm then H.ShowPanel() end end
                        UIDropDownMenu_AddButton(info,level)
                    end
                end,"MENU")
                H.filterInstalled=true
            end
        end
        local priorMouseUp=WorldMapButton:GetScript("OnMouseUp")
        WorldMapButton:SetScript("OnMouseUp",function(self,button,...)
            if gm and H.newKind and IsControlKeyDown() and button=="LeftButton" then
                local map,zone=View();local x,y=AtCursor()
                if not x or not zone or zone==0 then Notify("Open a zone map first.");return end
                local kind=H.newKind;H.newKind=nil;H.vertex=nil
                Send("newmap "..kind.." "..map.." "..zone.." "..x.." "..y.." "..fields.Name:GetText())
                if kind=="polygon" then Send("point "..x.." "..y)
                elseif kind=="circle" then Send("radius "..fields["Radius yards"]:GetText()) end
                return
            end
            if gm and draft and draft.kind=="polygon" and IsAltKeyDown() and button=="LeftButton" then
                local map=View();local x,y=AtCursor();local best
                if map~=N(draft.map) or not x then return end
                for i,p in ipairs(type(draft.points)=="table" and draft.points or {}) do
                    local distance=(x-N(p.x))^2+(y-N(p.y))^2
                    if not best or distance<best then H.vertex=i;best=distance end
                end
                if best then Notify("Vertex "..H.vertex.." selected. CTRL-click its new position.") end
                return
            end
            if gm and draft and IsControlKeyDown() and button=="RightButton" and draft.kind=="polygon" then
                H.vertex=nil;Send("undo");return
            end
            if gm and draft and IsControlKeyDown() and button=="LeftButton" then
                local map=View()
                if map~=N(draft.map) then
                    Notify("Open the map where this draft belongs.");return
                end
                local x,y=AtCursor()
                if x then
                    local command=H.vertex and "vertex "..H.vertex.." " or draft.kind=="polygon" and "point " or "center "
                    Send(command..x.." "..y);H.vertex=nil
                end
                return
            end
            if priorMouseUp then priorMouseUp(self,button,...) end
        end)
    elseif enabled then H.Refresh() end
end)
local elapsed=0
frame:SetScript("OnUpdate",function(_,diff)
    elapsed=elapsed+diff
    if elapsed<0.3 then return end
    elapsed=0
    if H.searchAt and GetTime()>=H.searchAt then
        H.searchAt=nil;H.search=H.searchBox:GetText();H.Refresh(1)
    end
    if hovered and H.hoverOwner and H.tooltipSecond~=Now() then
        H.tooltipSecond=Now();Tooltip(hovered,H.hoverOwner)
    end
    if H.refreshPending and GetTime()-(H.refreshStarted or GetTime())>15 then
        H.refreshPending=false;H.refreshAgain=false;H.Refresh()
    end
    if #H.queue>0 then SendAddonMessage("XPHOT",table.remove(H.queue,1),"WHISPER",UnitName("player")) end
end)
