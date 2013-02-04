using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SdlDotNet.Graphics;
using SdlDotNet.Graphics.Primitives;
using SdlDotNet.Graphics.Sprites;
using SdlDotNet.Core;
using SdlDotNet.Input;
using System.Drawing;

namespace GameClient {

    class CQuest {
        public UInt32 QuestID;
        public String Title;
        public String Text;
    };

    class Program {
        private AnimatedSprite hero = new AnimatedSprite();
        private Size sz = new Size(64, 64);
        private Surface m_SpriteSheet;
        private Surface m_vidsurf;
        private Surface m_statsandslots;
        private Surface m_infobox;
        private Surface m_npcDialogBox;

        private Surface m_spriteNpcs;

        private Rectangle MapArea = new Rectangle();
        private Rectangle ActionInfoArea = new Rectangle();
        private Rectangle SlotsAndStatsArea = new Rectangle();
        private Rectangle InfoBoxArea = new Rectangle();
        private Rectangle RoomInfoTextArea = new Rectangle();

        private Rectangle StatsTextArea = new Rectangle();

        private Rectangle NPCDialogArea = new Rectangle();
        private Rectangle NPCDialogQuestTitleArea = new Rectangle();
        private Rectangle NPCDialogQuestTextArea = new Rectangle();

        private UInt32 CurrentNPC = 0;

        private GameNet net;

        private String StrActionInfo = "";
        private String StrMapInfo = "";
        private String StrRoomInfo = "";
        private byte RoomEnvType = 0;

        private List<CQuest> QuestsAvailable = new List<CQuest>();
        private String StrCurrentQuestTitle = "";
        private List<String> CurrentQuestText = null;
        private UInt32 CurrentQuestId = 0;

        private String[] arrStrMap = null;

        private SpriteCollection tex = new SpriteCollection();

        private SdlDotNet.Graphics.Font font_actioninfo;
        private SdlDotNet.Graphics.Font font_roominfo;
        private SdlDotNet.Graphics.Font font_questtext;
        private SdlDotNet.Graphics.Font font_questtexttitle;

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

            ActionInfoArea.X = 0;
            ActionInfoArea.Y = 0;
            ActionInfoArea.Height = 25;
            ActionInfoArea.Width = m_vidsurf.Width - 100;

            MapArea.X = 0;
            MapArea.Y = 25;
            MapArea.Height = m_vidsurf.Height - ActionInfoArea.Height - 100;
            MapArea.Width = m_vidsurf.Width - 100;

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

            NPCDialogQuestTextArea.X = 60;
            NPCDialogQuestTextArea.Y = 105;
            NPCDialogQuestTextArea.Width = 400;
            NPCDialogQuestTextArea.Height = 400;



            Events.Quit += new EventHandler<QuitEventArgs>(ApplicationQuitEventHandler);

            net = new GameNet();

            net.actioninfo += OnActionInfo;
            net.roominfo += OnRoomInfo;
            net.mapinfo += OnMapInfo;
            net.chatmsginfo += OnChatMessage;

            net.questtitlesinfo += OnQuestTitleInfo;
            net.questtextinfo += OnQuestTextInfo;

            net.Connect();

            LoadResources();

            Events.Fps = 60;
            Events.Tick += new EventHandler<TickEventArgs>(Events_Tick);

            Events.KeyboardDown += new EventHandler<KeyboardEventArgs>(Events_KeyboardDown);
            Events.KeyboardUp += new EventHandler<KeyboardEventArgs>(Events_KeyboardUp);

            Events.Run();
        }

        public void OnActionInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            StrActionInfo = str;

            frmDebug.addMessage(str + crlf[0]);
        }

        public void OnMapInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            StrMapInfo = str;

            //frmDebug.addMessage(str + crlf);

            arrStrMap = StrMapInfo.Split( crlf, StringSplitOptions.None );
        }

        public void OnRoomInfo(UInt32 command, UInt32 intparam1, UInt32 intparam2, String str) {
            StrRoomInfo = str;
            RoomEnvType = (byte)(intparam1 & 0xff);

            frmDebug.addMessage(str + crlf[0]);
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
            m_spriteNpcs.SourceColorKey = Color.FromArgb(255, 0, 255);
        }

        private void LoadTextures() {
            m_SpriteSheet = new Surface(@"..\..\Data\textures.png");
        }

        private void RenderActionInfo() {
            Surface m_ActionInfoSurf = font_actioninfo.Render(StrActionInfo, Color.Black);

            Video.Screen.Blit(m_ActionInfoSurf, ActionInfoArea);
        }

        private void RenderRoomInfo() {
            Video.Screen.Blit(m_infobox, InfoBoxArea);

            Surface m_RoomInfoSurf = font_actioninfo.Render(StrRoomInfo, Color.Black);

            Video.Screen.Blit(m_RoomInfoSurf, RoomInfoTextArea);
        }

        private void RenderNPCDialog() {
            Surface m_QuestTitle;

            if (CurrentNPC != 0) {
                Video.Screen.Blit(m_npcDialogBox, NPCDialogArea);

                // the dialog for when the player is receiving the quests the NPC can give the player
                if (CurrentQuestId == 0) {
                    int h = 0;

                    Rectangle r = new Rectangle(NPCDialogQuestTitleArea.X, NPCDialogQuestTitleArea.Y, NPCDialogQuestTitleArea.Width, NPCDialogQuestTitleArea.Height);

                    foreach (var q in QuestsAvailable) {
                        UInt32 id = q.QuestID;
                        String s = q.Title;

                        m_QuestTitle = font_actioninfo.Render(s, Color.Black);
                        Video.Screen.Blit(m_QuestTitle, r);

                        r.Y += 30;
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

            Surface m_Health     = font_actioninfo.Render("HP", Color.Black);   // current health, not a stat
            Surface m_Strength   = font_actioninfo.Render("Strength", Color.Black);
            Surface m_Energy     = font_actioninfo.Render("Energy", Color.Black);
            Surface m_Protection = font_actioninfo.Render("Protection", Color.Black);

            Surface m_HealthVal     = font_actioninfo.Render("100", Color.Black);
            Surface m_StrengthVal   = font_actioninfo.Render("0", Color.Black);
            Surface m_EnergyVal     = font_actioninfo.Render("0", Color.Black);
            Surface m_ProtectionVal = font_actioninfo.Render("0", Color.Black);


            StatsTextArea.X = SlotsAndStatsArea.X + 250;
            StatsTextArea.Width = 530;
            StatsTextArea.Height = 25;

            StatsTextArea.Y = SlotsAndStatsArea.Y + 10;
            Video.Screen.Blit(m_Health, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_Strength, StatsTextArea);
            
            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_Energy, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_Protection, StatsTextArea);

            StatsTextArea.X = SlotsAndStatsArea.X + 250 + 100;

            StatsTextArea.Y = SlotsAndStatsArea.Y + 10;
            Video.Screen.Blit(m_HealthVal, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_StrengthVal, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_EnergyVal, StatsTextArea);

            StatsTextArea.Y += 25;
            Video.Screen.Blit(m_ProtectionVal, StatsTextArea);

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
                        hero.X = worldpoint.X;
                        hero.Y = worldpoint.Y;
                    } else if ((arrStrMap[y][x] >= '0') && (arrStrMap[y][x] <= '9')) {
                        envtype = (byte)(UInt32.Parse(arrStrMap[y][x] + "") & 0xff);
                    } else if ((arrStrMap[y][x] >= 'A') && (arrStrMap[y][x] <= 'Z')) {
                        envtype = (byte)((arrStrMap[y][x] - 'A') & 0xff);
                    } else if ((arrStrMap[y][x] >= 'a') && (arrStrMap[y][x] <= 'z')) {
                        envtype = (byte)((arrStrMap[y][x] - 'a') & 0xff);
                        hero.X = worldpoint.X;
                        hero.Y = worldpoint.Y;
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

            
            Video.Screen.Blit(hero);
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

        private void Events_Tick(object sender, TickEventArgs e) {

            // Clear the screen, draw the hero and output to the window
            Video.Screen.Fill(Color.DarkGreen);
            try {
                RenderWorld();
                RenderSlotsAndStats();
                RenderActionInfo();
                RenderRoomInfo();
                RenderNPCDialog();
            } catch (System.ArgumentOutOfRangeException ex) {
                //Console.WriteLine(ex.StackTrace.ToString());
            }
            Video.Screen.Update();
        }

        private void Events_KeyboardDown(object sender, KeyboardEventArgs e) {
            // Check which key was pressed and change the animation accordingly
            switch (e.Key) {
                case Key.LeftArrow:
                    net.SendBinToServer(GameNet.c_run_walkleft, 0, 0, "");

                    hero.CurrentAnimation = "WalkLeft";
                    hero.Animate = true;
                    break;
                case Key.RightArrow:
                    net.SendBinToServer(GameNet.c_run_walkright, 0, 0, "");

                    hero.CurrentAnimation = "WalkRight";
                    hero.Animate = true;
                    break;
                case Key.DownArrow:
                    net.SendBinToServer(GameNet.c_run_walkbackwards, 0, 0, "");

                    hero.CurrentAnimation = "WalkDown";
                    hero.Animate = true;
                    break;
                case Key.UpArrow:
                    net.SendBinToServer(GameNet.c_run_walkforward, 0, 0, "");

                    hero.CurrentAnimation = "WalkUp";
                    hero.Animate = true;
                    break;
                case Key.Return:
                    // todo: getnpcname and id, start interaction
                    if (CurrentNPC != 0) {
                        net.SendBinToServer(GameNet.c_interact_getquesttext, 1, 0, "");
                    } else {
                        CurrentNPC = 2660;
                        net.SendBinToServer(GameNet.c_interact_getquesttitles, CurrentNPC, 0, "");
                    }

                    break;
                case Key.Escape:
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
