using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GameClient {
    public partial class Debug : Form {
        private List<String> lstMessages;
        private List<String> lstCombatEventMessages;

        public Debug() {
            InitializeComponent();

            lstMessages = new List<String>();
            lstCombatEventMessages = new List<String>();
        }

        public void addMessage(String s) {
            lstMessages.Add(s);
        }

        public void addCombatMessage(String s) {
            lstCombatEventMessages.Add(s);
        }

        private void timer1_Tick(object sender, EventArgs e) {
            textBox1.Text = "";

            for (int i = 0; i < lstMessages.Count; i++) {
                textBox1.Text += lstMessages[i];
            }

            // lock
            for (int i = 0; i < lstCombatEventMessages.Count; i++) {
                lstCombatEvents.Items.Add(lstCombatEventMessages[i]);
            }
            lstCombatEventMessages.Clear();
            // unlock

            lstMessages.Clear();
        }
    }
}
