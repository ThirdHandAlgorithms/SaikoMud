using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace GameClient {

    class CLinkedItem {
        public UInt32 item_id;
        public UInt32 amount;
        public String str;
    };

    class CQuest {
        public UInt32 QuestID;
        public String Title;
        public String Text;
        public bool CanComplete;

        public List<CLinkedItem> RequiredItems = new List<CLinkedItem>();

        public CLinkedItem GetKnownRequirement(UInt32 item_id) {
            foreach (var item in this.RequiredItems) {
                if (item.item_id == item_id) {
                    return item;
                }
            }

            return null;
        }

        public void UpdateRequirement(UInt32 iItemId, UInt32 iAmountRequired, String sStr) {
            CLinkedItem anItem = GetKnownRequirement(iItemId);
            bool b = false;
            if (anItem == null) {
                anItem = new CLinkedItem();
                b = true;
            }

            anItem.item_id = iItemId;
            anItem.amount = iAmountRequired;
            anItem.str = sStr;

            if (b) {
                RequiredItems.Add(anItem);
            }
        }
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
    }

    class CPositionedChar : CBaseStats {
        public UInt32 WorldID = 0;
        public String Nickname = "";
        public Int32 X;
        public Int32 Y;
        public bool isDead = false;
    };

    class CNPC : CPositionedChar {
        public String CurrentDialog;
    };

    class CCharacter : CPositionedChar {
        public CCharSlot slot1 = null;
        public CCharSlot slot2 = null;
        public CCharSlot slot3 = null;
        public CCharSlot slot4 = null;
        public CCharSlot slot5 = null;
    };

    class CAnotherPlayer : CCharacter {
    };

    class CCharSelf : CCharacter {
        public UInt32 level = 0;
        public UInt32 totalxp = 0;
        public UInt32 totalhp = 0;
        public UInt32 hp = 0;

        public List<CLinkedItem> bagslots = new List<CLinkedItem>();

        public UInt32 CountInBags(UInt32 iItemId) {
            UInt32 c = 0;

            foreach (var item in bagslots) {
                if (item.item_id == iItemId) {
                    c += item.amount;
                }
            }

            return c;
        }
    };

    class CItem : CBaseStats {
        public UInt64 Id = 0;
        public String Name = "";
        public List<String> Description;
        public UInt32 Type = 0;
        public UInt32 Slot = 0;
    };



    class CGameObjects {
        protected List<CItem> ItemCache = new List<CItem>();
        protected List<CAnotherPlayer> CharactersHere = new List<CAnotherPlayer>();
        protected List<CNPC> NPCsAvailable = new List<CNPC>();
        protected List<CQuest> QuestsAvailable = new List<CQuest>();

        public CAnotherPlayer FindChar(UInt32 iWorldId) {
            foreach (var c in CharactersHere) {
                if (c.WorldID == iWorldId) {
                    return c;
                }
            }

            return null;
        }

        public CAnotherPlayer FindOrAddChar(UInt32 iWorldId) {
            var c = FindChar(iWorldId);
            if (c == null) {
                c = new CAnotherPlayer();
                c.WorldID = iWorldId;

                CharactersHere.Add(c);
            }

            return c;
        }

        public CNPC FindKnownNPC(UInt32 iWorldId) {
            foreach (CNPC npc in NPCsAvailable) {
                if (npc.WorldID == iWorldId) {
                    return npc;
                }
            }

            return null;
        }

        public CNPC FindOrAddNPC(UInt32 iWorldID) {
            CNPC npcobj = FindKnownNPC(iWorldID);
            if (npcobj == null) {
                npcobj = new CNPC();
                npcobj.WorldID = iWorldID;
                npcobj.CurrentDialog = "";

                this.NPCsAvailable.Add(npcobj);
            }

            return npcobj;
        }

        public void UpdateNPCInfo(UInt32 iWorldId, String sNick, Int32 x, Int32 y) {
            var npc = FindOrAddNPC(iWorldId);
            npc.Nickname = sNick;
            npc.X = x;
            npc.Y = y;
        }

        public void UpdatePlayerInfo(UInt32 iWorldId, String sNick, Int32 x, Int32 y) {
            CAnotherPlayer c = FindOrAddChar(iWorldId);
            c.Nickname = sNick;
            c.X = (Int32)x;
            c.Y = (Int32)y;
        }

        public void CacheItemInfo(CItem item) {
            this.ItemCache.Add(item);
        }

        public CItem FindItemInCache(UInt32 itemid) {
            foreach (var item in this.ItemCache) {
                if (item.Id == itemid) {
                    return item;
                }
            }

            return null;
        }

        public CQuest GetKnownQuest(UInt32 iQuestId) {
            foreach (var q in QuestsAvailable) {
                if (q.QuestID == iQuestId) {
                    return q;
                }
            }

            return null;
        }

        public bool DoesCharMeetQuestRequirements(CCharSelf aChar, UInt32 iQuestId) {
            CQuest q = GetKnownQuest(iQuestId);
            if (q != null) {
                foreach (var item in q.RequiredItems) {
                    if (aChar.CountInBags(item.item_id) < item.amount) {
                        return false;
                    }
                }
            }

            return true;
        }
    };

};
