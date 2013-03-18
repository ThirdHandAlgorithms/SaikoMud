﻿using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SdlDotNet.Graphics;
using SdlDotNet.Graphics.Primitives;
using SdlDotNet.Graphics.Sprites;
using SdlDotNet.Core;
using SdlDotNet.Input;
using SdlDotNet.Audio;
using System.Drawing;
using System.Diagnostics;

namespace GameClient {

    class CQuest {
        public UInt32 QuestID;
        public String Title;
        public String Text;
    };

    class CNPC {
        public UInt32 WorldID;
        public String Name;
        public String CurrentDialog;
    };

    class CBaseStats {
        public UInt32 strength = 0;
        public UInt32 energy = 0;
        public UInt32 protection = 0;
    };

    class CCharSlot {
        public UInt32 slot_id = 0;
        public UInt32 item_id = 0;
        public String item_name = "";

        public CItem cacheditem = null;
    }

    class CCharacter : CBaseStats {
        public String Nickname = "";
        public UInt32 WorldID = 0;

        public CCharSlot slot1 = null;
        public CCharSlot slot2 = null;
        public CCharSlot slot3 = null;
        public CCharSlot slot4 = null;
        public CCharSlot slot5 = null;
    };

    class CCharSelf : CCharacter {
        public UInt32 level = 0;
        public UInt32 totalxp = 0;
        public UInt32 totalhp = 0;
        public UInt32 hp = 0;
    };

    class CItem : CBaseStats {
        public UInt64 Id = 0;
        public String Name = "";
        public List<String> Description;
        public UInt32 Type = 0;
        public UInt32 Slot = 0;
    };

    class Program {
        private AnimatedSprite hero = new AnimatedSprite();
        private Size sz = new Size(64, 64);

        // todo: why am i using m_, what kind of shitty standard is that??
        private Surface m_SpriteSheet;
        private Surface m_vidsurf;
        private Surface m_statsandslots;
        private Surface m_infobox;
        private Surface m_npcDialogBox;
        private Surface m_spriteNpcs;
        private Surface m_DeathScreen;
        private Surface m_TooltipBox;
        private Music m_level_music;

        private Rectangle MapArea = new Rectangle();
        private Rectangle ActionInfoArea = new Rectangle();
        private Rectangle EventInfoArea = new Rectangle();
        private Rectangle SlotsAndStatsArea = new Rectangle();
        private Rectangle InfoBoxArea = new Rectangle();
        private Rectangle RoomInfoTextArea = new Rectangle();

        private Rectangle StatsTextArea = new Rectangle();

        private Rectangle NPCDialogArea = new Rectangle();
        private Rectangle NPCDialogQuestTitleArea = new Rectangle();
        private Rectangle NPCDialogQuestTextArea = new Rectangle();

        private Rectangle DeathMessageArea = new Rectangle();

        private Point ToolTipLocation = new Point();

        private Point LastHeroPos = new Point();
        private Point NewHeroPos = new Point();

        private Surface m_generic_icon_slot1;
        private Surface m_generic_icon_slot2;
        private Surface m_generic_icon_slot3;
        private Surface m_generic_icon_slot4;
        private Surface m_generic_icon_slot5;
        private Rectangle Slot1Area = new Rectangle();
        private Rectangle Slot2Area = new Rectangle();
        private Rectangle Slot3Area = new Rectangle();
        private Rectangle Slot4Area = new Rectangle();
        private Rectangle Slot5Area = new Rectangle();

        private UInt32 CurrentNPC = 0;

        private GameNet net;

        private String StrActionInfo = "";
        private DateTime LastActionTime = DateTime.Now;

        private String StrMapInfo = "";
        private String StrRoomInfo = "";
        private byte RoomEnvType = 0;

        private CCharSelf CharSelf = new CCharSelf();
        private CItem LatestItem = new CItem();

        private List<CQuest> QuestsAvailable = new List<CQuest>();
        private List<CNPC> NPCsAvailable = new List<CNPC>();

        private String StrCurrentQuestTitle = "";
        private List<String> CurrentQuestText = null;
        private UInt32 CurrentQuestId = 0;

        private String[] arrStrMap = null;

        private SpriteCollection tex = new SpriteCollection();

        private SdlDotNet.Graphics.Font font_actioninfo;
        private SdlDotNet.Graphics.Font font_roominfo;
        private SdlDotNet.Graphics.Font font_questtext;
        private SdlDotNet.Graphics.Font font_questtexttitle;
        private SdlDotNet.Graphics.Font font_iteminfo;

        private String LastEventStr = "";
        private DateTime LastEventTime = DateTime.Now;

        private long lLoopTime = 0;

        string[] crlf = { "" + (char)(13) + (char)(10) };

        private Debug frmDebug;

        static void Main(string[] args) {
            new Program();
        }

        public Program() {
            String windir = "";
            if (System.Environment.OSVersion.Platform == PlatformID.Win32NT) {
                System.Environment.SetEnvironmentVariable("SDL_VIDEODRIVER", "directx");

                windir = System.Environment.GetFolderPath(Environment.SpecialFolder.Windows);
            }

            Video.WindowIcon();
            Video.WindowCaption = "SaikoMUD GameClient";
            m_vidsurf = Video.SetVideoMode(1280, 720, 32, false, false, false, true);

            frmDebug = new Debug();
            frmDebug.Show();

            font_actioninfo = new SdlDotNet.Graphics.Font(windir + "\\Fonts\\Comic.ttf", 18);
            font_roominfo = new SdlDotNet.Graphics.Font(windir + "\\Fonts\\Comic.ttf", 18);

            font_questtexttitle = new SdlDotNet.Graphics.Font(windir + "\\Fonts\\Comic.ttf", 16);
            font_questtext = new SdlDotNet.Graphics.Font(windir + "\\Fonts\\Comic.ttf", 14);

            font_iteminfo = new SdlDotNet.Graphics.Font(windir + "\\Fonts\\Comic.ttf", 14);

            ActionInfoArea.X = 0;
            ActionInfoArea.Y = 0;
            ActionInfoArea.Height = 25;
            ActionInfoArea.Width = m_vidsurf.Width - 100;

            EventInfoArea.X = 0;
            EventInfoArea.Y = 25;
            EventInfoArea.Height = 25;
            EventInfoArea.Width = m_vidsurf.Width - 100;

            MapArea.X = 0;
            MapArea.Y = 25;
            MapArea.Height = m_vidsurf.Height - ActionInfoArea.Height - 100;
            MapArea.Width = m_vidsurf.Width - 100;

            DeathMessageArea.X = 100;
            DeathMessageArea.Y = 100;

            SlotsAndStatsArea.X = m_vidsurf.Width - 550;
            SlotsAndStatsArea.Y = 25;

            InfoBoxArea.X = m_vidsurf.Width - 550;
            InfoBoxArea.Y = 25 + 245;
            
            RoomInfoTextArea.X = InfoBoxArea.X + 10;
            RoomInfoTextArea.Y = InfoBoxArea.Y + 10;

            NPCDialogArea.X = 50;
            NPCDialogArea.Y = 50;

            NPCDialogQuestTitleArea.X = 60;
            NPCDialogQuestTitleArea.Y = 60;
            NPCDialogQuestTitleArea.Width = 450;


            NPCDialogQuestTextArea.X = 60;
            NPCDialogQuestTextArea.Y = 105;
            NPCDialogQuestTextArea.Width = 350;
            NPCDialogQuestTextArea.Height = 400;

            Slot1Area.X = SlotsAndStatsArea.X + 86;
            Slot1Area.Y = SlotsAndStatsArea.Y + 10;
            Slot1Area.Width = 59;
            Slot1Area.Height = 59;

            Slot2Area.X = SlotsAndStatsArea.X + 86;
            Slot2Area.Y = SlotsAndStatsArea.Y + 94;
            Slot2Area.Width = 59;
            Slot2Area.Height = 59;

            Slot3Area.X = SlotsAndStatsArea.X + 86;
            Slot3Area.Y = SlotsAndStatsArea.Y + 160;
            Slot3Area.Width = 59;
            Slot3Area.Height = 59;

            Slot4Area.X = SlotsAndStatsArea.X + 13;
            Slot4Area.Y = SlotsAndStatsArea.Y + 94;
            Slot4Area.Width = 59;
            Slot4Area.Height = 59;

            Slot5Area.X = SlotsAndStatsArea.X + 160;
            Slot5Area.Y = SlotsAndStatsArea.Y + 94;
            Slot5Area.Width = 59;
            Slot5Area.Height = 59;

            Events.Quit += new EventHandler<QuitEventArgs>(ApplicationQuitEventHandler);

            net = new GameNet();

            net.actioninfo += OnActionInfo;
            net.roominfo += OnRoomInfo;
            net.mapinfo += OnMapInfo;
            net.chatmsginfo += OnChatMessage;

            net.questtitlesinfo += OnQuestTitleInfo;
            net.questtextinfo += OnQuestTextInfo;

            net.npcinfo += OnNPCInfo;
            net.dialog += OnDialog;

            net.earnsxp += OnEarnXP;
            net.statsinfo += OnStatsInfo;

            net.combatmsg += OnCombatEvent;
            net.iteminfo += OnItemInfo;
            net.itemstats += OnItemStatsInfo;
            
            net.gearslots += OnGearSlots;

            net.Connect();

            LoadResources();


            Events.Fps = 60;
            Events.Tick += new EventHandler<TickEventArgs>(Events_Tick);

            Events.KeyboardDown += new EventHandler<KeyboardEventArgs>(Events_KeyboardDown);
            Events.KeyboardUp += new EventHandler<KeyboardEventArgs>(Events_KeyboardUp);

            Events.Run();

            
            m_level_music.Play(1);
        }

        public void OnActionInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            StrActionInfo = str;

            if (CharSelf.WorldID == 0) {
                CharSelf.WorldID = intparam1;

                net.SendBinToServer(GameNet.c_self_getallstats, 0, 0, "");
                //net.SendBinToServer(GameNet.c_info_getgearslots, CharSelf.WorldID, 0, "");    // todo: bugfix communication first
            }

            LastActionTime = DateTime.Now;

            frmDebug.addMessage(str + crlf[0]);
        }

        public void OnMapInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            StrMapInfo = str;

            //frmDebug.addMessage(str + crlf);

            arrStrMap = StrMapInfo.Split( crlf, StringSplitOptions.None );
        }

        public void OnGearSlots(UInt32 command, List<UInt32> intarr, List<String> strarr) {

            if (intarr.Count == 6) {
                if (CharSelf.WorldID == intarr[0]) {
                    // item per slot
                    var slot1itemid = intarr[1];
                    var slot2itemid = intarr[2];
                    var slot3itemid = intarr[3];
                    var slot4itemid = intarr[4];
                    var slot5itemid = intarr[5];

                    var slot1itemname = strarr[1];
                    var slot2itemname = strarr[2];
                    var slot3itemname = strarr[3];
                    var slot4itemname = strarr[4];
                    var slot5itemname = strarr[5];

                    if (CharSelf.slot1 != null) {
                        if (CharSelf.slot1.item_id != slot1itemid) {
                            CharSelf.slot1 = null;
                        }
                    }
                    if (CharSelf.slot2 != null) {
                        if (CharSelf.slot2.item_id != slot2itemid) {
                            CharSelf.slot2 = null;
                        }
                    }
                    if (CharSelf.slot3 != null) {
                        if (CharSelf.slot3.item_id != slot3itemid) {
                            CharSelf.slot3 = null;
                        }
                    }
                    if (CharSelf.slot4 != null) {
                        if (CharSelf.slot4.item_id != slot4itemid) {
                            CharSelf.slot4 = null;
                        }
                    }
                    if (CharSelf.slot5 != null) {
                        if (CharSelf.slot5.item_id != slot5itemid) {
                            CharSelf.slot5 = null;
                        }
                    }

                    if ((CharSelf.slot1 == null) && (slot1itemid != 0)) {
                        CharSelf.slot1 = new CCharSlot();
                        CharSelf.slot1.item_id = slot1itemid;
                        CharSelf.slot1.item_name = slot1itemname;
                    }
                    if ((CharSelf.slot2 == null) && (slot2itemid != 0)) {
                        CharSelf.slot2 = new CCharSlot();
                        CharSelf.slot2.item_id = slot2itemid;
                        CharSelf.slot2.item_name = slot2itemname;
                    }
                    if ((CharSelf.slot3 == null) && (slot3itemid != 0)) {
                        CharSelf.slot3 = new CCharSlot();
                        CharSelf.slot3.item_id = slot3itemid;
                        CharSelf.slot3.item_name = slot3itemname;
                    }
                    if ((CharSelf.slot4 == null) && (slot4itemid != 0)) {
                        CharSelf.slot4 = new CCharSlot();
                        CharSelf.slot4.item_id = slot4itemid;
                        CharSelf.slot4.item_name = slot4itemname;
                    }
                    if ((CharSelf.slot5 == null) && (slot5itemid != 0)) {
                        CharSelf.slot5 = new CCharSlot();
                        CharSelf.slot5.item_id = slot5itemid;
                        CharSelf.slot5.item_name = slot5itemname;
                    }
                }
            }

        }

        public void OnItemInfo(UInt32 command, UInt32 itemid, UInt32 reserved, String sItemNameAndDescription, UInt32 iType, UInt32 iSlot) {
            // sItemNameAndDescription separated by |

            var arr = sItemNameAndDescription.Split('|');
            if (arr.Length == 2) {
                LatestItem.Id = itemid;
                LatestItem.Name = arr[0];
                LatestItem.Description = PrepareStringForDisplay(arr[1], font_iteminfo, 200);
                LatestItem.Type = iType;
                LatestItem.Slot = iSlot;
            } else {
                LatestItem.Id = 0;
            }
        }

        public void OnItemStatsInfo(UInt32 command, UInt32 itemid, UInt32 strength, String sItemNameAndDescription, UInt32 energy, UInt32 protection) {
            if (itemid == LatestItem.Id) {
                LatestItem.strength = strength;
                LatestItem.energy = energy;
                LatestItem.protection = protection;
            }
        }

        public void OnRoomInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            StrRoomInfo = str;
            RoomEnvType = (byte)(intparam1 & 0xff);

            frmDebug.addMessage(str + crlf[0]);

            NPCsAvailable.Clear();

            net.SendBinToServer(GameNet.c_radar_getnearbynpcs, 0, 0, "");
        }

        public void OnNPCInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            CNPC npc = new CNPC();
            npc.WorldID = intparam1;
            npc.Name = str;
            npc.CurrentDialog = "";

            NPCsAvailable.Add(npc);
        }

        public void OnDialog(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            foreach (CNPC npc in NPCsAvailable) {
                if (npc.WorldID == intparam1) {
                    npc.CurrentDialog = str;
                    break;
                }
            }
        }

        public CNPC GetKnownNPC(UInt32 iWorldId) {
            foreach (CNPC npc in NPCsAvailable) {
                if (npc.WorldID == iWorldId) {
                    return npc;
                }
            }

            return null;
        }

        public void OnQuestTitleInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            CQuest q = new CQuest();
            q.QuestID = intparam1;
            q.Title = str;

            QuestsAvailable.Add(q);

            // todo: temp
            //StrCurrentQuestTitle = q.Text;

            frmDebug.addMessage(str + crlf[0]);
        }

        public void OnEarnXP(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            // addxp intparam1
            // totalxp intparam2

            Int32 xpgain = (Int32)intparam1;
            if (xpgain < 0) {
                LastEventStr = "You have lost " + Math.Abs(xpgain) + " XP";
            } else {
                LastEventStr = "You have earned " + xpgain + " XP";
            }
            LastEventTime = DateTime.Now;
        }

        public void OnStatsInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            // command -> xp, lvl, str, enrgy, prot

            // intparam1 = worldid
            // intparam2 = statvalue

            if ((intparam1 == 0) || (intparam1 == CharSelf.WorldID)) {
                if (command == GameNet.c_event_statinfo_level) {
                    CharSelf.level = intparam2;
                } else if (command == GameNet.c_event_statinfo_totalxp) {
                    CharSelf.totalxp = intparam2;
                } else if (command == GameNet.c_event_statinfo_totalhp) {
                    CharSelf.totalhp = intparam2;
                } else if (command == GameNet.c_event_statinfo_hp) {
                    CharSelf.hp = intparam2;
                } else if (command == GameNet.c_event_statinfo_strength) {
                    CharSelf.strength = intparam2;
                } else if (command == GameNet.c_event_statinfo_energy) {
                    CharSelf.energy = intparam2;
                } else if (command == GameNet.c_event_statinfo_protection) {
                    CharSelf.protection = intparam2;
                }
            }

        }

        public void OnCombatEvent(UInt32 command, UInt32 source, UInt32 target, String str, UInt32 eventtype, UInt32 amount) {
            LastEventStr = str;
            LastEventTime = DateTime.Now;

            // todo: do something with this information.... combatlog?
            // and then request stats to update character display..

            frmDebug.addCombatMessage(str);

            if (source == CharSelf.WorldID) {
                StrActionInfo = str;
                LastActionTime = DateTime.Now;
            }

            // always request current stats if event is targetted at self, every event influences
            if (target == CharSelf.WorldID) {
                net.SendBinToServer(GameNet.c_self_getallstats, 0, 0, "");

                if (eventtype == GameNet.COMBATEVENT_DEATH) {
                    // we died!

                    // ... now what...

                    // TODO: GAME OVER overlay
                    // + teleport player to respawn point

                }
            }
        }

        public int FindPreviousSpace(String s, int iLastPos) {
            int i = iLastPos;
            if (i >= 0) {
                while (s[i] != ' ') {
                    i--;
                    if (i <= 0) {
                        i = 0;
                        break;
                    }
                }
            }
            return i;
        }

        public List<String> PrepareStringForDisplay(String s, SdlDotNet.Graphics.Font f, int maxwidth) {
            List<String> vs = new List<String>();

            String[] arrParagraphs = s.Split(crlf, StringSplitOptions.None);

            foreach (var line in arrParagraphs) {
                bool bDoneWithLine = false;
                String linepartleft = line;
                while (!bDoneWithLine) {
                    Size ls = f.SizeText(linepartleft);
                    int c1 = linepartleft.Length;
                    int p1 = c1;
                    if (p1 > 0) {
                        while (ls.Width > maxwidth) {
                            p1 = FindPreviousSpace(linepartleft, p1 - 1);

                            ls = f.SizeText(linepartleft.Substring(0, p1));
                        }

                        vs.Add(linepartleft.Substring(0, p1));
                    }

                    if (c1 > p1 + 1) {
                        linepartleft = linepartleft.Substring(p1 + 1);
                        if (linepartleft.Trim().Length == 0) {
                            bDoneWithLine = true;
                        }
                    } else {
                        bDoneWithLine = true;
                    }
                }

                vs.Add("");
            }

            return vs;
        }

        public List<String> PrepareQuestText(String s) {
            return PrepareStringForDisplay(s, font_questtext, 450);
        }

        public void OnQuestTextInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            /*
            var q =
                from quest in QuestsAvailable
                where quest.QuestID == intparam1
                select quest;
            // q is an enumerable... why loop over it twice?... let's not do linq here../
            */

            foreach (var q in QuestsAvailable) {
                if (q.QuestID == intparam1) {
                    q.Text = str;
                    StrCurrentQuestTitle = q.Title;
                    CurrentQuestText = PrepareQuestText(str);
                    CurrentQuestId = intparam1;
                    break;
                }
            }

            frmDebug.addMessage(str + crlf[0]);
        }

        public void OnChatMessage(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            // intparam is channelnumber (==2)
            // str is message

            frmDebug.addMessage(str + crlf[0]);
        }

        private void LoadResources() {
            LoadTextures();
            LoadHero();

            m_statsandslots = new Surface(@"..\..\Data\statsandslots.png");
            m_statsandslots.SourceColorKey = Color.FromArgb(255, 0, 255);

            m_infobox = new Surface(@"..\..\Data\infobox.png");
            m_infobox.SourceColorKey = Color.FromArgb(255, 0, 255);

            m_spriteNpcs = new Surface(@"..\..\Data\npcs.png");
            m_spriteNpcs.SourceColorKey = Color.FromArgb(255, 0, 255);

            m_npcDialogBox = new Surface(@"..\..\Data\npc_dialog.png");
            m_npcDialogBox.SourceColorKey = Color.FromArgb(255, 0, 255);

            m_level_music = new SdlDotNet.Audio.Music(@"..\..\Data\intro_test.mp3");
            m_DeathScreen = new Surface(@"..\..\Data\deathscreen.png");
            m_DeathScreen.SourceColorKey = Color.FromArgb(255, 0, 255);

            m_TooltipBox = new Surface(@"..\..\Data\tooltip.png");
            m_TooltipBox.SourceColorKey = Color.FromArgb(255, 0, 255);

            m_generic_icon_slot1 = new Surface(@"..\..\Data\generic_headslot.png");
            m_generic_icon_slot1.SourceColorKey = Color.FromArgb(255, 0, 255);
            m_generic_icon_slot2 = new Surface(@"..\..\Data\generic_bodyslot.png");
            m_generic_icon_slot2.SourceColorKey = Color.FromArgb(255, 0, 255);
            m_generic_icon_slot3 = new Surface(@"..\..\Data\generic_feetslot.png");
            m_generic_icon_slot3.SourceColorKey = Color.FromArgb(255, 0, 255);
            m_generic_icon_slot4 = new Surface(@"..\..\Data\generic_weaponslot.png");
            m_generic_icon_slot4.SourceColorKey = Color.FromArgb(255, 0, 255);
            m_generic_icon_slot5 = new Surface(@"..\..\Data\generic_handsslot.png");
            m_generic_icon_slot5.SourceColorKey = Color.FromArgb(255, 0, 255);

        }

        private void LoadTextures() {
            m_SpriteSheet = new Surface(@"..\..\Data\textures.png");
        }

        private void RenderActionInfo() {
            if (DateTime.Now <= LastActionTime.AddSeconds(5)) {
                Surface m_ActionInfoSurf = font_actioninfo.Render(StrActionInfo, Color.Black);
                Video.Screen.Blit(m_ActionInfoSurf, ActionInfoArea);
            }

            if (DateTime.Now <= LastEventTime.AddSeconds(5)) {
                Surface m_EventInfoSurf = font_actioninfo.Render(LastEventStr, Color.Red);
                Video.Screen.Blit(m_EventInfoSurf, EventInfoArea);
            }
        }

        private void RenderRoomInfo() {
            Video.Screen.Blit(m_infobox, InfoBoxArea);


            var arr = PrepareStringForDisplay(StrRoomInfo, font_questtext, 500);

            Rectangle liner = new Rectangle(RoomInfoTextArea.X, RoomInfoTextArea.Y, RoomInfoTextArea.Width, RoomInfoTextArea.Height);

            foreach (var line in arr) {
                Surface m_Text = font_questtext.Render(line, Color.Black);
                Video.Screen.Blit(m_Text, liner);

                liner.Y += m_Text.Height;
            }
        }

        private void RenderNPCDialog() {
            Surface m_QuestTitle;

            if (CurrentNPC != 0) {
                CNPC npc = GetKnownNPC(CurrentNPC);
                if (npc != null) {
                    Video.Screen.Blit(m_npcDialogBox, NPCDialogArea);

                    // the dialog for when the player is receiving the quests the NPC can give the player
                    if (CurrentQuestId == 0) {
                        List<String> arr = null;

                        Rectangle liner = new Rectangle(NPCDialogQuestTitleArea.X, NPCDialogQuestTitleArea.Y, NPCDialogQuestTitleArea.Width, NPCDialogQuestTitleArea.Height);

                        if (npc.CurrentDialog != "") {
                            arr = PrepareStringForDisplay(npc.CurrentDialog, font_questtext, NPCDialogQuestTitleArea.Width);
                            foreach (var line in arr) {
                                Surface m_Text = font_questtext.Render(line, Color.Black);
                                Video.Screen.Blit(m_Text, liner);

                                liner.Y += m_Text.Height;
                            }

                            if (arr.Count > 0) {
                                liner.Y += 30;
                            }
                        }

                        foreach (var q in QuestsAvailable) {
                            UInt32 id = q.QuestID;
                            String s = q.Title;

                            m_QuestTitle = font_actioninfo.Render(s, Color.Black);
                            Video.Screen.Blit(m_QuestTitle, liner);

                            liner.Y += m_QuestTitle.Height;
                        }

                        if (QuestsAvailable.Count == 0) {
                            m_QuestTitle = font_actioninfo.Render(npc.Name + " has no quests for you.", Color.Black);
                            Video.Screen.Blit(m_QuestTitle, liner);
                        }
                    }
                }


                // this is the dialog when the player is reading a quest (and thus has selected one already)
                if (CurrentQuestId != 0) {
                    m_QuestTitle = font_questtexttitle.Render(StrCurrentQuestTitle, Color.Black);
                    Video.Screen.Blit(m_QuestTitle, NPCDialogQuestTitleArea);

//                    Surface m_QuestText = font_actioninfo.Render(StrCurrentQuestText, Color.Black, true, 400, 20);
//                    Video.Screen.Blit(m_QuestText, NPCDialogQuestTextArea);

                    Rectangle liner = new Rectangle(NPCDialogQuestTextArea.X, NPCDialogQuestTextArea.Y, NPCDialogQuestTextArea.Width, NPCDialogQuestTextArea.Height);

                    foreach (var line in CurrentQuestText) {
                        Surface m_QuestText = font_questtext.Render(line, Color.Black);
                        Video.Screen.Blit(m_QuestText, liner);

                        liner.Y += m_QuestText.Height;
                    }
                }
            }
        }

        private void RenderSlotsAndStats() {
            Video.Screen.Blit(m_statsandslots, SlotsAndStatsArea);
            
            /*
            // test slot icons
            Video.Screen.Blit(m_generic_icon_slot1, Slot1Area);
            Video.Screen.Blit(m_generic_icon_slot2, Slot2Area);
            Video.Screen.Blit(m_generic_icon_slot3, Slot3Area);
            Video.Screen.Blit(m_generic_icon_slot4, Slot4Area);
            Video.Screen.Blit(m_generic_icon_slot5, Slot5Area);
             */

            if (CharSelf != null) {
                if (CharSelf.slot1 != null) {
                    if (CharSelf.slot1.item_id != 0) {
                        Video.Screen.Blit(m_generic_icon_slot1, Slot1Area);
                    }
                }

                if (CharSelf.slot2 != null) {
                    if (CharSelf.slot2.item_id != 0) {
                        Video.Screen.Blit(m_generic_icon_slot2, Slot2Area);
                    }
                }

                if (CharSelf.slot3 != null) {
                    if (CharSelf.slot3.item_id != 0) {
                        Video.Screen.Blit(m_generic_icon_slot3, Slot3Area);
                    }
                }

                if (CharSelf.slot4 != null) {
                    if (CharSelf.slot4.item_id != 0) {
                        Video.Screen.Blit(m_generic_icon_slot4, Slot4Area);
                    }
                }

                if (CharSelf.slot5 != null) {
                    if (CharSelf.slot5.item_id != 0) {
                        Video.Screen.Blit(m_generic_icon_slot5, Slot5Area);
                    }
                }
            }


            Surface m_Level = font_actioninfo.Render("Level", Color.Black);
            Surface m_XP = font_actioninfo.Render("XP", Color.Black);

            Surface m_Health = font_actioninfo.Render("HP", Color.Black);   // current health, not a stat
            Surface m_Strength   = font_actioninfo.Render("Strength", Color.Black);
            Surface m_Energy     = font_actioninfo.Render("Energy", Color.Black);
            Surface m_Protection = font_actioninfo.Render("Protection", Color.Black);

            Surface m_LevelVal = font_actioninfo.Render("" + CharSelf.level, Color.Black);
            Surface m_XPVal = font_actioninfo.Render("" + CharSelf.totalxp, Color.Black);
            Surface m_HealthVal     = font_actioninfo.Render("" + CharSelf.hp, Color.Black);
            Surface m_StrengthVal = font_actioninfo.Render("" + CharSelf.strength, Color.Black);
            Surface m_EnergyVal = font_actioninfo.Render("" + CharSelf.energy, Color.Black);
            Surface m_ProtectionVal = font_actioninfo.Render("" + CharSelf.protection, Color.Black);

            // statnames
            StatsTextArea.X = SlotsAndStatsArea.X + 250;
            StatsTextArea.Width = 530;
            StatsTextArea.Height = 25;

            StatsTextArea.Y = SlotsAndStatsArea.Y + 10;
            Video.Screen.Blit(m_Level, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_XP, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_Health, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_Strength, StatsTextArea);
            
            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_Energy, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_Protection, StatsTextArea);

            // values
            StatsTextArea.X = SlotsAndStatsArea.X + 250 + 100;

            StatsTextArea.Y = SlotsAndStatsArea.Y + 10;
            Video.Screen.Blit(m_LevelVal, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_XPVal, StatsTextArea);

            StatsTextArea.Y += 25;
            Rectangle bararea = new Rectangle();
            bararea.X = StatsTextArea.X - 2;
            bararea.Y = StatsTextArea.Y + 2;

            Surface hpbar = new Surface(102, 21);
            hpbar.Transparent = true;
            hpbar.TransparentColor = Color.Black;
            Box b = new Box(1, 1, (short)(CharSelf.hp), 19);
            b.Draw(hpbar, Color.Red, true, true);

            Video.Screen.Blit(hpbar, bararea);
            Video.Screen.Blit(m_HealthVal, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_StrengthVal, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_EnergyVal, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_ProtectionVal, StatsTextArea);
        }

        private void RenderItemTooltip() {
            if (LatestItem.Id != 0) {
                ToolTipLocation.X = 100;
                ToolTipLocation.Y = 100;

                Video.Screen.Blit(m_TooltipBox, ToolTipLocation);

                Rectangle r = new Rectangle(ToolTipLocation, new Size(m_TooltipBox.Width, m_TooltipBox.Height));

                r.X += 10;
                r.Y += 10;
                Surface title = font_iteminfo.Render(LatestItem.Name, Color.Black);
                Video.Screen.Blit(title, r);

                r.Y += 50;
                foreach (var s in LatestItem.Description) {
                    Surface description = font_iteminfo.Render(s, Color.Black);
                    Video.Screen.Blit(description, r);
                    r.Y += 25;
                }

                String slottext = "";
                switch (LatestItem.Slot) {
                    case 1: slottext = "headpiece"; break;
                    case 2: slottext = "bodyarmor"; break;
                    case 3: slottext = "shoes"; break;
                    case 4: slottext = "gloves"; break;
                    case 5: slottext = "weapon"; break;
                }

                r.Y = ToolTipLocation.Y + 170;
                Surface m_slottext = font_iteminfo.Render("Equipable as: " + slottext, Color.Black);
                Video.Screen.Blit(m_slottext, r);

                r.Y += 25;
                int ry = r.Y;

                Surface st = font_iteminfo.Render("Strength", Color.Black);
                Video.Screen.Blit(st, r);
                
                r.Y += 25;
                Surface en = font_iteminfo.Render("Energy", Color.Black);
                Video.Screen.Blit(en, r);

                r.Y += 25; 
                Surface pr = font_iteminfo.Render("Protection", Color.Black);
                Video.Screen.Blit(pr, r);

                r.Y = ry;
                r.X += 100;
                Surface stval = font_iteminfo.Render(LatestItem.strength + "", Color.Black);
                Video.Screen.Blit(stval, r);

                r.Y += 25;
                Surface enval = font_iteminfo.Render(LatestItem.energy + "", Color.Black);
                Video.Screen.Blit(enval, r);

                r.Y += 25;
                Surface prval = font_iteminfo.Render(LatestItem.protection + "", Color.Black);
                Video.Screen.Blit(prval, r);
            }
        }

        private void RenderHero() {
            long xDiff = NewHeroPos.X - LastHeroPos.X;
            long yDiff = NewHeroPos.Y - LastHeroPos.Y;

            if (lLoopTime != 0) {
                if (xDiff != 0) {
                    hero.X += (int)(xDiff * (lLoopTime / 1000));
                }

                if (yDiff != 0) {
                    hero.Y += (int)(xDiff * (lLoopTime / 1000));
                }
            } else {
                hero.X = NewHeroPos.X;
                hero.Y = NewHeroPos.Y;
            }

            if (hero.X == NewHeroPos.X) {
                LastHeroPos.X = hero.X;
            }
            if (hero.Y == NewHeroPos.Y) {
                LastHeroPos.Y = hero.Y;
            }

            Video.Screen.Blit(hero);
        }

        private void RenderWorld() {
            Point worldpoint = new Point(0, 0);
            Point spriteoffset = new Point(0, 0);

            Rectangle spriterect = new Rectangle(spriteoffset, sz);

            if (arrStrMap == null) {
                return;
            }

            int maxy = arrStrMap.Length;
            int maxx = 0;

            if (maxy > 0) {
                maxx = arrStrMap[0].Length;
            }
            
            worldpoint.Y = MapArea.Y;
            for (int y = 0; y < maxy; y++) {
                worldpoint.X = MapArea.X;
                maxx = arrStrMap[y].Length;
                for (int x = 0; x < maxx; x++) {
                    byte envtype = 0;
                    if (arrStrMap[y][x] == '@') {
                        envtype = this.RoomEnvType;
                        NewHeroPos.X = worldpoint.X;
                        NewHeroPos.Y = worldpoint.Y;
                    } else if ((arrStrMap[y][x] >= '0') && (arrStrMap[y][x] <= '9')) {
                        envtype = (byte)(UInt32.Parse(arrStrMap[y][x] + "") & 0xff);
                    } else if ((arrStrMap[y][x] >= 'A') && (arrStrMap[y][x] <= 'Z')) {
                        envtype = (byte)((arrStrMap[y][x] - 'A') & 0xff);
                    } else if ((arrStrMap[y][x] >= 'a') && (arrStrMap[y][x] <= 'z')) {
                        envtype = (byte)((arrStrMap[y][x] - 'a') & 0xff);
                        NewHeroPos.X = worldpoint.X;
                        NewHeroPos.Y = worldpoint.Y;
                    } else {
                        envtype = 0;
                    }

                    spriterect.Y = envtype * sz.Height;
                    Video.Screen.Blit(m_SpriteSheet, worldpoint, spriterect);

                    if ((arrStrMap[y][x] >= 'A') && (arrStrMap[y][x] <= 'Z')) {
                        Rectangle r = new Rectangle();
                        r.X = worldpoint.X;
                        r.Y = worldpoint.Y;
                        Video.Screen.Blit(m_spriteNpcs, r);
                    } else if ((arrStrMap[y][x] >= 'a') && (arrStrMap[y][x] <= 'z')) {
                        Rectangle r = new Rectangle();
                        r.X = worldpoint.X;
                        r.Y = worldpoint.Y;
                        Video.Screen.Blit(m_spriteNpcs, r);
                    }

                    worldpoint.X += sz.Width;
                }
                worldpoint.Y += sz.Height;
            }


            RenderHero();
        }

        private void LoadHero() {
            string filePath = Path.Combine("..", "..");
            string fileDirectory = "Data";
            string fileName = "puppet.png";
            if (File.Exists(fileName)) {
                filePath = "";
                fileDirectory = "";
            } else if (File.Exists(Path.Combine(fileDirectory, fileName))) {
                filePath = "";
            }

            string file = Path.Combine(Path.Combine(filePath, fileDirectory), fileName);

            // Load the image
            Surface image = new Surface(file);
            image.SourceColorKey = Color.FromArgb(255, 0, 255);


            // Create the animation frames
            SurfaceCollection walkUp = new SurfaceCollection();
            walkUp.Add(image, sz, 0);
            SurfaceCollection walkRight = new SurfaceCollection();
            walkRight.Add(image, sz, 1);
            SurfaceCollection walkDown = new SurfaceCollection();
            walkDown.Add(image, sz, 2);
            SurfaceCollection walkLeft = new SurfaceCollection();
            walkLeft.Add(image, sz, 3);

            // Add the animations to the hero
            AnimationCollection animWalkUp = new AnimationCollection();
            animWalkUp.Add(walkUp, 1);
            hero.Animations.Add("WalkUp", animWalkUp);
            AnimationCollection animWalkRight = new AnimationCollection();
            animWalkRight.Add(walkRight, 1);
            hero.Animations.Add("WalkRight", animWalkRight);
            AnimationCollection animWalkDown = new AnimationCollection();
            animWalkDown.Add(walkDown, 1);
            hero.Animations.Add("WalkDown", animWalkDown);
            AnimationCollection animWalkLeft = new AnimationCollection();
            animWalkLeft.Add(walkLeft, 1);
            hero.Animations.Add("WalkLeft", animWalkLeft);

            // Setup the startup animation and make him not walk
            hero.CurrentAnimation = "WalkUp";
            hero.Animate = false;
            // Put him in the center of the screen
            hero.Center = new Point(
                MapArea.X + hero.Width / 2,
                MapArea.Y + hero.Height / 2);
        }

        private void RenderDeath() {
            //DeathMessageArea

            Video.Screen.Blit(m_DeathScreen, DeathMessageArea);

        }

        private void Events_Tick(object sender, TickEventArgs e) {
            Stopwatch sw = new Stopwatch();

            sw.Start();

            // Clear the screen, draw the hero and output to the window
            Video.Screen.Fill(Color.DarkGreen);
            try {
                RenderWorld();
                RenderSlotsAndStats();
                RenderActionInfo();
                RenderRoomInfo();
                RenderNPCDialog();
                RenderItemTooltip();

                if (CharSelf.hp == 0) {
                    RenderDeath();
                }
            } catch (System.ArgumentOutOfRangeException ex) {
                //Console.WriteLine(ex.StackTrace.ToString());
            }
            Video.Screen.Update();

            sw.Stop();

            lLoopTime = sw.ElapsedMilliseconds;
        }

        private void Events_KeyboardDown(object sender, KeyboardEventArgs e) {
            // Check which key was pressed and change the animation accordingly
            switch (e.Key) {
                case Key.LeftArrow:
                    net.SendBinToServer(GameNet.c_run_walkleft, 0, 0, "");

                    hero.CurrentAnimation = "WalkLeft";
                    hero.Animate = true;

                    CurrentNPC = 0;
                    CurrentQuestId = 0;
                    break;
                case Key.RightArrow:
                    net.SendBinToServer(GameNet.c_run_walkright, 0, 0, "");

                    hero.CurrentAnimation = "WalkRight";
                    hero.Animate = true;

                    CurrentNPC = 0;
                    CurrentQuestId = 0;
                    break;
                case Key.DownArrow:
                    net.SendBinToServer(GameNet.c_run_walkbackwards, 0, 0, "");

                    hero.CurrentAnimation = "WalkDown";
                    hero.Animate = true;

                    CurrentNPC = 0;
                    CurrentQuestId = 0;
                    break;
                case Key.UpArrow:
                    net.SendBinToServer(GameNet.c_run_walkforward, 0, 0, "");

                    hero.CurrentAnimation = "WalkUp";
                    hero.Animate = true;

                    CurrentNPC = 0;
                    CurrentQuestId = 0;
                    break;
                case Key.KeypadMultiply:
                    if (this.NPCsAvailable.Count() > 0) {
                        net.SendBinToServer(GameNet.c_attack_start, this.NPCsAvailable[0].WorldID, 0, "");
                    }
                    break;
                case Key.Return:
                    // todo: getnpcname and id, start interaction
                    if (CurrentNPC != 0) {
                        if (this.QuestsAvailable.Count() > 0) {
                            net.SendBinToServer(GameNet.c_interact_getquesttext, this.QuestsAvailable[0].QuestID, 0, "");
                        }
                    } else {
                        if (this.NPCsAvailable.Count() > 0) {
                            CurrentNPC = this.NPCsAvailable[0].WorldID;
                            this.QuestsAvailable.Clear();
                            net.SendBinToServer(GameNet.c_interact_greet, CurrentNPC, 0, "");
                            net.SendBinToServer(GameNet.c_interact_getquesttitles, CurrentNPC, 0, "");
                        }
                    }

                    break;
                case Key.Escape:
                    CurrentNPC = 0;
                    CurrentQuestId = 0;

                    break;
                case Key.Keypad1:
                    net.SendBinToServer(GameNet.c_info_getiteminfo, 1, 0, "");
                    break;
                case Key.Keypad2:
                    net.SendBinToServer(GameNet.c_info_getiteminfo, 2, 0, "");
                    break;
                case Key.Keypad8:
                    net.SendBinToServer(GameNet.c_info_getgearslots, CharSelf.WorldID, 0, "");
                    break;
                case Key.Keypad9:
                    List<UInt32> stuffi = new List<UInt32>();
                    List<String> stuffs = new List<String>();

                    stuffi.Add(123);
                    stuffi.Add(456);
                    stuffi.Add(789);

                    stuffs.Add("123");
                    stuffs.Add("123456");

                    net.SendBin2ToServer(GameNet.c_response_gearslots, stuffi, stuffs);
                    break;
                case Key.Q:
                    Events.QuitApplication();
                    break;
            }
        }

        private void Events_KeyboardUp(object sender, KeyboardEventArgs e) {
            // Check which key was brought up and stop the hero if needed
            if (e.Key == Key.LeftArrow && hero.CurrentAnimation == "WalkLeft") {
                hero.Animate = false;
            } else if (e.Key == Key.UpArrow && hero.CurrentAnimation == "WalkUp") {
                hero.Animate = false;
            } else if (e.Key == Key.DownArrow && hero.CurrentAnimation == "WalkDown") {
                hero.Animate = false;
            } else if (e.Key == Key.RightArrow && hero.CurrentAnimation == "WalkRight") {
                hero.Animate = false;
            }
        }

        private void ApplicationQuitEventHandler(object sender, QuitEventArgs args) {
            Events.Tick -= Events_Tick;

            net.Disconnect();

            Events.QuitApplication();
        }
    }
}
