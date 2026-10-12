import json
from pathlib import Path
import unittest
from lupa.lua51 import LuaRuntime


ROOT = Path(__file__).resolve().parents[1]
MOCK = r'''
frames={};messages={};clock=1000;oldClicks=0;control=false
StaticPopupDialogs={}
function StaticPopup_Show(which,name)
 popup={which=which,name=name};return popup
end
local methods={}
function methods:SetScript(k,v) self.scripts[k]=v end
function methods:GetScript(k) return self.scripts[k] end
function methods:HookScript(k,v)
 local old=self.scripts[k]
 self.scripts[k]=function(...) if old then old(...) end;v(...) end
end
function methods:SetText(t) self.text=tostring(t) end
function methods:GetText() return self.text or "" end
function methods:SetChecked(t) self.checked=t end
function methods:GetChecked() return self.checked end
function methods:Show() self.shown=true end
function methods:Hide() self.shown=false end
function methods:IsShown() return self.shown end
function methods:IsVisible() return self.shown end
function methods:GetEffectiveScale() return 1 end
function methods:GetLeft() return 0 end
function methods:GetTop() return 100 end
function methods:GetWidth() return self.width or 100 end
function methods:GetHeight() return self.height or 100 end
function methods:GetFrameLevel() return 1 end
function methods:SetSize(w,h) self.width=w;self.height=h end
function methods:SetPoint(...) self.point={...} end
function methods:CreateTexture() return CreateFrame("Texture") end
function methods:CreateFontString() return CreateFrame("Font") end
function CreateFrame(kind,name,parent,template)
 local f={scripts={},shown=true,kind=kind,parent=parent,width=100,height=100,text="",checked=false}
 setmetatable(f,{__index=function(_,k) return methods[k] or (k:match("^[A-Z]") and function() end) end})
 frames[#frames+1]=f;return f
end
WorldMapFrame=CreateFrame("Frame");WorldMapButton=CreateFrame("Button")
WorldMapButton:SetScript("OnMouseUp",function() oldClicks=oldClicks+1 end)
WorldMapTooltip=CreateFrame("Tooltip");GameTooltip=CreateFrame("Tooltip")
DEFAULT_CHAT_FRAME=CreateFrame("Chat")
menuEntries={}
WorldMapButton.Filters={IsVisible=function() return true end,DropDown={initialize=function(self,level,menuList)
 menuEntries[#menuEntries+1]={text=menuList or "Existing filters",level=level}
end}}
function UIDropDownMenu_Initialize(frame,initialize) frame.initialize=initialize end
function UIDropDownMenu_CreateInfo() return {} end
function UIDropDownMenu_AddButton(info,level) menuEntries[#menuEntries+1]=info end
function OpenFilters(level,menuList)
 menuEntries={};WorldMapButton.Filters.DropDown:initialize(level,menuList)
end
function GetCurrentMapAreaID() return 37 end
C_WorldMap={GetMapIDByAreaID=function() return 0 end,GetZoneIDByAreaID=function() return 44 end,
 GetWorldPosition=function(area,x,y) return 100-y*100,100-x*100 end}
C_Serialize={FromJSON=function(self,value) return testRow end}
function UnitName() return "Tester" end
function GetCursorPosition() return 25,75 end
function IsControlKeyDown() return control end
function IsShiftKeyDown() return shift end
function IsAltKeyDown() return alt end
function SendAddonMessage(...) messages[#messages+1]={...} end
function time() return clock end
function GetTime() return clock end
function SecondsToTime(s) return tostring(s) end
function ButtonNamed(text)
 for _,f in ipairs(frames) do if f.text==text then return f end end
end
'''


class HotspotClientTests(unittest.TestCase):
    def setUp(self):
        self.lua = LuaRuntime(unpack_returned_tuples=True)
        self.lua.execute(MOCK)
        self.lua.execute((ROOT / 'client/Interface/FrameXML/XPHotspots.lua').read_text())
        self.lua.execute('driver=frames[6];driver.scripts.OnEvent(driver,"PLAYER_LOGIN")')

    def receive(self, text, sender='Tester'):
        self.lua.globals().packet = text
        self.lua.globals().sender = sender
        self.lua.execute('driver.scripts.OnEvent(driver,"CHAT_MSG_ADDON","XPHOT",packet,"WHISPER",sender)')

    def row(self, **values):
        row = dict(id=1, revision=1, name='Redridge', kind='circle', map=0, zone=44, x=50, y=50,
                   radius=20, xp=150, duration=3600, weight=1, cooldown=3600, ends=4600,
                   enabled=True, allowed=True, icon=True, highlight=True)
        row.update(values)
        self.lua.globals().testRow = self.lua.table_from(row)
        self.receive('R|0|1|{}')

    def snapshot(self, gm=1, **values):
        self.receive(f'B|1|{gm}|1|1000|1|1')
        self.row(**values)
        self.receive('E')

    def test_server_snapshot_positions_and_hover_coverage(self):
        self.snapshot()
        self.lua.execute('p=XPHotspots.pins[1];p.scripts.OnEnter(p)')
        self.assertEqual(self.lua.eval('p.point[4]'), 50)
        self.assertEqual(self.lua.eval('p.point[5]'), -50)
        self.assertGreater(self.lua.eval('#XPHotspots.fills'), 0)

    def test_foreign_sender_cannot_enable_feature(self):
        self.receive('B|1|1|1|1000|1|1', 'AnotherPlayer')
        self.lua.execute('OpenFilters(1)')
        self.assertEqual(self.lua.eval('#menuEntries'), 1)

    def test_disable_clears_rows_editor_and_highlight(self):
        self.snapshot()
        self.lua.execute('XPHotspots.ShowPanel();p=XPHotspots.pins[1];p.scripts.OnEnter(p)')
        self.receive('OFF')
        self.assertTrue(self.lua.eval('next(XPHotspots.rows)==nil'))
        self.lua.execute('OpenFilters(1)')
        self.assertEqual(self.lua.eval('#menuEntries'), 1)
        self.assertFalse(self.lua.eval('XPHotspots.fills[1].shown'))
        self.assertEqual(self.lua.eval('#XPHotspots.queue'), 0)

    def test_atomic_save_and_ctrl_click_preserve_normal_map_click(self):
        self.snapshot()
        self.row(draft=True)
        self.lua.execute('XPHotspots.queue={};b=ButtonNamed("Save draft");b.scripts.OnClick(b)')
        self.assertEqual(self.lua.eval('#XPHotspots.queue'), 1)
        self.assertEqual(self.lua.eval('XPHotspots.queue[1]'), 'save 150 3600 1 3600 1 1 20 Redridge')
        self.lua.execute('control=true;WorldMapButton.scripts.OnMouseUp(WorldMapButton,"LeftButton")')
        self.assertEqual(self.lua.eval('oldClicks'), 0)
        self.assertEqual(self.lua.eval('XPHotspots.queue[2]'), 'center 75 75')
        self.lua.execute('control=false;WorldMapButton.scripts.OnMouseUp(WorldMapButton,"LeftButton")')
        self.assertEqual(self.lua.eval('oldClicks'), 1)

    def test_disabled_icon_still_available_to_gm_catalog(self):
        self.lua.execute('XPHotspots.catalog=true')
        self.snapshot(enabled=False, icon=False)
        self.assertTrue(self.lua.eval('XPHotspots.pins[1].shown'))

    def test_incomplete_chunks_are_not_accepted(self):
        self.receive('B|1|1|1|1000|1|1')
        self.receive('R|0|3|{')
        self.receive('R|2|3|}')
        self.receive('E')
        self.assertTrue(self.lua.eval('next(XPHotspots.rows)==nil'))

    def test_regular_player_cannot_open_gm_editor(self):
        self.snapshot(gm=0)
        self.lua.execute('XPHotspots.ShowPanel()')
        self.assertTrue(self.lua.eval('ButtonNamed("Save draft")==nil'))

    def test_polygon_highlight_and_zone_mask_only_cover_defined_area(self):
        self.snapshot()
        self.lua.execute('''
XPHotspots.Highlight({kind="polygon",map=0,highlight=true,
 points={{x=20,y=20},{x=80,y=20},{x=50,y=80}}})
''')
        self.assertTrue(self.lua.eval('XPHotspots.fills[1].shown'))
        self.lua.execute('''
local mask={}
for i=1,64 do mask[i]=string.rep("0",32)..string.rep("1",32) end
XPHotspots.Highlight({kind="zone",map=0,highlight=true,
 bounds={100,0,100,0},mask=mask})
''')
        self.assertEqual(self.lua.eval('XPHotspots.fills[1].point[4]'), 50)
        self.assertEqual(self.lua.eval('XPHotspots.fills[1].width'), 50)

    def test_pending_refresh_retains_requested_page(self):
        self.snapshot()
        self.lua.execute('XPHotspots.catalog=true;XPHotspots.queue={};XPHotspots.Refresh(1);XPHotspots.Refresh(3)')
        self.receive('B|2|1|1|1000|1|9')
        self.row()
        self.receive('E')
        self.assertEqual(self.lua.eval('XPHotspots.page'), 3)
        self.assertEqual(self.lua.eval('XPHotspots.queue[2]'), 'list 0 44 3 ')

    def test_search_is_debounced_and_resets_page(self):
        self.snapshot()
        self.lua.execute('''
XPHotspots.ShowPanel();b=ButtonNamed("Browse / active");b.scripts.OnClick(b)
XPHotspots.refreshPending=false;XPHotspots.queue={}
s=XPHotspots.searchBox;s:SetText("Barr");s.scripts.OnTextChanged(s,true)
driver.scripts.OnUpdate(driver,0.3)
''')
        self.assertEqual(self.lua.eval('#XPHotspots.queue'), 0)
        self.lua.execute('''
clock=1000.2;s:SetText("Barrens");s.scripts.OnTextChanged(s,true)
clock=1000.7;driver.scripts.OnUpdate(driver,0.3)
''')
        self.assertEqual(self.lua.eval('messages[#messages][2]'), 'list 0 44 1 Barrens')

    def test_realm_brand_does_not_change_neutral_titles(self):
        self.snapshot()
        self.lua.execute('XPHotspots.ShowPanel();b=ButtonNamed("Browse / active");b.scripts.OnClick(b)')
        self.receive('BRAND|Bear Cave')
        self.assertTrue(self.lua.eval('ButtonNamed("Bear Cave Hotspots")==nil'))
        self.assertTrue(self.lua.eval('ButtonNamed("XP Hotspot Manager")~=nil'))
        self.receive('BRAND|')
        self.assertTrue(self.lua.eval('ButtonNamed("XP Hotspots")~=nil'))

    def test_filter_menu_preserves_filters_and_limits_entry_to_gms(self):
        self.lua.execute('OpenFilters(1)')
        self.assertEqual(self.lua.eval('#menuEntries'), 1)
        self.snapshot(gm=0)
        self.lua.execute('OpenFilters(1)')
        self.assertEqual(self.lua.eval('#menuEntries'), 1)
        self.snapshot()
        self.lua.execute('OpenFilters(1);savedEntry=menuEntries[2]')
        self.assertEqual(self.lua.eval('menuEntries[1].text'), 'Existing filters')
        self.assertEqual(self.lua.eval('savedEntry.text'), 'XP Hotspot Menu')
        self.lua.execute('savedEntry.func()')
        self.assertTrue(self.lua.eval('ButtonNamed("XP Hotspot Manager")~=nil'))
        self.lua.execute('OpenFilters(2,"teleport")')
        self.assertEqual(self.lua.eval('#menuEntries'), 1)
        self.assertEqual(self.lua.eval('menuEntries[1].text'), 'teleport')
        self.lua.execute('OpenFilters(1)')
        self.assertEqual(self.lua.eval('#menuEntries'), 2)
        self.receive('OFF')
        self.lua.execute('XPHotspots.ShowPanel=function() error("unauthorized") end;savedEntry.func()')

    def test_marker_click_opens_details_and_outlines_area_instead_of_pin(self):
        self.snapshot()
        self.lua.execute('p=XPHotspots.pins[1];p.scripts.OnClick(p)')
        self.assertTrue(self.lua.eval('ButtonNamed("Selected Hotspot #1")~=nil'))
        self.assertTrue(self.lua.eval('ButtonNamed("Redridge")~=nil'))
        self.assertFalse(self.lua.eval('p.selection.shown'))
        self.assertEqual(self.lua.eval('p.width'), 24)
        self.assertGreater(self.lua.eval('XPHotspots.outlineCount'), 0)
        self.lua.execute('p.scripts.OnLeave(p)')
        self.assertFalse(self.lua.eval('p.selection.shown'))
        self.assertTrue(self.lua.eval('XPHotspots.outlines[1].shown'))

    def test_whole_zone_selection_has_padded_icon_border(self):
        self.snapshot(kind='zone')
        self.lua.execute('p=XPHotspots.pins[1];p.scripts.OnClick(p)')
        self.assertTrue(self.lua.eval('p.selection.shown'))
        self.assertEqual(self.lua.eval('p.width'), 24)
        self.assertEqual(self.lua.eval('p.selection.width'), 64)
        self.assertEqual(self.lua.eval('XPHotspots.outlineCount'), 0)

    def test_shift_marker_selection_toggles_merge_outline(self):
        self.snapshot()
        self.lua.execute('p=XPHotspots.pins[1];shift=true;p.scripts.OnClick(p)')
        self.assertEqual(self.lua.eval('XPHotspots.merge[1]'), 1)
        self.assertGreater(self.lua.eval('XPHotspots.outlineCount'), 0)
        self.assertFalse(self.lua.eval('p.selection.shown'))
        self.lua.execute('p.scripts.OnClick(p)')
        self.assertTrue(self.lua.eval('XPHotspots.merge[1]==nil'))
        self.assertEqual(self.lua.eval('XPHotspots.outlineCount'), 0)
        self.assertFalse(self.lua.eval('XPHotspots.outlines[1].shown'))

    def test_selection_visible_when_player_highlight_disabled_and_clears_on_other_map(self):
        self.snapshot(highlight=False)
        self.lua.execute('p=XPHotspots.pins[1];p.scripts.OnClick(p)')
        self.assertGreater(self.lua.eval('XPHotspots.outlineCount'), 0)
        self.lua.execute('C_WorldMap.GetMapIDByAreaID=function() return 1 end;XPHotspots.Draw()')
        self.assertEqual(self.lua.eval('XPHotspots.outlineCount'), 0)
        self.assertFalse(self.lua.eval('XPHotspots.outlines[1].shown'))

    def test_world_map_reprojects_pins_and_clears_zone_boundary(self):
        self.snapshot()
        self.lua.execute('''
p=XPHotspots.pins[1];p.scripts.OnClick(p)
GetCurrentMapAreaID=function() return 0 end
C_WorldMap.GetMapPosition=function(map,x,y,z)
 projected={map,x,y,z};return 0.2,0.3
end
driver.scripts.OnEvent(driver,"WORLD_MAP_UPDATE")
''')
        self.assertEqual(self.lua.eval('p.point[4]'), 20)
        self.assertEqual(self.lua.eval('p.point[5]'), -30)
        self.assertEqual(self.lua.eval('projected[1]'), 0)
        self.assertEqual(self.lua.eval('XPHotspots.outlineCount'), 0)
        self.assertFalse(self.lua.eval('XPHotspots.outlines[1].shown'))
        self.lua.execute('C_WorldMap.GetMapPosition=function() return 0,0 end;XPHotspots.Draw()')
        self.assertFalse(self.lua.eval('p.shown'))

    def test_perimeter_does_not_box_union_interior(self):
        self.snapshot()
        self.lua.execute('''
XPHotspots.outlineCount=0
XPHotspots.Outline({kind="union",map=0,parts={
 {kind="circle",x=50,y=40,radius=20},
 {kind="circle",x=50,y=60,radius=20}}})
interiorEdges=0
for i=1,XPHotspots.outlineCount do
 local t=XPHotspots.outlines[i]
 if math.abs(t.point[4]-50)<5 and math.abs(t.point[5]+50)<5 then interiorEdges=interiorEdges+1 end
end
''')
        self.assertGreater(self.lua.eval('XPHotspots.outlineCount'), 0)
        self.assertEqual(self.lua.eval('interiorEdges'), 0)

    def test_delete_requires_confirmation_and_keeps_original_id_revision(self):
        self.snapshot()
        self.lua.execute('''
XPHotspots.Select(XPHotspots.rows[1]);XPHotspots.queue={}
local b=ButtonNamed("Delete selected");b.scripts.OnClick(b)
''')
        self.assertEqual(self.lua.eval('#XPHotspots.queue'), 0)
        self.assertIn('cannot be reversed', self.lua.eval('StaticPopupDialogs[popup.which].text'))
        self.lua.execute('''
XPHotspots.Select({id=2,revision=9,name="Other"})
StaticPopupDialogs[popup.which].OnAccept(popup)
''')
        self.assertEqual(self.lua.eval('XPHotspots.queue[1]'), 'delete 1 1')
        self.receive('DELETED|1')
        self.assertTrue(self.lua.eval('XPHotspots.rows[1]==nil'))

    def test_teleport_uses_selected_revision_and_browser_button_is_full_width_bottom(self):
        self.snapshot()
        self.lua.execute('''
XPHotspots.Select(XPHotspots.rows[1]);XPHotspots.queue={}
local b=ButtonNamed("Teleport to center");b.scripts.OnClick(b)
''')
        self.assertEqual(self.lua.eval('XPHotspots.queue[1]'), 'teleport 1 1')
        self.assertEqual(self.lua.eval('ButtonNamed("Browse / active").width'), 275)
        self.assertLess(self.lua.eval('ButtonNamed("Browse / active").point[3]'),
                        self.lua.eval('ButtonNamed("Cancel draft").point[3]'))

    def test_server_invalidated_draft_cannot_be_resubmitted(self):
        self.snapshot()
        self.row(draft=True)
        self.lua.execute('XPHotspots.merge[1]=1;XPHotspots.queue={}')
        self.receive('DRAFTCLEARED|A hotspot used by this draft was permanently deleted.')
        self.lua.execute('local b=ButtonNamed("Save draft");b.scripts.OnClick(b)')
        self.assertEqual(self.lua.eval('#XPHotspots.queue'), 0)
        self.assertTrue(self.lua.eval('next(XPHotspots.merge)==nil'))

    def test_general_settings_save_units_and_explicit_bulk_option(self):
        self.snapshot()
        self.receive('G|1|3|150|3600|3600|20|20|20|20|20|20|20|4')
        self.lua.execute('XPHotspots.ShowGeneral();XPHotspots.queue={};b=ButtonNamed("Save general settings");b.scripts.OnClick(b)')
        self.assertEqual(self.lua.eval('XPHotspots.queue[1]'), 'general 1 3 150 3600 3600 20 20 20 20 20 20 20 4 0')
        self.lua.execute('''
for _,f in ipairs(frames) do
 if f.kind=="Font" and f.text=="Apply defaults to all existing hotspots" then f.point[2]:SetChecked(true) end
end
b.scripts.OnClick(b)
''')
        self.assertTrue(self.lua.eval('XPHotspots.queue[2]:match("4 1$")~=nil'))
        self.receive('G|1|3|150|3600|3600|20|20|20|20|20|20|20|5')
        self.receive('OK|General settings saved.')
        self.lua.execute('b.scripts.OnClick(b)')
        self.assertTrue(self.lua.eval('XPHotspots.queue[3]:match("5 0$")~=nil'))

    def test_general_settings_are_gm_only_and_validate_fields(self):
        self.snapshot(gm=0)
        self.lua.execute('XPHotspots.ShowGeneral()')
        self.assertTrue(self.lua.eval('ButtonNamed("Save general settings")==nil'))
        self.snapshot()
        self.receive('G|1|3|150|3600|3600|20|20|20|20|20|20|20|1')
        self.lua.execute('XPHotspots.ShowGeneral();XPHotspots.queue={};ButtonNamed("3"):SetText("21");b=ButtonNamed("Save general settings");b.scripts.OnClick(b)')
        self.assertEqual(self.lua.eval('#XPHotspots.queue'), 0)

    def test_general_unsaved_edits_survive_snapshot_and_stale_save(self):
        self.snapshot()
        self.receive('G|1|3|150|3600|3600|20|20|20|20|20|20|20|4')
        self.lua.execute('XPHotspots.ShowGeneral();edit=ButtonNamed("3");edit:SetText("5");edit.scripts.OnTextChanged(edit,true)')
        self.receive('G|1|7|150|3600|3600|20|20|20|20|20|20|20|5')
        self.assertEqual(self.lua.eval('edit:GetText()'), '5')
        self.lua.execute('XPHotspots.queue={};b=ButtonNamed("Save general settings");b.scripts.OnClick(b)')
        self.assertTrue(self.lua.eval('XPHotspots.queue[1]:match("4 0$")~=nil'))
        self.receive('ERROR|Invalid or stale general settings. Refresh and try again.')
        self.assertEqual(self.lua.eval('edit:GetText()'), '5')
        self.lua.execute('b=ButtonNamed("Reload");b.scripts.OnClick(b)')
        self.receive('G|1|7|150|3600|3600|20|20|20|20|20|20|20|5')
        self.assertEqual(self.lua.eval('edit:GetText()'), '7')

    def test_new_polygon_starts_at_map_cursor(self):
        self.snapshot()
        self.lua.execute('''
XPHotspots.ShowPanel();XPHotspots.queue={}
b=ButtonNamed("Polygon");b.scripts.OnClick(b)
control=true;WorldMapButton.scripts.OnMouseUp(WorldMapButton,"LeftButton")
''')
        self.assertEqual(self.lua.eval('XPHotspots.queue[1]'), 'newmap polygon 0 44 75 75 New Hotspot')
        self.assertEqual(self.lua.eval('XPHotspots.queue[2]'), 'point 75 75')

    def test_merge_uses_revisions_and_union_highlight_preserves_gap(self):
        self.snapshot()
        self.lua.execute('''
XPHotspots.ShowPanel();XPHotspots.queue={};XPHotspots.merge={[1]=3,[2]=7}
b=ButtonNamed("Merge selection");b.scripts.OnClick(b)
XPHotspots.Highlight({kind="union",map=0,highlight=true,parts={
 {kind="circle",x=50,y=20,radius=10},{kind="circle",x=50,y=80,radius=10}}})
''')
        self.assertEqual(self.lua.eval('XPHotspots.queue[1]'), 'merge 1 3 2 7')
        self.assertTrue(self.lua.eval('XPHotspots.fills[1].shown'))
        self.assertTrue(self.lua.eval('''(function()
for _,f in ipairs(XPHotspots.fills) do
 if f.shown and f.point[4]<50 and f.point[4]+f.width>50 then return false end
end
return true end)()'''))


if __name__ == '__main__':
    unittest.main()
