using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GameClient {
    public partial class LoginScreen : Form {
        public String getUsername() {
            return edUsername.Text;
        }
        public String getPassword() {
            return edPassword.Text;
        }

        public LoginScreen() {
            InitializeComponent();

            edUsername.Focus();
        }

        private void LoginScreen_Load(object sender, EventArgs e) {

        }

        private void btnLogin_Click(object sender, EventArgs e) {

        }

        private void edPassword_KeyPress(object sender, KeyPressEventArgs e) {
            if (e.KeyChar == 13) {
                DialogResult = System.Windows.Forms.DialogResult.OK;
            }
        }
    }
}
