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

        protected List<uint> npcs = new List<uint>();

        public Info() {
            InitializeComponent();
        }

        public void clearNpcs() {
            lstNPCs.Items.Clear();
            npcs.Clear();
        }

        public void feedEnvtypes( saikomudDataSet.environmentDataTable table ) {
            comboBox1.Items.Clear();

            //comboBox1.ItemsSource = from r in table.AsEnumerable() select r.name;


            foreach (saikomudDataSet.environmentRow pRow in table.Rows) {
                comboBox1.Items.Add( new EnvTypeItem(pRow) );
            }
        }

        public void addNPC( uint id, string name ) {
            lstNPCs.Items.Add(name);
            npcs.Add(id);
        }

        protected int getCbxIndexOfEnv( uint id ) {
            int i = 0;

            foreach (EnvTypeItem item in comboBox1.Items) {
                if (item.id == id) {
                    return i;
                }
                i++;
            }

            return -1;
        }

        public void setRoomInfo( Point room, string description, uint envtype, bool traversable, RoomInfoCallback func ) {
            xy.X = room.X;
            xy.Y = room.Y;
            this.desc = description;
            this.func = func;

            label1.Content = "(" + room.X.ToString() + "," + room.Y.ToString() + ")";

            textBox1.Clear();
            textBox1.Text = description;

            checkBox1.IsChecked = traversable;

            comboBox1.SelectedIndex = getCbxIndexOfEnv(envtype);
        }

        public void save() {
            if (func != null) {
                EnvTypeItem item = (EnvTypeItem)(comboBox1.SelectedItem);
                func(xy, textBox1.Text, item.id, (bool)checkBox1.IsChecked);
            }
        }

        private void button1_Click(object sender, RoutedEventArgs e) {
            save();

            textBox1.Focus();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e) {

        }
    }

    public delegate void RoomInfoCallback(Point room, string description, uint envtype, bool traversable);

    public class EnvTypeItem: ComboBoxItem {
        public uint id;

        public EnvTypeItem(saikomudDataSet.environmentRow pRow) {
            this.id = (uint)(pRow.id);

            uint rgb = uint.Parse(pRow.flatcolor, System.Globalization.NumberStyles.AllowHexSpecifier);
            Color c = Color.FromRgb((byte)((rgb >> 16) & 255), (byte)((rgb >> 8) & 255), (byte)(rgb & 255));

            Content = pRow.name;
            Background = new SolidColorBrush(c);
        }
    }
}
