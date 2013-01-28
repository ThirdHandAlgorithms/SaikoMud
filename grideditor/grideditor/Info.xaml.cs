using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace grideditor {
    /// <summary>
    /// Interaction logic for Info.xaml
    /// </summary>
    public partial class Info : Window {
        protected Point xy = new Point(0,0);
        protected string desc = "";
        protected RoomInfoCallback func;

        public Info() {
            InitializeComponent();
        }

        public void setRoomInfo( Point room, string description, RoomInfoCallback func ) {
            xy.X = room.X;
            xy.Y = room.Y;
            this.desc = description;
            this.func = func;

            label1.Content = "(" + room.X.ToString() + "," + room.Y.ToString() + ")";

            textBox1.Clear();
            textBox1.Text = description;
        }

        private void button1_Click(object sender, RoutedEventArgs e) {
            if ( func != null ) {
                func(xy, textBox1.Text);
            }
            textBox1.Focus();
        }
    }

    public delegate void RoomInfoCallback( Point room, string description );
}
